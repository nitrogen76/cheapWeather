#!/usr/bin/python3
##
## Influx to Wunderground
##
##
import configparser
import json
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
query='SELECT last("temperature_C") *1.8+32   FROM '

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

tempF=client.query(query + station)
tempResult = tempF.raw
pprint(tempResult)
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
