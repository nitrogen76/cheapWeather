#!/usr/bin/python3

from sense_hat import SenseHat
import math
import time
import configparser
from influxdb import InfluxDBClient
import subprocess
import sys


config = configparser.ConfigParser()
## Not smart enough to make this plug in yet, so change it for you!
config.read('/home/leo/bin/cheapWeather.ini')

useDracal=config.get('Baro','useDracal')
useSensehat=config.get('Baro','useSenseHat')
dracalPath=config.get('Baro','dracalPath')
dracalSwitches=config.get('Baro','dracalSwitches')

influxUser=config.get('Influx','user')
influxPass=config.get('Influx','password')
influxDB=config.get('Influx','database')
influxHost=config.get('Influx','host')

thermometer=config.get('Query','thermometer')
station=config.get('Query','station')

myAltitude=config.get('Baro','myAltitude')
Altitude=float(myAltitude)
## setup influx session
client = InfluxDBClient(host=(influxHost), port=8086, username=(influxUser), password=(influxPass), database=(influxDB))
#DEBUG
#print("useSensehat is "+useSensehat)
#END DEBUG
if useSensehat == "1":
    #DEBUG
    #print("useSensehat is triggered "+useSensehat)
    #END DEBUG
    sense = SenseHat()
    #Convert to inHG
    pressure = (sense.get_pressure()*0.029529983071445)
    #Convert to Mean Sea Level Pressure
    baroX=pressure/pow(1-((Altitude)/44330.0),5.255)
    baro=round((baroX),4)
    #DEBUG
    #print( baro)
    #END DEBUG
    baroJSON = [{"measurement":"SenseHat",

        "fields":
        {
        "Barometer":(baro)
        }
        },
        ]

    #print(baroJSON)
    client.write_points(baroJSON)
else:
    print("no Sensehat")

#DEBUG
#print("useDracal is: " + useDracal)
#END DEBUG
if useDracal == "1":
    #run usbtenkiget to get the barometer reading with 19 decimals of precision
    result=subprocess.run([(dracalPath),(dracalSwitches), "-x19" ], capture_output=True)
    dracalBaro=(result.stdout).strip()
    dracalBaroFloat=float(dracalBaro)
    #DEBUG
    #print (dracalBaroFloat)
    #END DEBUG
    #This sensor has a temp sensor, lets grab that data also, why not?
    temp=subprocess.run([(dracalPath),(dracalSwitches), "-x19","-i1" ], capture_output=True)
    dracalTemp=(temp.stdout)
    dracalTempFloat=float(dracalTemp)
    ##dracalBaroX = dracalBaroFloat + (( dracalBaroFloat * 9.80665 * Altitude)/(287 * (273 + dracalTempFloat + (Altitude/400))))
    dracalBaroSL=dracalBaroFloat/pow(1-((Altitude)/44330.0),5.255)
    #dracalBaroSL=(dracalBaroX*0.02953)
    ##print(dracalBaroSL)
    dracalBaroJSON = [{"measurement":"Dracal",

        "fields":
        {
        "Barometer":(dracalBaroSL),
        "Thermometer":(dracalTempFloat)
        }
        }
        ]

    client.write_points(dracalBaroJSON)
else:
    #DEBUG
    print ("No Dracal")
    #END DEBUG
