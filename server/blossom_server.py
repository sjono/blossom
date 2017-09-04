# Name: blossom_server.py (GITHUB VERSION!)
 # Author: Jono Sanders
 # Date: Aug 24 2016
 # Description: Code to count twitter posts
 # 09/01 Updated to search mongodb collection


from flask import Flask
app = Flask(__name__)

import os
import json
import pymongo


@app.route("/")
def counter():
	
	client = pymongo.MongoClient('mongodb://blossominteractive:plant2bear@ds019846.mlab.com:19846/blossom_test')
	
	db = client['blossom_test'] # CHANGE THIS TO CORRECT Database!!
	collection = db['watering']
	
	
	try: 
		tweets = collection.find({"type": "tweet"}).count()
	except tweets = 0
	try:
		instagram = collection.find({"type": "instagram"}).count()
	except instagram = 0
		print("Empty database")

	return "Tweet count is: " + str(tweets) + "\n Instagram count is: " + str(instagram)
if __name__ == "__main__":
	app.run()