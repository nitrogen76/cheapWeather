#!/usr/bin/python3

## Proof of concept/testing

import aprslib
import datetime
import configparser
import argparse
import requests
import re
import math
import time
import pytz
import subprocess
import sys
import metpy.calc as mpcalc
from openweather_pws import Measurements
from metpy.units import units
from influxdb import InfluxDBClient

DEBUG=False
## Argumenmts
parser = argparse.ArgumentParser('description: Send an aprs beacon with weather data')
parser.add_argument('-d','--debug',action='store_true', help='Show debug information')
args=parser.parse_args()
DEBUG=(args.debug)

tz_zulu=pytz.timezone('GMT-0')

if DEBUG==True:
   print ("Debug mode is on")

## set some variables that some functions will use
now = datetime.datetime.now()
zulunow = datetime.datetime.now(tz_zulu)
zuluday = str(zulunow.strftime('%d'))
zuluhour = str(zulunow.strftime('%H'))
zuluminute = str(zulunow.strftime('%M'))
year= int(now.strftime('%Y'))
month=int(now.strftime('%m'))
day=  int(now.strftime('%d'))
thisMin = int(now.strftime('%M'))
hourago = (datetime.datetime.now() - datetime.timedelta(hours=1))
morning=datetime.datetime(year,month,day,0,0,0)
morningTS= morning.timestamp()*1000
morningTS=int(morningTS)
morning=str(morningTS)
houragoTS=hourago.timestamp()*1000
houragoTS=int(houragoTS)
hourago=str(houragoTS)


##Get config info
config = configparser.ConfigParser(allow_no_value=True)
config.read('/etc/cheapWeather.ini')
### Variables
softwareVersion='&softwaretype=cheapWeather%20version%20pikel'
influxUser=config.get('Influx','user')
influxPass=config.get('Influx','password')
influxDB=config.get('Influx','database')
influxHost=config.get('Influx','host')
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
aprsCallsign=config.get('APRS','callsign')
aprsPassword=config.get('APRS','password')
aprsPort=config.get('APRS','port')
aprsHost=config.get('APRS','host')
aprsLat=config.get('APRS','lat')
aprsLong=config.get('APRS','long')
Altitude=float(Altitude)

## If we have an SSID, lets get it, otherwise we'll leave it blank
try:
   aprsSSID=config.get('APRS','ssid')
except:
   aprsSSID=None

if DEBUG == True:
   print ('aprsSSID is', aprsSSID)



## If we have a custom status message, we'll use it, otherwise we'll use a default
try:
   aprsStatus=config.get('APRS','status')
except:
   aprsStatus='cheapWeather: https://github.com/nitrogen76/cheapWeather'

if DEBUG == True:
   print ('aprsStatus is', aprsStatus)

## initialize influxDB V1
client = InfluxDBClient(host=(influxHost), port=8086, username=(influxUser), password=(influxPass), database=(influxDB))

def fixInfluxOutput(input):
    output=str(input)
    if output == 'ResultSet({})':
       output = None
       if DEBUG == True:
           print ("fixInfluxOutput didn't find any output, writing a null instead")
    else:
       output=output.split(':')[5].split('}')[0]
       output=output.strip()
       output=float(output)
    return output


## Function to make a generic query from influx for a generic value
def getQuery(field,measurement,varType):
    output=client.query('select last(' + field + ') FROM ' + measurement + ' GROUP BY time(-1h)')
    output=fixInfluxOutput(output)
    if varType == 'int':
       output=int(output)
    elif varType == 'float':
       output = float(output)
    else:
       output=float(output)
    return output

## Function to get wind
def getWindDirQuery(field,measurement,aggregatorType,varType):
    output=client.query('select  last(' + field + ') FROM ' + measurement + ' GROUP BY time(-1h)')
    output=fixInfluxOutput(output)
    if varType == 'int':
       output=int(output)
    elif varType == 'float':
       output = float(output)
    else:
       output=int(output)
    if aggregatorType != 'median' and aggregatorType !='mean' and aggregatorType != 'mode':
        raise SystemExit('invalid aggregatorType "'+aggregatorType+'" used in getWindDirQuery.')
    return output

