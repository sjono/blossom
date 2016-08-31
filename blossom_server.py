# Name: blossom_server.py (GITHUB VERSION!)
 # Author: Jono Sanders
 # Date: Aug 24 2016
 # Description: Code to count twitter posts


from flask import Flask
app = Flask(__name__)

import os
import json


@app.route("/")
def counter():
	tweets = []
	tweet_ct = 0
	#with tweets exported to json like this >>
	#json.dump(tweets, open("test.json", "w"))
	with open("test.json") as data_file:
		try: 
			data = json.load(data_file)
		except:
			data = 0
			print("Empty JSON file")
	if (data != 0):
		tweet_ct = len(data)
	return "Tweet count is: " + str(tweet_ct)
if __name__ == "__main__":
	app.run()