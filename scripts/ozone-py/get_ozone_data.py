#!/usr/bin/python3

from influxdb import InfluxDBClient
import configparser

import sys
import time
sys.path.append("../")
from DFRobot_Ozone import *

config = configparser.ConfigParser()
config.read('/etc/cheapWeather.ini')

influxUser=config.get('Influx','user')
influxPass=config.get('Influx','password')
influxDB=config.get('Influx','database')
influxHost=config.get('Influx','host')

client = InfluxDBClient(host=(influxHost), port=8086, username=(influxUser), password=(influxPass), database=(influxDB))

## Stuff needed for ozone sensor
COLLECT_NUMBER   = 100              # collect number, the collection range is 1-100
IIC_MODE         = 0x01            # default use IIC1

'''
   The first  parameter is to select i2c0 or i2c1
   The second parameter is the i2c device address
   The default address for i2c is OZONE_ADDRESS_3
      OZONE_ADDRESS_0        0x70
      OZONE_ADDRESS_1        0x71
      OZONE_ADDRESS_2        0x72
      OZONE_ADDRESS_3        0x73
'''
ozone = DFRobot_Ozone_IIC(IIC_MODE ,OZONE_ADDRESS_3)
'''
  The module is configured in automatic mode or passive
    MEASURE_MODE_AUTOMATIC  active  mode
    MEASURE_MODE_PASSIVE    passive mode
''' 
ozone.set_mode(MEASURE_MODE_AUTOMATIC)
while(1):
  ''' Smooth data collection the collection range is 1-100 '''
  ozone_concentration = ozone.get_ozone_data(COLLECT_NUMBER)
  print("Ozone concentration is %d PPB."%ozone_concentration)
  ozoneJSON = [{"measurement":"AQ",

        "fields":
        {
        "Ozone_PBB":(ozone_concentration),
        }
        }
        ]
  client.write_points(ozoneJSON)
  print (ozoneJSON)
  time.sleep(10)
