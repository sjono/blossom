 # Name: tweet_stream.py
 # Author: Jono Sanders
 # Date: Jun 18 2016
 # Description: Code to stream twitter results based on desired filters
 # 08/24 Updated to look at tweet times
	
import os
import json
import time

from twitter import Api

consumer_key = 'na'
consumer_secret = 'na'
access_token = 'na'
access_token_secret = 'na'
#From blossominteract account


def main():
	'''Run main code here'''
	# Users to watch for should be a list. This will be joined by Twitter and the
	# data returned will be for any tweet mentioning:
	# @twitter *OR* @twitterapi *OR* @support.
	# filter = ['hunger', 'filter:images']
	filter = ['#hunger']
	# Assumption - this excludes retweets?
	api = Api(consumer_key,
			consumer_secret,
			access_token,
			access_token_secret)
	
	for line in api.GetStreamFilter(track=filter):
		try: 
			entry = {"id": line["id"], "text": line["text"], "time": time.time()}
		except:
			print "unable to store tweet" #this works!
		
	#location = (39.985306,-75.229460, 39.913735, -75.139111) #Bounding box around Philly
									# Doc on this at https://dev.twitter.com/streaming/overview/request-parameters
	# if include_entities == true
	# has filter:links
	
	
if __name__ == '__main__':
    main()
	
	
	
