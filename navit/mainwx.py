from wxalerts import *
import time

lat = 41.8
lon = -78.64
while(True):
    time.sleep(5)
    get_alerts(lat,lon)
    lat = lat+0.01