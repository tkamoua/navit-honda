from wxalerts import *
import time
import gpsd


gpsd.connect()
while(True):
    packet = gpsd.get_current()
    lat = packet.lat
    lon = packet.lon
    time.sleep(5)
    get_alerts(lat,lon)