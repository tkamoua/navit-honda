#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Created on Tue Sep 14 20:12:26 2021
@author: samephraim
"""
import requests
import json
import ephem
from datetime import datetime, timedelta
import numpy as np
def get_forecast_main(lat, lon):
    day = datetime.utcnow()
    wx = get_forecast(lat,lon,day)
    time_arr = np.empty(0)
    wx_code_arr = np.empty(0)
    temp_arr = np.empty(0)
    prob_arr = np.empty(0)
    for forc in wx:
        time_arr = np.append(time_arr, datetime.strptime(forc["startTime"], "%Y-%m-%dT%H:%M:%SZ"))
        wx_code_arr = np.append(wx_code_arr, forc["values"]["weatherCode"])
        prob_arr = np.append(prob_arr, int(forc["values"]["precipitationProbability"]))
        temp_arr = np.append(temp_arr, forc["values"]["temperature"])
    f2 = open("temp_forecast.txt", "w+")
    f2.write(str(temp_arr[0]))
    # precip in the future/no precip
    if prob_arr[0] < 50:
        first_ind = np.argmax(prob_arr > 50)
        #precip in future
        if first_ind:
             f = open("time_forecast.txt", "w+")
             f.write("in " + str(first_ind*5) + " minutes")
             f = open("precip_forecast.txt", "w+")
             f.write(str(prob_arr[first_ind]))
             f = open("icon_forecast.txt", "w+")
             f.write(get_icon(lat, lon, wx_code_arr[first_ind], time_arr[first_ind]))
        # no precip in future
        else:
             f = open("time_forecast.txt", "w+")
             f.write("in 30 minutes")
             f = open("precip_forecast.txt", "w+")
             f.write(str(prob_arr[-1]))
             f = open("icon_forecast.txt", "w+")
             f.write(get_icon(lat, lon, wx_code_arr[-1], time_arr[-1]))
    #precip occuring
    else:
        end_ind = np.argmax(prob_arr < 50)
        #precip ending
        if end_ind:
             f = open("time_forecast.txt", "w+")
             f.write("for " + str(end_ind*5) + " minutes")
             f = open("precip_forecast.txt", "w+")
             f.write(str(prob_arr[0]))
             f = open("icon_forecast.txt", "w+")
             f.write(get_icon(lat, lon, wx_code_arr[0], time_arr[0]))
        # precip continuing
        else:
             f = open("time_forecast.txt", "w+")
             f.write("for 30+ minutes")
             f = open("precip_forecast.txt", "w+")
             f.write(str(prob_arr[0]))
             f = open("icon_forecast.txt", "w+")
             f.write(get_icon(lat, lon, wx_code_arr[0], time_arr[0]))
def get_forecast(lat,lon,time):
    coor = str(lat) + "," + str(lon)
    startTime = (str(time.year) + "-" + str(time.strftime('%m')) + "-" +
                 str(time.strftime('%d')) + "T" + str(time.strftime('%H')) + ":"
                 + str(time.strftime('%M'))+
                 ":00Z")
    time = time + timedelta(minutes = 30)
    endTime = (str(time.year) + "-" + str(time.strftime('%m')) + "-" +
                 str(time.strftime('%d')) + "T" + str(time.strftime('%H')) + ":"
                 + str(time.strftime('%M'))+ ":00Z")
    url = "https://api.tomorrow.io/v4/timelines"
    querystring = {"location": coor,"fields":["weatherCode",
                "precipitationProbability","temperature"],"units":"imperial","timesteps":"5m",
                "startTime":startTime, "endTime":endTime,"apikey":"ET05gU7mq7CClzvZLSbeHfZCTwfY50AR"}
    headers = {"Accept": "application/json"}
    response = requests.request("GET", url, headers=headers, params=querystring)
    data = json.loads(response.text)
    print(data)
    return data["data"]["timelines"][0]["intervals"][:]
def get_icon(lat, lon, wx_code, time):
    codes = {
      "0": "Unknown",
      "1000": "clear",
      "1001": "cloudy",
      "1100": "mostly_clear",
      "1101": "partly_cloudy",
      "1102": "mostly_cloudy",
      "2000": "fog",
      "2100": "fog_light",
      "3000": "Light Wind",
      "3001": "Wind",
      "3002": "Strong Wind",
      "4000": "drizzle",
      "4001": "rain",
      "4200": "rain_light",
      "4201": "rain_heavy",
      "5000": "snow",
      "5001": "flurries",
      "5100": "snow_light",
      "5101": "snow_heavy",
      "6000": "freezing_drizzle",
      "6001": "freezing_rain",
      "6200": "freezing_rain_light",
      "6201": "freezing_rain_heavy",
      "7000": "ice_pellets",
      "7101": "ice_pellets_heavy",
      "7102": "ice_pellets_light",
      "8000": "tstorm"
    }
    wx_code = int(wx_code)
    user = ephem.Observer()
    user.date = time
    user.lat = str(lat)    # See wikipedia.org/Oldenburg
    user.lon = str(lon)   # See wikipedia.org/Oldenburg
    user.elevation = 4        # See wikipedia.org/Oldenburg
    user.temp = 20            # current air temperature gathered manually
    user.pressure = 1019.5    # current air pressure gathered manually
    next_sunrise_datetime = user.next_rising(ephem.Sun()).datetime()
    next_sunset_datetime = user.next_setting(ephem.Sun()).datetime()
    # If it is daytime, we will see a sunset sooner than a sunrise.
    it_is_day = next_sunset_datetime < next_sunrise_datetime
    print("It's day." if it_is_day else "It's night.")
    img = codes[str(wx_code)]
    if str(wx_code) in {"1000","1100","1101"}:
        sun = ""
        if it_is_day:
            sun = "day"
        else:
            sun = "night"
        img = img + "_" + sun
    return img