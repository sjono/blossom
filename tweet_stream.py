 # Name: tweet_stream.py (GITHUB VERSION!)
 # Author: Jono Sanders
 # Date: Jun 18 2016
 # Description: Code to stream twitter results based on desired filters
 # 09/01 Updated to post to mongodb
	
import json
import time 
import pymongo
import os

from twitter import Api



##Use below for heroku deployment
#consumer_key = os.getenv('TWITTER_CONSUMER_KEY')
#consumer_secret = os.getenv('TWITTER_CONSUMER_SECRET')
#access_token = os.getenv('TWITTER_ACCESS_TOKEN')
#access_token_secret = os.getenv('TWITTER_ACCESS_SECRET')

consumer_key = 'gbaambMUJadDoqEGTa03Z8urI'
consumer_secret = 'iWpdNvYhgKRZIPgy3TRlMZ2OH8vz28N2B9keWxULw4i9o6ZAp4'
access_token = '732740396430839809-2aibPYIhLRgLbSWinemwTbrDpskEiwe'
access_token_secret = 'gJJS8ovoACCHzAfe05QP4E0qqORI8zru5uxmzVanIGxCs'


def main():
	'''Run main code here'''
	# Users to watch for should be a list. This will be joined by Twitter and the
	# data returned will be for any tweet mentioning:
	# @twitter *OR* @twitterapi *OR* @support.
	# filter = ['hunger', 'filter:images']
	filter = ['#feedblossom']
	# Assumption - this excludes retweets?
	api = Api(consumer_key,
			consumer_secret,
			access_token,
			access_token_secret)
	#client = pymongo.MongoClient('mongodb://localhost:27017/') 	#Locally hoste! Add server info here!
	client = pymongo.MongoClient('mongodb://blossominteractive:plant2bear@ds019846.mlab.com:19846/blossom_test')
	
	db = client['blossom_test'] # CHANGE THIS TO CORRECT Database!!
	
	collection = db['watering']
	print("opening twitter connection")
	while True:
		try:
			for line in api.GetStreamFilter(track=filter):

				try: #Store tweet data and check for most recent tweet
					entry = {"screen_name": line["user"]["screen_name"], "text":  line["text"], "time": time.time(), "type":"tweet"}

					tweets = list(collection.find({"screen_name": entry.get("screen_name")}).sort('time', pymongo.DESCENDING))

					if len(tweets) >= 1 and (time.time() - tweets[0].get("time")) < 86400: # 24 hrs is 86,400 sec
						print("Someone tweeted >1 in 24 hrs, time: " + str(time.time() - tweets[0].get("time")))
					else:
						try:
							collection.insert_one(entry)
							print("one tweet stored")
						except:	
							print("Could not store tweet to db")
				except Exception as e:
					print("Info could not be pulled from tweet: {}".format(e)) #this works!
		except:
			print "Twitter connection interrupted, reconnecting in 20 sec"
			time.sleep(20) # Pause 20 seconds
	#END WHILE LOOP

if __name__ == '__main__':
    main()
	
	
	
