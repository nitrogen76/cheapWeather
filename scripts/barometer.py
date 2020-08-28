#!/usr/bin/python
##
## Requires sensehat support, which is primitive in fedora.
## This would work better on raspibian, but hacks are hacks
##


## Files to read barometer info for.  Currently using a 
## raspberry PI sense hat to get internal temp, humidity, and pressure readings.
## it's primitive as hell on a raspbery pi.  Hopefully there's a better method for this."
##

import configparser
config = configparser.ConfigParser()
config.read('barometer.ini')

pressure_RAW=config.get('Main','pressure_RAW')
pressure_scale=config.get('Main','pressure_scale')

a= open(pressure_RAW)
b= open(pressure_scale)

rawX =a.read()
scaleX =b.read()

raw = float(rawX)
scale = float(scaleX)

## Math from some old driver information, reports barometer in milibars
## Multiply the answer by 0.029529983071445 for inches of Mercury.

inHGX= (((raw) * (scale) *10.16) * 0.029529983071445)
inHG= round(inHGX,2)
print(inHG)

