#!/usr/bin/python3

from gmc import GMC
from influxdb import InfluxDBClient
import configparser

config = configparser.ConfigParser()
config.read('/home/leo/bin/cheapWeather.ini')

influxUser=config.get('Influx','user')
influxPass=config.get('Influx','password')
influxDB=config.get('Influx','database')
influxHost=config.get('Influx','host')

client = InfluxDBClient(host=(influxHost), port=8086, username=(influxUser), password=(influxPass), database=(influxDB))

g = GMC()
CPM=g.cpm()
cpmraw = CPM.replace("CPM","")
cpmint = int(cpmraw)

gmcJSON = [{"measurement":"cpm",

        "fields":
        {
        "CPM":(cpmint),
        }
        }
        ]

client.write_points(gmcJSON)