def getRainDiffQuery(measurement,delta):
    max=client.query('SELECT max("rain_mm")  FROM ' + measurement +' WHERE time >=  '+ delta +'ms and time <= now()' )
    max=fixInfluxOutput(max)
    min=client.query('SELECT min("rain_mm")  FROM ' + measurement +' WHERE time >=  '+ delta +'ms and time <= now()' )
    min=fixInfluxOutput(min)
    if min == None or max == None:
       output=0
       if DEBUG == True:
          print ("Rain difference plugin had nulls")
    else:
       output=(max-min)
    return output


# Define influx queries to get information
tempPint=(getQuery('temperature_C',thermometer,'int') * units.degC)
humidityPint=(getQuery('humidity',humidityStation,'int') * units.percent)
windPint=(getQuery('wind_avg_km_h',windStation,'int') * units.kilometer_per_hour)
windDIRPint=(getWindDirQuery('wind_dir_deg',windStation,'mode','int') * units.degrees)
windGustPint=(getQuery('wind_max_km_h',windStation,'int') * units.kilometer_per_hour)
rainDiffHourPint=(getRainDiffQuery(rainStation,hourago) * units.millimeter)
rainDiffDayPint=(getRainDiffQuery(rainStation,morning) * units.millimeter)
lux=(getQuery('light_lux',windStation,'int'))
uv=(getQuery('uvi',windStation,'int'))
baroINHGPint=(getQuery('Barometer','Dracal',int) * units.inHg)
baroMB=(baroINHGPint.to('hPa'))


winddir=round(windDIRPint.magnitude)
windspeed=round(windPint.to('mile_per_hour').magnitude)
windgust=round(windGustPint.to('mile_per_hour').magnitude)
temp=round(tempPint.to('degF').magnitude)
rainHr=round(rainDiffHourPint.to('inch').magnitude)
rain24Hr=round(rainDiffDayPint.to('inch').magnitude)
baro=round(baroMB.magnitude *10)
winddir=str(round(winddir)).zfill(3)
windspeed=str(windspeed).zfill(3)
windgust=str(windgust).zfill(3)
temp=str(temp).zfill(3)
rainHr=str(rainHr).zfill(3)
rain24Hr=str(rain24Hr).zfill(3)
baro=str(baro)
humidity=str(humidityPint.magnitude).zfill(2)


## Connect to an APRS-IS server with creds
AIS=aprslib.IS((aprsCallsign), passwd=(aprsPassword), port=(aprsPort), host=(aprsHost))
if DEBUG == True:
   print ('aprs callsign is: ',(aprsCallsign), 'aprs password is: ',(aprsPassword), 'aprs port is: ',(aprsPort), 'arps host is: ',(aprsHost))
AIS.connect()

## Format the weather message
aprsWeatherFrame=('@'+(zuluday)+(zuluhour)+(zuluminute)+'z'+(aprsLat)+'/'+(aprsLong)+'_'+(winddir)+'/'+(windspeed)+'g'+(windgust)+'t'+(temp)+'r'+(rainHr)+'p'+(rain24Hr)+'P...'+'h'+(humidity)+'b'+(baro))

## Format the APRS prefix
aprsPrefix=((aprsCallsign)+'>ADPR16,WIDE1-1:')

## Put the message together
aprsFullMessage=((aprsPrefix)+(aprsWeatherFrame))
if DEBUG == True:
   print (aprsFullMessage)
   print ('Not sending APRS weather beacon because debug is on')

else:
   AIS.sendall(aprsFullMessage)
   print ('Sending APRS weather beacon')

    
## Format the status message
aprsFullStatus=((aprsPrefix)+'>'+(aprsStatus))
if DEBUG == True:
   print (aprsFullStatus)
   print ('Not sending APRS status because debug is on')
else:
   AIS.sendall(aprsFullStatus)
   print ('Sending APRS status beacon')

