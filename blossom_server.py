from flask import Flask
app = Flask(__name__)

import os
import json
tweet_ct = 0

@app.route("/")
def counter():
	tweets = []
	#with tweets exported to json like this >>
	#json.dump(tweets, open("test.json", "w"))
	with open("test.json") as data_file:
		data = json.load(data_file)
	tweet_ct = len(data)
	return "Tweet count is: " + str(tweet_ct)
if __name__ == "__main__":
	app.run()