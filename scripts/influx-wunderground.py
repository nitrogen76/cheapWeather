#!/usr/bin/python3
##
## Influx to Wunderground
##
##
import configparser
import json
import re
from pprint import pprint

from influxdb import InfluxDBClient
config = configparser.ConfigParser()
config.read('/home/leo/bin/cheapWeather.ini')

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


client = InfluxDBClient(host=(influxHost), port=8086, username=(influxUser), password=(influxPass), database=(influxDB))


## Get last temp
tmp=client.query(tempQuery + station)
tmpString=str(tmp)
print(tmpString)
tmpSliced=tmpString.split(':')[5]
print("Sliced " +tmpSliced)
tempF=tmpSliced.split('}')[0]
print("Diced " +tempF)

## Get last humidity
tmp=client.query(humidityQuery + station)
tmpString=str(tmp)
##print(tmpString)
tmpSliced=tmpString.split(':')[5]
##print("Sliced " +tmpSliced)
humidityP=tmpSliced.split('}')[0]
##print("Diced " +humidityP)

## Get last windspeed
tmp=client.query(windspeedQuery + windStation)
tmpString=str(tmp)
##print(tmpString)
tmpSliced=tmpString.split(':')[5]
##print("Sliced " +tmpSliced)
windMPH=tmpSliced.split('}')[0]
##print("Diced " +windMPH)

## Get last winddir
tmp=client.query(winddirQuery + windStation)
tmpString=str(tmp)
##print(tmpString)
tmpSliced=tmpString.split(':')[5]
##print("Sliced " +tmpSliced)
windDIR=tmpSliced.split('}')[0]
##print("Diced " +windDIR)


## get last windgust
tmp=client.query(windgustQuery + windStation)
tmpString=str(tmp)
##print(tmpString)
tmpSliced=tmpString.split(':')[5]
##print("Sliced " +tmpSliced)
windGustMPH=tmpSliced.split('}')[0]
##print("Diced " +windGustMPH)

## get last barometer
tmp=client.query(baroQuery + baroStation)
tmpString=str(tmp)
##print(tmpString)
tmpSliced=tmpString.split(':')[5]
##print("Sliced " +tmpSliced)
baroINHG=tmpSliced.split('}')[0]
##print("Diced " +baroINHG)

print("TempF" + tempF)
print("Humidity" + humidityP)
print ("Wind MPH "+ windMPH)
print("Direction " + windDIR)
print("Gust " + windGustMPH)
print("Barometer " + baroINHG)
