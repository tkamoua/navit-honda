from wxalerts import *
from weather_forecast import *
import time
import gpsd
from datetime import datetime, timedelta


gpsd.connect()
lat = 42
lon = -83
counter = 0

while(True):
    f = open("data_log.txt",'a')
    try:
        packet = gpsd.get_current()
        lat = packet.lat
        lon = packet.lon
    except:
        pass
    day = datetime.utcnow()
    data = day.strftime("%m/%d/%Y, %H:%M:%S") + "," + str(lat) + "," + str(lon) + "\n"
    f.write(data)
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
    f.close()
    counter = counter + 1