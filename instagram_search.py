 # Name: insta_stream.py
 # Author: Jono Sanders
 # Date: Jun 18 2016
 # Description: Code to stream instagram results based on desired filters
 # Updated 08/01
 	
from instagram.client import InstagramAPI
import requests # Preferred library to communicate with the Canvas API (alt is urllib)

client_id = 'na'
client_secret = 'na'
#From blossominteractive account

def main():
	'''Run main code here'''

#	api = InstagramAPI(client_id, client_secret)
#	recent_media = api.user_recent_media(user_id="blossominteractive", count=5)
#	for media in recent_media:
#	   print media.caption.text
	
	authenticate = {'client_id': client_id, 'client_secret': client_secret}
	token = 'na' #attempt 01
	token2 = 'na' #attempt 02
	token3 = 'na' #08/01

	url = 'https://api.instagram.com/v1/user/self/feed?access_token=ACCESSTOKEN'
	
	# this should work? https://api.instagram.com/v1/tags/nofilter/media/recent?access_token=ACCESS_TOKEN
	r = requests.get(url) # This stores that JSON into r
	print "See message: \n \n \n", r
	
if __name__ == '__main__':
    main()


	r = requests.get('https://api.github.com/user', auth=('user', 'pass'))