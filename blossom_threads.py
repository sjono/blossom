 # Name: blossom_threads.py
 # Author: Jono Sanders
 # Date: Sep 09 2016
 # Description: Code to run tweet stream, instagram search and server


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
	
	url = 'https://api.instagram.com/v1/tags/wilderness/media/recent?access_token=231661582.54da896.18a9f4094b854f879d3385931910b9b4'
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

def twitter_loop():
	consumer_key = 'gbaambMUJadDoqEGTa03Z8urI'
	consumer_secret = 'iWpdNvYhgKRZIPgy3TRlMZ2OH8vz28N2B9keWxULw4i9o6ZAp4'
	access_token = '732740396430839809-2aibPYIhLRgLbSWinemwTbrDpskEiwe'
	access_token_secret = 'gJJS8ovoACCHzAfe05QP4E0qqORI8zru5uxmzVanIGxCs'
	#FOR HEROKU
#	consumer_key = os.getenv('TWITTER_CONSUMER_KEY')
#	consumer_secret = os.getenv('TWITTER_CONSUMER_SECRET')
#	access_token = os.getenv('TWITTER_ACCESS_TOKEN')
#	access_token_secret = os.getenv('TWITTER_ACCESS_SECRET')
	while True:
		print ('twitter event!')
		filter = ['#freedom']
		api = Api(consumer_key,
				consumer_secret,
				access_token,
				access_token_secret)
		client = pymongo.MongoClient('mongodb://blossominteractive:plant2bear@ds019846.mlab.com:19846/blossom_test')

		db = client['blossom_test'] # CHANGE THIS TO CORRECT Database!!

		collection = db['watering']

		for line in api.GetStreamFilter(track=filter):
			try: #Store tweet data and check for most recent tweet
				entry = {"screen_name": line["user"]["screen_name"], "text":  line["text"], "time": time.time(), "type":"tweet"}

				tweets = list(collection.find({"screen_name": entry.get("screen_name")}).sort('time', pymongo.DESCENDING))

				if len(tweets) >= 1 and (tweets[0].get("time")-time.time()) < 86400: # 24 hrs is 86,400 sec
					print("Someone tweeted >1 in 24 hrs")
				else:
					try:
						collection.insert_one(entry)
						print("one tweet stored")
					except:	
						print("Could not store tweet to db")
			except Exception as e:
				print("Info could not be pulled from tweet: {}".format(e))

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
	return "Tweet count is: " + str(tweets) + "\n Instagram count is: " + str(instagram)
if __name__ == "__main__":
	app.run()

pool.apply_async(instagram_loop)
pool.apply_async(twitter_loop)