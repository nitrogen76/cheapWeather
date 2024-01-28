#!/usr/bin/python3

##
## Influx to Wunderground
##
##
## Going back to v1, cuz v2/flux is awful.

## Imports
import datetime
import configparser
import argparse
import requests
import re
import math
import time
import subprocess
import sys
import metpy.calc as mpcalc
from metpy.units import units
#import influxdb_client
#from influxdb_client.client.write_api import SYNCHRONOUS
from influxdb import InfluxDBClient

# Get config info
config = configparser.ConfigParser(allow_no_value=True)
config.read('/etc/cheapWeather.ini')
### Variables
softwareVersion='&softwaretype=cheapWeather%20version%20pikel'

## influx
influxUser=config.get('Influx','user')
influxPass=config.get('Influx','password')
influxDB=config.get('Influx','database')
influxHost=config.get('Influx','host')

## Get config from cheapWeather.ini
thermometer=config.get('Query','thermometer')
humidityStation=config.get('Query','humiditystation')
windStation=config.get('Query','windstation')
baroStation=config.get('Query','barostation')
rainStation=config.get('Query','rainstation')
useDracal=config.get('Baro','useDracal')
dracalPath=config.get('Baro','dracalPath')
dracalSwitches=config.get('Baro','dracalSwitches')
useSensehat=config.get('Baro','useSenseHat')
Altitude=config.get('Baro','myAltitude')
Altitude=float(Altitude)
useWunderground=config.get('Wunderground','useWunderground')
pollutionMeasurement=config.get('Pollution','pollutionMeasurement')
pollutionLocation=config.get('Pollution','locationID')
pm01=config.get('Pollution','pm01')
pm02=config.get('Pollution','pm25')
pm10=config.get('Pollution','pm10')


client = InfluxDBClient(host=(influxHost), port=8086, username=(influxUser), password=(influxPass), database=(influxDB))


## Function to fix wonky influx module outputs
def fixInfluxOutput(input):
    output=str(input)
    output=output.split(':')[5].split('}')[0]
    output=output.strip()
    output=float(output)
    return output

## Function to get rain differences between a specific time and "now"
## FIXME: These are currently set in variables up top



## set some variables that some functions will use
##now      = datetime.datetime.now()
##hourago  = (datetime.datetime.now() - datetime.timedelta(hours=1))
#3nowTS    = int(now.timestamp()*1000)
##delta=(datetime.datetime.now() - datetime.timedelta(minutes=30))
##delta=int(thirtyminago.timestamp()*1000)

measure='Dracal'
field='Barometer'
def getBaroAvgQuery(measurement,field,start):

    if start == 0:
       startDT=datetime.datetime.now()
       print('      startDT: ',startDT)
       startTS=int(startDT.timestamp()*1000)
##       print (startTS)
    else: 
       startDT=datetime.datetime.now() - datetime.timedelta(minutes=start)
       print("else: startDT: ",startDT)
       print("      start  : ",start)
       startTS=int(startDT.timestamp()*1000)
    end=start+30
    deltaDT=datetime.datetime.now() - datetime.timedelta(minutes=end)
    print ('      deltaDT: ',deltaDT)
    print('         end:   ',end)
    deltaTS=int(deltaDT.timestamp()*1000)
##    print('delta: ',deltaTS)
    deltaSTR=str(deltaTS)
    startSTR=str(startTS)
    output= client.query ('SELECT mean("' + field +'")  FROM "' + measurement +'" WHERE time >=  '+ deltaSTR +'ms and time <= ' + startSTR +'ms' )
##    print('raw: ',output)
    output=fixInfluxOutput(output)
##    print(output)
    return float(output)


press0  = getBaroAvgQuery(measure,field,0)
press0Pint=(press0 * units.inHg)

## 30m
press1  =  getBaroAvgQuery(measure,field,30)
press1Pint=(press1 * units.inHg)

## 60m
press2  =  getBaroAvgQuery(measure,field,60)
press2Pint=(press2 * units.inHg)

## 90m
press3  =  getBaroAvgQuery(measure,field,90)
press3Pint=(press3 * units.inHg)

## 120m
press4  =  getBaroAvgQuery(measure,field,120)
press4Pint=(press4 * units.inHg)

