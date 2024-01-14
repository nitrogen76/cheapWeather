#!/usr/bin/python3
##
## Influx to Wunderground
##
##

## Imports
import configparser
import argparse
import requests
import re
import metpy.calc as mpcalc
from metpy.units import units
from influxdb import InfluxDBClient

## Argumenmts
parser = argparse.ArgumentParser('description: generate some values every minute from values in influx, and also send data to wunderground.')
parser.add_argument('-d','--debug',action='store_true', help='Show debug information')
parser.add_argument('-n','--no_wunderground',action='store_false',help='Do not use wunderground')
args=parser.parse_args()
DEBUG=(args.debug)
useWunder=(args.no_wunderground)

if DEBUG==True:
    print ("DEBUG: ENABLED")

# Get config info
config = configparser.ConfigParser()
config.read('/local/leo/cheapWeather/cheapWeather.ini')

### Variables
softwareVersion='&softwaretype=cheapWeather%20version%20swullock'

## If we aren't using wunderground, we don't need to get the variables
if useWunder==True:
    wundergroundUser=config.get('Wunderground','user')
    wundergroundPass=config.get('Wunderground','password')
if useWunder==False and DEBUG==True:
    print ("DEBUG: Wunderground disabled, variables not read")
        
influxUser=config.get('Influx','user')
influxPass=config.get('Influx','password')
influxDB=config.get('Influx','database')
influxHost=config.get('Influx','host')
thermometer=config.get('Query','thermometer')
station=config.get('Query','station')
windStation=config.get('Query','windstation')
baroStation=config.get('Query','barostation')

tempQueryC='SELECT last("temperature_C") FROM '
humidityQuery='SELECT last("humidity")   FROM '
windspeedQuery='SELECT last("wind_avg_km_h") FROM '
winddirQuery='SELECT last("wind_dir_deg") FROM '
windgustQuery='SELECT top("wind_avg_km_h", 1) FROM '
gustTime='WHERE time > now() - 15m'
baroQuery='SELECT last("Barometer") FROM '

if useWunder==True:
   WUurl = "https://weatherstation.wunderground.com/weatherstation/updateweatherstation.php?"
   WUcreds = "ID=" + wundergroundUser + "&PASSWORD="+ wundergroundPass

######

client = InfluxDBClient(host=(influxHost), port=8086, username=(influxUser), password=(influxPass), database=(influxDB))


# get last temp C
tmpC=client.query(tempQueryC + station)
tmpString=str(tmpC)
tmpSliced=tmpString.split(':')[5]
tempC=tmpSliced.split('}')[0]
tempC=tempC.strip()
tempC=float(tempC)
tempPint=(tempC * units.degC)

## Get last humidity
tmp=client.query(humidityQuery + station)
tmpString=str(tmp)
tmpSliced=tmpString.split(':')[5]
humidityP=tmpSliced.split('}')[0]
humidityP=humidityP.strip()
humidityPF=float(humidityP)
##print ("humidityPercent= ",humidityPF)
humidityPint=(humidityPF * units.percent)

## Get last windspeed
tmp=client.query(windspeedQuery + windStation)
tmpString=str(tmp)
tmpSliced=tmpString.split(':')[5]
windKPH=tmpSliced.split('}')[0]
windKPH=windKPH.strip()
windKPH=float(windKPH)
windPint=(windKPH * units.kilometer_per_hour)

## Get last winddir
tmp=client.query(winddirQuery + windStation)
tmpString=str(tmp)
tmpSliced=tmpString.split(':')[5]
windDIR=tmpSliced.split('}')[0]
windDIR=windDIR.strip()
windDIRPint=(windDIR * units.degrees)

## get last windgust
tmp=client.query(windgustQuery + windStation + gustTime)
tmpString=str(tmp)
tmpSliced=tmpString.split(':')[5]
windGustKPH=tmpSliced.split('}')[0]
windGustKPH=windGustKPH.strip()
windGustKPH=float(windGustKPH)
windGustPint=(windGustKPH * units.kilometer_per_hour)

