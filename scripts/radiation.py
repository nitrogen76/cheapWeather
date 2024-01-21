#!/usr/bin/python3
## radiation grabber for influx v2
## pip install gmc
## will get the module you need.

from gmc import GMC
import influxdb_client 
from influxdb_client.client.write_api import SYNCHRONOUS
import configparser

config = configparser.ConfigParser()
config.read('/etc/cheapWeather.ini')

##influxUser=config.get('Influx','user')
##influxPass=config.get('Influx','password')
##influxDB=config.get('Influx','database')
##influxHost=config.get('Influx','host')
influxv2Url=config.get('InfluxV2','url')
influxv2Bucket=config.get('InfluxV2','bucket')
influxv2Org=config.get('InfluxV2','org')
influxv2Token=config.get('InfluxV2','token')

##client = InfluxDBClient(host=(influxHost), port=8086, username=(influxUser), password=(influxPass), database=(influxDB))
client = influxdb_client.InfluxDBClient(url=(influxv2Url), token=(influxv2Token), org=(influxv2Org), bucket=(influxv2Bucket))

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

##client.write_points(gmcJSON)

write_api = client.write_api(write_options=SYNCHRONOUS)

p = influxdb_client.Point("cpm").tag("location", "Sachse").field("CPM",(cpmint))
write_api.write(bucket=(influxv2Bucket),org=(influxv2Org),record=p)
