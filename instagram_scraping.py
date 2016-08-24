 # Name: instagram_scraping.py
 # Author: Jono Sanders
 # Date: Aug 22 2016
 # Description: Code to scrape info from an instagram tag web site

import bs4
import requests
import selenium.webdriver as webdriver
url = 'https://www.instagram.com/explore/tags/wilderness/'
driver = webdriver.Firefox()
driver.get(url)
text= bs4.BeautifulSoup(driver.page_source)