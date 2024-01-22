#!/usr/bin/python3

##
## Influx to Wunderground
##
##
## Going back to v1, cuz v2/flux is awful.

## Imports
import configparser
import argparse
import requests
import re
import math
import time
import subprocess
import sys
import metpy.calc as mpcalc
from metpy.units import units
#import influxdb_client
#from influxdb_client.client.write_api import SYNCHRONOUS
from influxdb import InfluxDBClient

## Argumenmts
parser = argparse.ArgumentParser('description: generate some values every minute from values in influx, and also send data to wunderground.')
parser.add_argument('-d','--debug',action='store_true', help='Show debug information')
parser.add_argument('-n','--no_wunderground',action='store_false',help='Do not use wunderground')
args=parser.parse_args()
DEBUG=(args.debug)
useWunder=(args.no_wunderground)
if useWunder==1:
   useWunder=True
elif useWunder==0:
     useWunder=False
else:
   useWunder=True

if DEBUG==True:
    print ("DEBUG: ENABLED")

# Get config info
config = configparser.ConfigParser(allow_no_value=True)
config.read('/etc/cheapWeather.ini')
### Variables
softwareVersion='&softwaretype=cheapWeather%20version%20swullock'

## If we aren't using wunderground, we don't need to get the variables
if useWunder==True:
    wundergroundUser=config.get('Wunderground','user')
    wundergroundPass=config.get('Wunderground','password')
if useWunder==False and DEBUG==True:
    print ("DEBUG: Wunderground disabled, variables not read")
## influx
influxUser=config.get('Influx','user')
influxPass=config.get('Influx','password')
influxDB=config.get('Influx','database')
influxHost=config.get('Influx','host')

thermometer=config.get('Query','thermometer')
humidityStation=config.get('Query','humiditystation')
windStation=config.get('Query','windstation')
baroStation=config.get('Query','barostation')
useDracal=config.get('Baro','useDracal')
dracalPath=config.get('Baro','dracalPath')
dracalSwitches=config.get('Baro','dracalSwitches')
useSensehat=config.get('Baro','useSenseHat')
Altitude=config.get('Baro','myAltitude')
Altitude=float(Altitude)
useWunderground=config.get('Wunderground','useWunderground')



##influxv2Url=config.get('InfluxV2','url')
##influxv2Bucket=config.get('InfluxV2','bucket')
##influxv2Org=config.get('InfluxV2','org')
##influxv2Token=config.get('InfluxV2','token')

pollutionMeasurement=config.get('Pollution','pollutionMeasurement')
pollutionLocation=config.get('Pollution','locationID')
pm01=config.get('Pollution','pm01')
pm02=config.get('Pollution','pm25')
pm10=config.get('Pollution','pm10')

altitudePint=(Altitude * units.meters)
minWindChillPint=(50 * units.degF)
minHeatIndexPint=(80 * units.degF)
minWindChillWindPint=(3 * units.mile_per_hour)
## initialize influxDB V1
client = InfluxDBClient(host=(influxHost), port=8086, username=(influxUser), password=(influxPass), database=(influxDB))

def getQuery(field,measurement,varType):
    output=client.query('select last(' + field + ') FROM ' + measurement + ' GROUP BY time(-1h)')
    output=str(output)
    output=output.split(':')[5].split('}')[0]
    output=output.strip()
    if varType == 'int':
       output=int(output)
       if DEBUG == True:
          print ('DEBUG: int specifically chosen for getQuery')
    elif varType == 'float':
       output = float(output)
       if DEBUG == True:
          print ('DEBUG: Float specifically chosen for getQuery')
    else:
       output=float(output)
       if DEBUG == True:
          print ('DEBUG: Float chosen by default for getQuery')
    return output

def getPollutionQuery(field,measurement,location):
    output=client.query('select last(' + field + ') FROM "' + measurement + '" where  "locationId"::field = ' + location + ' GROUP BY time(-1h)')
    output=str(output)
    output=output.split(':')[5].split('}')[0]
    output=output.strip()
    return output