## get last barometer
tmp=client.query(baroQuery + baroStation)
tmpString=str(tmp)
tmpSliced=tmpString.split(':')[5]
baroINHG=tmpSliced.split('}')[0]
baroINHG=baroINHG.strip()
baroINHG=float(baroINHG)
baroPint=(baroINHG * units.inHg)

## Use metpy to calculate some values
dewPointPint=mpcalc.dewpoint_from_relative_humidity(tempPint, humidityPint)
wetBulbPint=mpcalc.wet_bulb_temperature(baroPint,tempPint,dewPointPint)
windchillPint=mpcalc.windchill(tempPint,windPint,False,False)
heatindexPint=mpcalc.heat_index(tempPint,humidityPint,False)

## If temp is below what makes sense for heatindex, just make it equal to temperature
if tempPint.magnitude <= 25:
   heatindexPint = tempPint
   if DEBUG==True:
       print ("DEBUG: temp value too low to generate a heat index")
## if temp is above what makes sense for windchill, set it to the temp.
if tempPint.magnitude >= 10:
    windchillPint = tempPint
    if DEBUG==True:
        print ("DEBUG: temp value too high to generate a windchill")

if DEBUG==True:
   print("DEBUG: Values grabbed and generated")
   print("     TempF" , (tempPint.to('degF').magnitude))
   print("     Humidity" , humidityP)
   print("     Wind MPH ", (windPint.to('mile_per_hour').magnitude))
   print("     Direction " ,  windDIR)
   print("     Gust " ,  (windGustPint.to('mile_per_hour').magnitude))
   print("     Barometer " , baroINHG)
   print("     heat index: ",(heatindexPint.to('degF').magnitude))
   print("     Windchill: ", (windchillPint.to('degF').magnitude))




dewpointJSON = [{"measurement":"Dewpoint",

    "fields":
    {
    "dewpoint":(dewPointPint.magnitude)
    }
    },
    ]

heatIndexJSON = [{"measurement":"Heatindex",

    "fields":
    {
    "heatindex":(heatindexPint.magnitude)
    }
    },
    ]

WBTIndexJSON = [{"measurement":"Wet Bulb Temp",

    "fields":
    {
    "Wet Bulb Temp":(wetBulbPint.magnitude)
    }
    },
    ]

windchillJSON = [{"measurement":"Windchill",

    "fields":
    {
    "Windchill":(windchillPint.magnitude)
    }
    },
    ]

if DEBUG==False:
    client.write_points(dewpointJSON)
    client.write_points(heatIndexJSON)
    client.write_points (WBTIndexJSON)
    client.write_points (windchillJSON)

if DEBUG==True:
    print ("DEBUG: Not sending to INFLUX")
    print ("     This is what would have been sent")
    print ("     dewpoint JSON: ",dewpointJSON)
    print ("     heatIndexJSON: ",heatIndexJSON)
    print ("     Wet Bulb JSON: ",WBTIndexJSON)
    print ("     Windchill JSON: ",windchillJSON)
## hack hack bad code alert

if useWunder==True:
           wundergroundRequest=(WUurl + WUcreds + "&dateutc=now&action=updateraw" + "&humidity=" + str(humidityP) + "&tempf=" + str(tempPint.to('degF').magnitude) + "&winddir=" + str(windDIR) + "&windspeedmph=" + str(windPint.to('mile_per_hour').magnitude) + "&windgustmph=" + str(windGustPint.to('mile_per_hour').magnitude) + "&baromin=" + str(baroINHG) + "&dewptf=" + str(dewPointPint.to('degF').magnitude) + softwareVersion)
if useWunder==True and DEBUG==False:
   httpstatus=requests.get(wundergroundRequest)
   print(("Received " + str(httpstatus.status_code) + " " + str(httpstatus.text)))
   if DEBUG==True and useWunder==True:
      print ("DEBUG: wunderground URL Generated")
      print (wundergroundRequest)

if useWunder==False and DEBUG==True:
   print ("DEBUG: Wunderground send disabled")
