 # Name: blossom_threads.py [GITHUB version!!]
 # Author: Jono Sanders
 # Date: Sep 09 2016
 # Description: Code to run tweet stream, instagram search and server
 # 09/17 Updated to search for #FeedBlossom
 # 09/18 Jono updated to run tweet_stream in a separate dyno
 # 09/18 Run based on twilio texts
 # 09/19 Added try for instagram connection

from multiprocessing.pool import ThreadPool
import time

from flask import Flask
app = Flask(__name__)

import os
import json
import pymongo
import requests
from twilio.rest import TwilioRestClient

from twitter import Api

pool = ThreadPool(processes=3)



def instagram_loop():
	client = pymongo.MongoClient('mongodb://blossominteractive:plant2bear@ds019846.mlab.com:19846/blossom_test')
	db = client['blossom_test']
	collection = db['watering']
	
	url = 'https://api.instagram.com/v1/tags/feedblossom/media/recent?access_token=231661582.54da896.18a9f4094b854f879d3385931910b9b4'
	while True:
		try: 
			res = requests.get(url)
			body = res.json()
			print ('instagram event!')
		except:
			print("instagram get failed")
		# Get desired parameters from each instagram post in the json
		for item in body['data']:
			if item['caption'] is None:
				text = ''
			else:
				text = item['caption']['text']
			entry = {
			  'screen_name': item['user']['username'],
			  'text': text,
			  'time': time.time(),
			  'instagram_id': item['id'],
			  'type': 'instagram',
			} 
			# check if the ID already inpu
			instas = list(collection.find({"screen_name": entry.get("screen_name")}).sort('time', pymongo.DESCENDING))
			user_posted_before = (len(instas) >= 1)
			if (user_posted_before is True):
				posted_recently = time.time()-instas[0].get("time") < 86400
			else: posted_recently = False
			have_seen_before = collection.find({"instagram_id": entry.get("instagram_id")}).count() > 0
			if posted_recently or have_seen_before:
				pass
				#print("insta post already counted")
			else:
				try:
					collection.insert_one(entry)
					print("one insta stored")
				except:
					print("Could not store insta to db")
		print("waiting 20 sec")
		time.sleep(20) # wait 30 seconds

def text_send():
	twilio_ct = 0
	account = "ACf7902a47050a7fdc9a5d96915dcb0f19"
	token = "5a4a726216f7a908d37258bd57069469"
	twilio = TwilioRestClient(account, token)
	print("twilio setup complete")
	client = pymongo.MongoClient('mongodb://blossominteractive:plant2bear@ds019846.mlab.com:19846/blossom_test')
	db = client['blossom_test'] # CHANGE THIS TO CORRECT Database!!
	collection = db['watering']
	try: 
		tweets = collection.find({"type": "tweet"}).count()
	except: tweets = 0
	try:
		instagram = collection.find({"type": "instagram"}).count()
	except: instagram = 0
	waters = instagram + tweets
	temp = waters
	message = twilio.messages.create(to="+15038476273", from_="+12672140103", body=str(waters)) #Initialize for blossom
	print("initial counts taken and texted")
	while(True):
		try: 
			tweets = collection.find({"type": "tweet"}).count()
		except: tweets = 0
		try:
			instagram = collection.find({"type": "instagram"}).count()
		except: instagram = 0
		waters = instagram + tweets
		if (waters > temp):
			#sent twilio sms
			message = twilio.messages.create(to="+15038476273", from_="+12672140103", body=str(waters))
			print("Twilio send a message to Blossom - count is" + str(waters))
		temp = waters
		twilio_ct+=1
		if (twilio_ct > 50):
			print("twilio checked 50 times (~4 min), waiting another 5 sec") #remove this line!
			twilio_ct = 0
		time.sleep(5)
		
		
@app.route('/')
def client():
	client = pymongo.MongoClient('mongodb://blossominteractive:plant2bear@ds019846.mlab.com:19846/blossom_test')
	db = client['blossom_test'] # CHANGE THIS TO CORRECT Database!!
	collection = db['watering']
	
	try: 
		tweets = collection.find({"type": "tweet"}).count()
	except: tweets = 0
	try:
		instagram = collection.find({"type": "instagram"}).count()
	except: instagram = 0
	return str(tweets+instagram) + " is TOTAL, Tweet count is: " + str(tweets) + "\n Instagram count is: " + str(instagram)
if __name__ == "__main__":
	app.run()

pool.apply_async(instagram_loop)
pool.apply_async(text_send)