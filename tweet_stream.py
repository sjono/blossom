 # Name: tweet_stream.py (GITHUB VERSION!)
 # Author: Jono Sanders
 # Date: Jun 18 2016
 # Description: Code to stream twitter results based on desired filters
 # 08/24 Updated to look at tweet times, this version is in GITHUB
 # 08/27 Fixed screen_name part
	
import os
import json
import time

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
	tweets = []
	
	for line in api.GetStreamFilter(track=filter):
		try:
			entry = {"screen_name": line["entities"]["user_mentions"][0]["screen_name"], "text": line["text"], "time": time.time()}
			tweets.append(entry)
			json.dump(tweets, open("test.json", "w"))  
			print("one tweet stored")
		except:
			print("unable to store tweet") #this works!

if __name__ == '__main__':
    main()
	
	
	
