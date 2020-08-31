#!/usr/bin/python3

from sense_hat import SenseHat
import math
import time
import configparser
from influxdb import InfluxDBClient


config = configparser.ConfigParser()
config.read('/home/leo/bin/cheapWeather.ini')
influxUser=config.get('Influx','user')
influxPass=config.get('Influx','password')
influxDB=config.get('Influx','database')
influxHost=config.get('Influx','host')
thermometer=config.get('Query','thermometer')
station=config.get('Query','station')

sense = SenseHat()
myAltitude=config.get('Baro','myAltitude')
Altitude=float(myAltitude)

pressure = (sense.get_pressure()*0.029529983071445)



baroX=pressure/pow(1-((Altitude)/44330.0),5.255)
baro=round((baroX),2)
#print(baro) 

baroJSON = [{"measurement":"SenseHat",

        "fields":
        {
        "Barometer":(baro)
        }
        },
        ]

#print(baroJSON)
client = InfluxDBClient(host=(influxHost), port=8086, username=(influxUser), password=(influxPass), database=(influxDB))

client.write_points(baroJSON)
