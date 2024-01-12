* ~~Post dashboards with variables to choose different measurements~~
* ~~Better query for dewpoints with the real math.  This will be easier with Flux.~~
* Better way to calculate dewpoints than doing in code every minute
* ~~write a better script for wunderground exports~~ Crappy python3 script replaces crappy shell script.
* ~~Figure out a good way to make custom combinations of sensors for a panel with variables, etc.~~
* Better quality scripts than the initial stabs.  Scripts need error checking, and debug
* Better queries for lightning sensor.  (Distance should be null if there
  have been no strikes for the timefilter.)
* Work with rtl_433 crew to get the lux and UV meter readings useful

* Clean up the code so it's ~~not~~ less embarrassing garbage.
* See about an easy way to switch between Freedom Units and SI units.  All backend values are in SI units, to make computation easier, but it's a bit of a hack right now to get all the output in Freedom Units.  IT should be easily switchable instead of modifying each query.  
* See about automatically calculating a forecast from historical values and deltas.
* Modify wunderground script to disable dewpoint calculation or wunderground uploads (or fix dewpoint queries)

* Convert to MetPy for all the calculation crap.  It crashed hard on raspibian which I hate.

* Implement [Airgradient](https://www.airgradient.com) monitoring
