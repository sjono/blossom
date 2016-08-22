 # Name: tweet_stream.py
 # Author: Jono Sanders
 # Date: Jun 18 2016
 # Description: Code to stream twitter results based on desired filters
 	
import os
import json

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
	
	with open('output.txt', 'a') as f:
		# api.GetStreamFilter will return a generator that yields one status
		# message (i.e., Tweet) at a time as a JSON dictionary.
		for line in api.GetStreamFilter(track=filter):
			#json_text = "'" + json.dumps(line) + "'"
			parsed_json = json.dumps(line)
			print parsed_json
			f.write("Post by " + parsed_json['name'] + ": " + parsed_json['text'] + "\n" + "Posted on " + parsed_json['created_at'])
			f.write('\n')
	#location = (39.985306,-75.229460, 39.913735, -75.139111) #Bounding box around Philly
									# Doc on this at https://dev.twitter.com/streaming/overview/request-parameters
	# if include_entities == true
	# has filter:links
	
	
if __name__ == '__main__':
    main()
	
	
	
