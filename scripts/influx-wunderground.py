#!/usr/bin/python
##
## Influx to Wunderground 
##
##
import configparser
from influxdb import InfluxDBClient
config = configparser.ConfigParser()
config.read('influx-wunderground.ini')

wundergroundUser=config.get('Wunderground','user')
wundergroundPass=config.get('Wunderground','password')
influxUser=config.get('Influx','user')
influxPass=config.get('Influx','password')
influxDB=config.get('Influx','database')
influxHost=config.get('Influx','host')
thermometer=config.get('Query','thermometer')
station=config.get('Query','station')
tempQuery='SELECT last("temperature_C") *1.8+32   FROM "Acurite-5n1"'

print (wundergroundUser)
print (wundergroundPass)
print (influxUser)
print (influxPass)
print (influxDB)
print (influxHost)
print (thermometer)
print (station)

client = InfluxDBClient(host=(influxHost), port=8086, username=(influxUser), password=(influxPass), database=(influxDB))

tempF=client.query(tempQuery)
#results.raw
#humidity=client.query('SELECT last("humidity")   FROM (thermometer)')
#windspeed=client.query('SELECT last("wind_avg_km_h") * 0.6213712 FROM (station)')
#winddir=client.query('SELECT last("wind_dir_deg")  FROM (station)')
#windgust=client.query('SELECT top("wind_avg_km_h", 1) * 0.6213712 FROM (station) WHERE time > now() - 15m')

print(tempF)
#print(humidity)
#print(windspeed)
#print(winddir)
#print(windgust)
