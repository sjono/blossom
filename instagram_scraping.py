 # Name: instagram_scraping.py
 # Author: Jono Sanders
 # Date: Aug 22 2016
 # Description: Code to scrape info from an instagram tag web site
 # Sep09 - getting this to use api token (!)


import requests

import json
import time
import pymongo

#~~~~~~~~ PASTED FROM TWEET_STREAM.PY ~~~~~~~~~~~
client = pymongo.MongoClient('mongodb://blossominteractive:plant2bear@ds019846.mlab.com:19846/blossom_test')
db = client['blossom_test'] # CHANGE THIS TO CORRECT Database!!
collection = db['watering']
#~~~~~~~~~~~~~~~~END PASTE~~~~~~~~~~~~~~~~~~~~ 

url = 'https://api.instagram.com/v1/tags/wilderness/media/recent?access_token=231661582.54da896.18a9f4094b854f879d3385931910b9b4'

while (1):
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
		# check if the ID already input
		instas = list(collection.find({"screen_name": entry.get("screen_name")}).sort('time', pymongo.DESCENDING))
		user_posted_before = (len(instas) >= 1)
		if (user_posted_before is True):
			posted_recently = instas[0].get("time")-time.time() < 86400
		have_seen_before = collection.find({"instagram_id": entry}).count() > 0
		if (user_posted_before and posted_recently) or have_seen_before:
			print("insta post already counted")
		else:
			try:
				collection.insert_one(entry)
				print("one insta stored")
			except:
				print("Could not store insta to db")
	print("waiting 30 sec")
	time.sleep(30) # wait 30 seconds
	