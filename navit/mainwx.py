from wxalerts import *
from weather_forecast import *
import time
import gpsd


gpsd.connect()
lat = 42
lon = -83
counter = 0
while(True):
    try:
        packet = gpsd.get_current()
        lat = packet.lat
        lon = packet.lon
    except:
        pass
    time.sleep(5)
    print(lat," ",lon)
    try:
        get_alerts(lat,lon)
    except:
        pass

    if counter%60 == 0:
        try:
            get_forecast_main(lat,lon)
        except:
            pass
    counter = counter + 1