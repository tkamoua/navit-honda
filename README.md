[![Build Status](https://img.shields.io/circleci/project/github/navit-gps/navit/trunk.svg)](https://circleci.com/gh/navit-gps/navit)
[![CodeFactor](https://www.codefactor.io/repository/github/navit-gps/navit/badge)](https://www.codefactor.io/repository/github/navit-gps/navit)


Navit on Android tablet:

![navit on android](https://raw.githubusercontent.com/navit-gps/navit/trunk/contrib/images/androidtablet.png)

Navit on Linux based Carputer:

![navit-nuc-osd](https://github.com/pgrandin/navit-nuc-layout/raw/master/screenshot.png)

[<img src="https://fdroid.gitlab.io/artwork/badge/get-it-on.png"
     alt="Get it on F-Droid"
     height="130">](https://f-droid.org/packages/org.navitproject.navit/)
[<img src="https://play.google.com/intl/en_us/badges/images/generic/en-play-badge.png"
     alt="Get it on Google Play"
     height="130">](https://play.google.com/store/apps/details?id=org.navitproject.navit)

Navit
=====

Navit is a open source (GPL) car navigation system with routing engine.

It's modular design is capable of using vector maps of various formats
for routing and rendering of the displayed map. It's even possible to
use multiple maps at a time.

The GTK+ or SDL user interfaces are designed to work well with touch
screen displays. Points of Interest of various formats are displayed
on the map.

The current vehicle position is either read from gpsd or directly from
NMEA GPS sensors.

The routing engine not only calculates an optimal route to your
destination, but also generates directions and even speaks to you.

Navit currently speaks over 70 languages!

You can help translating via our web based translation page:
 http://translations.launchpad.net/navit/trunk/+pots/navit


For help or more information, please refer to the wiki:
 http://wiki.navit-project.org

If you don't know where to start, we recommend you to read the
Interactive Help : http://wiki.navit-project.org/index.php/Interactive_help


Maps
====

The best navigation system is useless without maps. Those three maps
are known to work:

- OpenStreetMap: display, routing, but street name search isn't complete
 (see http://wiki.navit-project.org/index.php/OpenStreetMap)

- Grosser Reiseplaner and compliant maps: full support
 (see http://wiki.navit-project.org/index.php/Marco_Polo_Grosser_Reiseplaner)

- Garmin maps: display, routing, search is being worked on
 (see http://wiki.navit-project.org/index.php/Garmin_maps)


GPS Support
===========

Navit reads the current vehicle position:
- directly from a file or port
- from gpsd (local or remote)
- from the location service of several mobile platforms
- from udp server (friends tracking) (experimental)


Routing algorithm
=================

Navit uses LPA* (see [Lifelong_Planning_A*](https://en.wikipedia.org/wiki/Lifelong_Planning_A*)), a derivative of the Dijkstra algorithm, for
routing. Routing starts at the destination by assigning a value to each point directly connected to the destination
point. The value represents the estimated time needed to reach the destination from that point.

Now the point with the lowest value is chosen using the Fibonacci heap, and a value is assigned to connected points
which are unevaluated or whose current value is greater than the new one.

The search is repeated until the origin is found.

Once the origin is reached, all that needs to be done is to follow the points with the lowest values to the
destination.

LPA* is slightly more complex, as it allows partial re-evaluation of the route graph as segment costs change. This is
used by the (still experimental) traffic module, which can process traffic reports and tries to find a way around
traffic problems. Refer to the Wikipedia page for a full description.