# Define influx queries to get information
tempPint=(getQuery('temperature_C',thermometer,float) * units.degC)
humidityPint=(getQuery('humidity',humidityStation,float) * units.percent)
windPint=(getQuery('wind_avg_km_h',windStation,float) * units.kilometer_per_hour)
windDIRPint=(getQuery('wind_dir_deg',windStation,int) * units.degrees)
windGustPint=(getQuery('wind_avg_km_h',windStation,float) * units.kilometer_per_hour)
PM1Pint=(getPollutionQuery(pm01,pollutionMeasurement,pollutionLocation) * units.micrograms)
PM10Pint=(getPollutionQuery(pm10,pollutionMeasurement,pollutionLocation) * units.micrograms)
PM02Pint=(getPollutionQuery(pm02,pollutionMeasurement,pollutionLocation) * units.micrograms)



if DEBUG==True:
   print("DEBUG: Configuration options passed from cheapWeather.ini")
   print("       useSensehat= ", useSensehat)
   print("       Altitude= ", Altitude)
   print("       useDracal= ", useDracal)
   print("       dracalSwitches= ", dracalSwitches)
   print("       dracalPath= ", dracalPath)
   if useWunder == True:
      print("       useWunderground= ", useWunderground)
      print("       wunderground user= ", wundergroundUser)
   print("       thermometer= ", thermometer)
   print("       humidity station= ", humidityStation)
   print("       wind station= ", windStation)
   print("       baro station= ", baroStation)
##   print("       influxv2URL= ", influxv2Url)
##   print("       influxv2Bucket= ",influxv2Bucket)
##   print("       influxv2Org= ",influxv2Org)

if useWunder==True or useWunderground==1:
   WUurl = "https://weatherstation.wunderground.com/weatherstation/updateweatherstation.php?"
   WUcreds = "ID=" + wundergroundUser + "&PASSWORD="+ wundergroundPass

######

##client = influxdb_client.InfluxDBClient(url=(influxv2Url), token=(influxv2Token), org=(influxv2Org), bucket=(influxv2Bucket))
##write_api = client.write_api(write_options=SYNCHRONOUS)
##query_api = client.query_api()

## Move barometer.py into this
if useSensehat == "1":
    if DEBUG == True:
       print("useSensehat is triggered "+useSensehat)
    sense = SenseHat()
    #Convert to inHG
    pressure = (sense.get_pressure()*0.029529983071445)
    #Convert to Mean Sea Level Pressure
    baroX=pressure/pow(1-((Altitude)/44330.0),5.255)
    baro=round((baroX),4)
    if DEBUG==True:
      print(baro)
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
    if DEBUG==True:
       print("no Sensehat")
       print("useDracal is: " + useDracal)
if useDracal == "1":
    #run usbtenkiget to get the barometer reading with 19 decimals of precision
    result=subprocess.run([(dracalPath),(dracalSwitches), "-x6" ], capture_output=True)
    dracalBaro=(result.stdout).strip()
    dracalBaroFloat=float(dracalBaro)
    if DEBUG==True:
       print (dracalBaroFloat)
    #This sensor has a temp sensor, lets grab that data also, why not?
    temp=subprocess.run([(dracalPath),(dracalSwitches), "-x6","-i1" ], capture_output=True)
    dracalTemp=(temp.stdout)
    dracalTempFloat=float(dracalTemp)
    baroSL=dracalBaroFloat/pow(1-((Altitude)/44330.0),5.255)
    baroPint=(baroSL * units.inHg)
    uncorrectedBaroPint=(dracalBaroFloat * units.inHg)
    if DEBUG==True:
       print(baroPint)
    baroJSON = [{"measurement":"Dracal",
        "fields":
        {
        "Barometer":(baroPint.magnitude),
        "Thermometer":(dracalTempFloat),
        "UnCorrectedBarometer":(dracalBaroFloat)
        }
        }
        ]

    client.write_points(baroJSON)




## Get pollution
## get_last_pollution(bucket,field,measurement,location):


## Use metpy to calculate some values
dewPointPint=mpcalc.dewpoint_from_relative_humidity(tempPint, humidityPint)
wetBulbPint=mpcalc.wet_bulb_temperature(baroPint,tempPint,dewPointPint)
windchillPint=mpcalc.windchill(tempPint,windPint,False,False)
heatindexPint=mpcalc.heat_index(tempPint,humidityPint,False)

