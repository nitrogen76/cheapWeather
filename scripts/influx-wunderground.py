#!/usr/bin/python3
##
## Influx to Wunderground
##
##

## Imports
import configparser
import requests
import re

from influxdb import InfluxDBClient


# Get config info
config = configparser.ConfigParser()
config.read('/home/leo/bin/cheapWeather.ini')

### Variables
softwareVersion='&softwaretype=cheapWeather%20version%20Zonda'
wundergroundUser=config.get('Wunderground','user')
wundergroundPass=config.get('Wunderground','password')
influxUser=config.get('Influx','user')
influxPass=config.get('Influx','password')
influxDB=config.get('Influx','database')
influxHost=config.get('Influx','host')
thermometer=config.get('Query','thermometer')
station=config.get('Query','station')
windStation=config.get('Query','windstation')
baroStation=config.get('Query','barostation')

tempQuery='SELECT last("temperature_C") *1.8+32   FROM '
humidityQuery='SELECT last("humidity")   FROM '
windspeedQuery='SELECT last("wind_avg_km_h") * 0.6213712 FROM '
winddirQuery='SELECT last("wind_dir_deg")  FROM '
windgustQuery='SELECT top("wind_avg_km_h", 1) * 0.6213712 FROM '
gustTime='WHERE time > now() - 15m'
baroQuery='SELECT last("Barometer") FROM '

WUurl = "https://weatherstation.wunderground.com/weatherstation/updateweatherstation.php?"
WUcreds = "ID=" + wundergroundUser + "&PASSWORD="+ wundergroundPass
######

client = InfluxDBClient(host=(influxHost), port=8086, username=(influxUser), password=(influxPass), database=(influxDB))


## Get last temp
tmp=client.query(tempQuery + station)
tmpString=str(tmp)
tmpSliced=tmpString.split(':')[5]
tempF=tmpSliced.split('}')[0]
tempF=tempF.strip()

## Get last humidity
tmp=client.query(humidityQuery + station)
tmpString=str(tmp)
tmpSliced=tmpString.split(':')[5]
humidityP=tmpSliced.split('}')[0]
humidityP=humidityP.strip()

## Get last windspeed
tmp=client.query(windspeedQuery + windStation)
tmpString=str(tmp)
tmpSliced=tmpString.split(':')[5]
windMPH=tmpSliced.split('}')[0]
windMPH=windMPH.strip()

## Get last winddir
tmp=client.query(winddirQuery + windStation)
tmpString=str(tmp)
tmpSliced=tmpString.split(':')[5]
windDIR=tmpSliced.split('}')[0]
windDIR=windDIR.strip()

## get last windgust
tmp=client.query(windgustQuery + windStation + gustTime)
tmpString=str(tmp)
tmpSliced=tmpString.split(':')[5]
windGustMPH=tmpSliced.split('}')[0]
windGustMPH=windGustMPH.strip()

## get last barometer
tmp=client.query(baroQuery + baroStation)
tmpString=str(tmp)
tmpSliced=tmpString.split(':')[5]
baroINHG=tmpSliced.split('}')[0]
baroINHG=baroINHG.strip()

#print("TempF" + tempF)
#print("Humidity" + humidityP)
##print ("Wind MPH "+ windMPH)
#print("Direction " + windDIR)
#print("Gust " + windGustMPH)
#print("Barometer " + baroINHG)

wundergroundRequest=(WUurl + WUcreds + "&dateutc=now&action=updateraw" + "&humidity=" + humidityP + "&tempf=" + tempF + "&winddir=" + windDIR + "&windspeedmph=" + windMPH + "&windgustmph=" + windGustMPH + "&baromin=" + baroINHG + softwareVersion)
##print (wundergroundRequest)
httpstatus=requests.get(wundergroundRequest)
print(("Received " + str(httpstatus.status_code) + " " + str(httpstatus.text)))
