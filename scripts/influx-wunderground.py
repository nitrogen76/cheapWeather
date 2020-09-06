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
baroQuery



## DEBUG
#print (wundergroundUser)
#print (wundergroundPass)
#print (influxUser)
#print (influxPass)
#print (influxDB)
#print (influxHost)
#print (thermometer)
#print (station)
## End # DEBUG

client = InfluxDBClient(host=(influxHost), port=8086, username=(influxUser), password=(influxPass), database=(influxDB))


## Get last temp
temp=client.query(tempQuery + station)
##tempResult = tempF.raw
tempString=str(temp)

print(tempString)
tempSliced=tempString.split(':')[5]
print("Sliced " +tempSliced)
tempF=tempSliced.split('}')[0]
print("Diced " +tempF)

## Get last humidity
humidity=client.query(humidityQuery + station)

## Get last windspeed
windSpeed=client.query(windspeedQuery + windStation)

## Get last winddir
windDir=client.query(winddirQuery + windStation)

## get last windgust
windGust=client.query(windgustQuery + windStation + gustTime)

## get last barometer
barometer=client.query()



#results.raw
#humidity=client.query('SELECT last("humidity")   FROM (thermometer)')
#windspeed=client.query('SELECT last("wind_avg_km_h") * 0.6213712 FROM (station)')
#winddir=client.query('SELECT last("wind_dir_deg")  FROM (station)')
#windgust=client.query('SELECT top("wind_avg_km_h", 1) * 0.6213712 FROM (station) WHERE time > now() - 15m')

#print(tempF)
#print(humidity)
#print(windspeed)
#print(winddir)
#print(windgust)
