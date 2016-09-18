 # Name: blossom_threads.py
 # Author: Jono Sanders
 # Date: Sep 09 2016
 # Description: Code to run tweet stream, instagram search and server
 # 09/17 Updated to search for #FeedBlossom
 # 09/18 Jono updated to run tweet_stream in a separate dyno


from multiprocessing.pool import ThreadPool
import time

from flask import Flask
app = Flask(__name__)

import os
import json
import pymongo
import requests

from twitter import Api

pool = ThreadPool(processes=3)


def instagram_loop():
	client = pymongo.MongoClient('mongodb://blossominteractive:plant2bear@ds019846.mlab.com:19846/blossom_test')
	db = client['blossom_test']
	collection = db['watering']
	
	url = 'https://api.instagram.com/v1/tags/feedblossom/media/recent?access_token=231661582.54da896.18a9f4094b854f879d3385931910b9b4'
	while True:
		print ('instagram event!')
		res = requests.get(url)
		body = res.json()
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
				posted_recently = instas[0].get("time")-time.time() < 86400
			have_seen_before = collection.find({"instagram_id": entry}).count() > 0
			if (user_posted_before and posted_recently) or have_seen_before:
				pass
				#print("insta post already counted")
			else:
				try:
					collection.insert_one(entry)
					print("one insta stored")
				except:
					print("Could not store insta to db")
		print("waiting 30 sec")
		time.sleep(30) # wait 30 seconds


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
pool.apply_async(twitter_loop)