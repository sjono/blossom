 # Name: tweet_stream.py (GITHUB VERSION!)
 # Author: Jono Sanders
 # Date: Jun 18 2016
 # Description: Code to stream twitter results based on desired filters
 # 09/01 Updated to post to mongodb
	
import json
import time
import pymongo

from twitter import Api


consumer_key = 'gbaambMUJadDoqEGTa03Z8urI'
consumer_secret = 'iWpdNvYhgKRZIPgy3TRlMZ2OH8vz28N2B9keWxULw4i9o6ZAp4'
access_token = '732740396430839809-2aibPYIhLRgLbSWinemwTbrDpskEiwe'
access_token_secret = 'gJJS8ovoACCHzAfe05QP4E0qqORI8zru5uxmzVanIGxCs'
#From blossominteract account


def main():
	'''Run main code here'''
	# Users to watch for should be a list. This will be joined by Twitter and the
	# data returned will be for any tweet mentioning:
	# @twitter *OR* @twitterapi *OR* @support.
	# filter = ['hunger', 'filter:images']
	filter = ['#freedom']
	# Assumption - this excludes retweets?
	api = Api(consumer_key,
			consumer_secret,
			access_token,
			access_token_secret)
	#client = pymongo.MongoClient('mongodb://localhost:27017/') 	#Locally hoste! Add server info here!
	client = pymongo.MongoClient('mongodb://blossominteractive:plant2bear@ds019846.mlab.com:19846/blossom_test')
	
	db = client['blossom_test'] # CHANGE THIS TO CORRECT Database!!
	
	collection = db['watering']
	
	for line in api.GetStreamFilter(track=filter):
		try:
			entry = {"screen_name": line["entities"]["user_mentions"][0]["screen_name"], "text": line["text"], "time": time.time(), "type":"tweet"}
			try:
				collection.insert_one(entry)
				print("one tweet stored")
			except:
				print("Could not store tweet to db")
		except:
			print("Info not pulled from tweet") #this works!

if __name__ == '__main__':
    main()
	
	
	
