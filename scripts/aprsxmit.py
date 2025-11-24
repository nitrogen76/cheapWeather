#!/usr/bin/python3

## Proof of concept/testing

import aprslib
import datetime
import configparser
import argparse
##import requests
##import re
##import math
##import time
import pytz
#3import subprocess
#import sys
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
tenminago = (datetime.datetime.now() - datetime.timedelta(minutes=10))
morning=datetime.datetime(year,month,day,0,0,0)
morningTS= morning.timestamp()*1000
morningTS=int(morningTS)
morning=str(morningTS)

houragoTS=hourago.timestamp()*1000
houragoTS=int(houragoTS)
hourago=str(houragoTS)

tenminagoTS=tenminago.timestamp()*1000
tenminagoTS=int(tenminagoTS)
tenminago=str(tenminagoTS)

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

def getAltimiterPressure(mbPressure,mHeight):
   F1 = (1013.25 ** 0.190284) * 0.0065/288
   F2 = (mHeight)/(((mbPressure) - 0.3) ** 0.190284)
   F3 = 1/0.190284
   F = (1 + ((F1) * (F2))) ** (F3)
   alt = (mbPressure - 0.3) *F
   return alt


# Define influx queries to get information
tempPint=(getQuery('temperature_C',thermometer,'int') * units.degC)
humidityPint=(getQuery('humidity',humidityStation,'int') * units.percent)
windPint=(getQuery('wind_avg_km_h',windStation,'int') * units.kilometer_per_hour)
windDIRPint=(getWindDirQuery('wind_dir_deg',windStation,'mode','int') * units.degrees)
windGustPint=(getQuery('wind_max_km_h',windStation,'int') * units.kilometer_per_hour)
rainDiffHourPint=(getRainDiffQuery(rainStation,hourago) * units.millimeter)
rainDiffDayPint=(getRainDiffQuery(rainStation,morning) * units.millimeter)
rainDiff10minPint=(getRainDiffQuery(rainStation,tenminago) * units.millimeter)
lux=(getQuery('light_lux',windStation,'int'))
uv=(getQuery('uvi',windStation,'int'))
stationBaroINHGPint=(getQuery('UnCorrectedBarometer','Dracal',int) * units.inHg)
stationBaroMB=(stationBaroINHGPint.to('hPa'))
winddir=round(windDIRPint.magnitude)
windspeed=round(windPint.to('mile_per_hour').magnitude)
windgust=round(windGustPint.to('mile_per_hour').magnitude)
temp=round(tempPint.to('degF').magnitude)
##rainHrPint=(rainDiffHourPint.to('inch').magnitude)
rainHr=round(rainDiffHourPint.to('inch').magnitude,3)
rainMidnight=round(rainDiffDayPint.to('inch').magnitude,3)
rain10min=round(rainDiff10minPint.to('inch').magnitude,3)
## Get altimiter from station pressure and altitide

baroMB=getAltimiterPressure(stationBaroMB.magnitude,Altitude)
print (baroMB)

baro=round(baroMB*10)
winddir=str(round(winddir)).zfill(3)
windspeed=str(windspeed).zfill(3)
windgust=str(windgust).zfill(3)
temp=str(temp).zfill(3)
solarRadiation=(lux/126.7)


## Solar radiation is a little different, we need to make sure it's 3 digits

if solarRadiation <= 999:
   solarRadiationAPRSSymbol="L"
   solarRadiationFormatted=str(int(solarRadiation)).zfill(3)
   if DEBUG==True:
      print ('solarRadiation is less than 999')
elif solarRadiation >= 999:
      solarRadiationFormatted=str(int(round(solarRadiation)/10)).zfill(3)
      solarRadiationAPRSSymbol="l"
      print ('solarRadiation is greater than 999')
      

if DEBUG==True:
   print ('solarRadiation unmodified is: ',solarRadiation)
   print ('solarRadiationAPRSSymbol is: ',solarRadiationAPRSSymbol)
   print ('SolarRadiationFormatted is: ',solarRadiationFormatted)


## Rain is a little different, we need to make sure it's 3 digits
rainHr=int(rainHr*100)
rainMidnight=int(rainMidnight*100)
rain10min=int(rain10min*100)
## then we need to stringify it with leading zeros if necessary
rainHr=str(rainHr).zfill(3)
rainMidnight=str(rainMidnight).zfill(3)
rain10min=str(rain10min).zfill(3)

if DEBUG == True:
   print ('rainMidnight str is: ',rainMidnight)
   print ('rainHr str is: ',rainHr)
baro=str(baro)
humidity=str(humidityPint.magnitude).zfill(2)


## Connect to an APRS-IS server with creds
AIS=aprslib.IS((aprsCallsign), passwd=(aprsPassword), port=(aprsPort), host=(aprsHost))
if DEBUG == True:
   print ('aprs callsign is: ',(aprsCallsign), 'aprs password is: ',(aprsPassword), 'aprs port is: ',(aprsPort), 'arps host is: ',(aprsHost))
AIS.connect()

## Format the weather message
aprsWeatherFrame=('@'+(zuluday)+\
                     (zuluhour)+\
                     (zuluminute)+\
                     'z'+(aprsLat)+\
                     '/'+(aprsLong)+\
                     '_'+(winddir)+'/'+(windspeed)+\
                     'g'+(windgust)+\
                     't'+(temp)+\
                     'r'+(rainHr)+\
                     'P'+(rainMidnight)+\
                     (solarRadiationAPRSSymbol)+(solarRadiationFormatted)\
                     +'h'+(humidity)+\
                     'b'+(baro)+\
                     '#'+(rain10min))

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

