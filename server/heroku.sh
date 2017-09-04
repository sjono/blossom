#!/bin/bash
gunicorn blossom_server:app --daemon
python tweet_stream.py