* ~~Post dashboards with variables to choose different measurements~~
* ~~Better query for dewpoints with the real math.  This will be easier with Flux.~~
  * This was done instead in the calculate.py script, which uses metpy, which is better.
* ~~write a better script for wunderground exports~~
*   Crappy python3 script replaces crappy shell script.
* ~~Figure out a good way to make custom combinations of sensors for a panel with variables, etc.~~
* ~~Work with rtl_433 crew to get the lux and UV meter readings useful~~
* ~~Convert to MetPy for all the calculation crap.  It crashed hard on raspibian which I hate.~~
* ~~Implement [Python pints](https://pypi.org/project/Pint/) for better tracking of units~~

* Better way to calculate dewpoints than doing in code every minute
* Better quality scripts than the initial stabs.  Scripts need error checking, and debug
* Better queries for lightning sensor.  (Distance should be null if there
  have been no strikes for the timefilter.)
* Clean up the code so it's ~~not~~ less embarrassing garbage.
* See about an easy way to switch between Freedom Units and SI units.  All backend values are in SI units, to make computation easier, but it's a bit of a hack right now to get all the output in Freedom Units.  IT should be easily switchable instead of modifying each query.  
* See about automatically calculating a forecast from historical values and deltas.
* Modify wunderground script to disable dewpoint calculation or wunderground uploads (or fix dewpoint queries)
* Implement [Airgradient](https://www.airgradient.com) monitoring  (this can easily be done with telegraf, but want to test some local solutions that do not require their service)