## 150m
press5  =  getBaroAvgQuery(measure,field,150)
press5Pint=(press5 * units.inHg)

## 180m
press6  =  getBaroAvgQuery(measure,field,180)
press6Pint=(press6 * units.inHg)

## 210m
press7  =  getBaroAvgQuery(measure,field,210)
press7Pint=(press7 * units.inHg)

## 240
press8  =  getBaroAvgQuery(measure,field,240)
press8Pint=(press8 * units.inHg)

## 270m
press9  =  getBaroAvgQuery(measure,field,270)
press9Pint=(press9 * units.inHg)

## 300m
press10 =  getBaroAvgQuery(measure,field,300)
press10Pint=(press10 * units.inHg)

## 330m
press11 =  getBaroAvgQuery(measure,field,330)
press11Pint=(press11 * units.inHg)

## 360m
press12 =  getBaroAvgQuery(measure,field,360)
press12Pint=(press12 * units.inHg)







pressurerdiff1 = (press0Pint.to('millibar').magnitude  - press1Pint.to('millibar').magnitude)*2
pressurerdiff2 = (press0Pint.to('millibar').magnitude - press2Pint.to('millibar').magnitude)
pressurerdiff3 = (press0Pint.to('millibar').magnitude - press3Pint.to('millibar').magnitude)/1.5
pressurerdiff4 = (press0Pint.to('millibar').magnitude - press4Pint.to('millibar').magnitude)/2
pressurerdiff5 = (press0Pint.to('millibar').magnitude - press5Pint.to('millibar').magnitude)/2.5
pressurerdiff6 = (press0Pint.to('millibar').magnitude - press6Pint.to('millibar').magnitude)/3
pressurerdiff7 = (press0Pint.to('millibar').magnitude - press7Pint.to('millibar').magnitude)/3.5
pressurerdiff8 = (press0Pint.to('millibar').magnitude - press8Pint.to('millibar').magnitude)/4
pressurerdiff9 = (press0Pint.to('millibar').magnitude - press9Pint.to('millibar').magnitude)/4.5
pressurerdiff10 = (press0Pint.to('millibar').magnitude - press10Pint.to('millibar').magnitude)/5
pressurerdiff11 = (press0Pint.to('millibar').magnitude - press11Pint.to('millibar').magnitude)/5.5
pressurerdiff12 = (press0Pint.to('millibar').magnitude - press12Pint.to('millibar').magnitude)/6



# Calculate the average of the differences
pressurerdiff = (pressurerdiff1 + pressurerdiff2 + pressurerdiff3 + pressurerdiff4 + pressurerdiff5 + pressurerdiff6 + pressurerdiff7 + pressurerdiff8 + pressurerdiff9 + pressurerdiff10 + pressurerdiff11 + pressurerdiff12)/12

# Get the current pressure
currentpress = press0

# Use the calculated pressure difference to finally make the forecast
# -------------------------------------------------------------------
# Rising Conditions
# -----------------
# Quickly rising, very unstable weather condition
print ('pressurerdiff is: ',pressurerdiff)
if pressurerdiff > 0.75:
    print('rising: unstable')
# Slowly rising, good weather condition, tendency rising
elif pressurerdiff > 0.42:
    print('rising: good, sun')
# Change in weather condition is possible, tendency rising
elif pressurerdiff > 0.25:
    print('rising: change')
    if (currentpress >= 1006 and currentpress <= 1020) or currentpress < 1006:
        print('sun')
# Falling Conditions
# ------------------
# Quickly falling, thunderstorm is highly possible
elif pressurerdiff < -0.75:
    print('falling: storm')
# Slowly falling, rainy weather condition, tendency falling
elif pressurerdiff < -0.42:
    print('falling: rainy')
    shutil.copyfile('/home/pi/pressure_info/DownRight.png', '/var/www/html/Arrow.png')
# Condition change is possible, tendency falling
elif pressurerdiff < -0.25:
    print('falling: change')
    if (currentpress >= 1006 and currentpress <= 1020) or currentpress > 1020:
        print('suncloud')
# Steady Conditions
# -----------------
# Condition is stable, don't change the weather symbol (sun, rain or sun/cloud), just change the arrow
elif pressurerdiff <= 0.25 and pressurerdiff >= -0.25:
    print ('stable')