## If temp is below what makes sense for heatindex, just make it equal to temperature
if tempPint.to('degC').magnitude <= minHeatIndexPint.to('degC').magnitude:
   heatindexPint = tempPint
   if DEBUG==True:
      print ("DEBUG: temp value, ", tempPint.to('degC'), "too low to generate a heat index, ", minHeatIndexPint.to('degC'))

## If windspeed is below minWindChillWindPint then thats too low for a windchill.
if windPint.to('kilometer_per_hour').magnitude <= minWindChillWindPint.to('kilometer_per_hour').magnitude:
   windchillPint = tempPint
   if DEBUG==True:
      print ("DEBUG: Wind speed too low, ",(windPint.to('kilometer_per_hour'))," to generate a windchill, ", minWindChillWindPint.to('kilometer_per_hour'))

## if temp is above what makes sense for windchill, set it to the temp.
if tempPint.magnitude >= minWindChillPint.to('degC').magnitude:
    windchillPint = tempPint
    if DEBUG==True:
        print ("DEBUG: temp value, ",(tempPint).to('degC')," too high to generate a windchill", (minWindTempPint).to('degC'))

if windchillPint.magnitude > tempPint.magnitude:
   windchillPint=tempPint
   if DEBUG==True:
      print ("DEBUG: Windchill was higher than temperature, which is wrong")

if DEBUG==True:
   print("DEBUG: Values grabbed and generated")
   print("     TempF" , tempPint.to('degF'))
   print("     Humidity" , humidityPint)
   print("     Wind ", windPint.to('mile_per_hour'))
   print("     Direction " ,  windDIRPint)
   print("     Gust " ,  windGustPint.to('mile_per_hour'))
   print("     Sea Level Corrected Barometer " , baroPint)
   print("     Station Pressure, ", uncorrectedBaroPint)
   print("     heat index: ",heatindexPint.to('degF'))
   print("     Windchill: ", windchillPint.to('degF'))
   print("     Altitude: ", altitudePint)

## Create JSON output for influx V1 because V2/flux sucks 
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


##dewpointPOINTS  = influxdb_client.Point("Dewpoint").field("dewpoint",(dewPointPint.magnitude))
##heatIndexPOINTS = influxdb_client.Point("Heatindex").field("heatindex",(heatindexPint.magnitude))
##WBTIndexPOINTS  = influxdb_client.Point("Wet Bulb Temp").field("Wet Bulb Temp",(wetBulbPint.magnitude))
##windchillPOINTS = influxdb_client.Point("Windchill").field("Windchill",(windchillPint.magnitude))
if DEBUG==False:
    client.write_points(dewpointJSON)
    client.write_points(heatIndexJSON)
    client.write_points (WBTIndexJSON)
    client.write_points (windchillJSON)

if DEBUG==True:
    print ("DEBUG: Not sending to INFLUX")
    print ("     This is what would have been sent")
    print ("     dewpoint : ",dewpointJSON)
    print ("     heatIndex: ",heatIndexJSON)
    print ("     Wet Bulb : ",WBTIndexJSON)
    print ("     Windchill : ",windchillJSON)
    print ("     Barometer : ",baroJSON)

if useWunder == True:
# Build the wunderground URL
           wundergroundRequest=(WUurl + WUcreds + "&dateutc=now&action=updateraw" + "&humidity=" + str(humidityPint.magnitude) + "&tempf=" + str(tempPint.to('degF').magnitude) + "&winddir=" + str(windDIRPint.magnitude) + "&windspeedmph=" + str(windPint.to('mile_per_hour').magnitude) + "&windgustmph=" + str(windGustPint.to('mile_per_hour').magnitude) + "&baromin=" + str(baroPint.magnitude) + "&dewptf=" + str(dewPointPint.to('degF').magnitude) + softwareVersion)
if useWunder==True and DEBUG==False:
   httpstatus=requests.get(wundergroundRequest)
   print(("Received " + str(httpstatus.status_code) + " " + str(httpstatus.text)))
if DEBUG==True and useWunder==True:
      print ("DEBUG: wunderground URL Generated but not sent in DEBUG mode")
      print ("     ",wundergroundRequest)

if useWunder==False and DEBUG==True:
   print ("DEBUG: Wunderground send disabled")
