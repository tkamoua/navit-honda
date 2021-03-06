#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Created on Mon Mar  1 11:49:24 2021
@author: samephraim
"""
from urllib.request import urlopen
import urllib.request as urllib2
import json
import numpy as np
import time
def index_equal(word, arr):
    arr = np.flip(arr)
    for ind, term in enumerate(arr):
        if term == word:
            return ind
    return -1
def get_alerts(lat,lon):
    URL = ("https://api.weather.gov/alerts?active=true&point="
    + str(lat) + "%2C" + str(lon))
    f = urllib2.urlopen(URL)
    json_string = f.read()
    parsed_json = json.loads(json_string)
    severity_arr = ["Unknown", "Minor", "Moderate", "Severe", "Extreme"]
    urgency_arr = ["Unknown", "Past", "Future", "Expected", "Immediate"]
    certainty_arr = ["Unknown", "Unlikely", "Possible", "Likely", "Observed"]
    alert_list = parsed_json['features']
    best_score = 0
    best_event = " "
    fl = open("wxalerts.txt", "r")
    prev_alert = fl.readline()
    fl.close()
    print(prev_alert)
    for alert in alert_list:
        details = alert['properties']
        event = details['event']
        description = details['description']
        instruction = details['instruction']
        severity = details['severity'] #[ Extreme, Severe, Moderate, Minor, Unknown ]
        urgency = details['urgency'] #[ Immediate, Expected, Future, Past, Unknown ]
        certainty = details['certainty'] #[ Observed, Likely, Possible, Unlikely, Unknown ]
        score = index_equal(severity,severity_arr) + index_equal(urgency,urgency_arr) + index_equal(certainty, certainty_arr)
        if event!="Special Weather Statement":
            if score>best_score:
                best_event = event
                best_score = score
    #f = open("wxalerts.txt", "w+")
    #print(best_event)
    fl = open("wxalerts.txt", "w+")
    #best_event = "Winter Weather Advisory"
    if not(best_event==prev_alert):
        #print("Hi")
        fl = open("wxalerts.txt", "w+")
        fl.write(best_event)
        fl.close()
        time.sleep(1)
        fl = open("wxalerts.txt", "w+")
        fl.write(" ")
        fl.close()
        time.sleep(1)
        fl = open("wxalerts.txt", "w+")
        fl.write(best_event)
        fl.close()
        time.sleep(1)
        fl = open("wxalerts.txt", "w+")
        fl.write(" ")
        fl.close()
        time.sleep(1)
    
    fl = open("wxalerts.txt", "w+")
    fl.write(best_event)
    fl.close()
