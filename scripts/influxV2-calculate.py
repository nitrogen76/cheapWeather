#!/usr/bin/python3
##
## Influx to Wunderground influx v2
##
##

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
import influxdb_client
from influxdb_client.client.write_api import SYNCHRONOUS

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

influxv2Url=config.get('InfluxV2','url')
influxv2Bucket=config.get('InfluxV2','bucket')
influxv2Org=config.get('InfluxV2','org')
influxv2Token=config.get('InfluxV2','token')


tempQueryC='SELECT last("temperature_C") FROM '
humidityQuery='SELECT last("humidity")   FROM '
windspeedQuery='SELECT last("wind_avg_km_h") FROM '
winddirQuery='SELECT last("wind_dir_deg") FROM '
windgustQuery='SELECT top("wind_avg_km_h", 1) FROM '
gustTime='WHERE time > now() - 15m'
baroQuery='SELECT last("Barometer") FROM '
           
if DEBUG==True:
   print("DEBUG: Configuration options passed from cheapWeather.ini")
   print(       "useSensehat= ", useSensehat)
   print(       "Altitude= ", Altitude)
   print(       "useDracal= ", useDracal)
   print(       "dracalSwitches= ", dracalSwitches)
   print(       "dracalPath= ", dracalPath)
   print(       "useWunderground= ", useWunderground)
   print(       "wunderground user= ", wundergroundUser)
   print(       "thermometer= ", thermometer)
   print(       "humidity station= ", humidityStation)
   print(       "wind station= ", windStation)
   print(       "baro station= ", baroStation)
   print(       "influxv2URL= ", influxv2Url)
   print(       "influxv2Bucket= ",influxv2Bucket)
   print(       "influxv2Org= ",influxv2Org)

if useWunder==True or useWunderground==1:
   WUurl = "https://weatherstation.wunderground.com/weatherstation/updateweatherstation.php?"
   WUcreds = "ID=" + wundergroundUser + "&PASSWORD="+ wundergroundPass

######

#client = InfluxDBClient(host=(influxHost), port=8086, username=(influxUser), password=(influxPass), database=(influxDB))
client = influxdb_client.InfluxDBClient(url=(influxv2Url), token=(influxv2Token), org=(influxv2Org), bucket=(influxv2Bucket))
write_api = client.write_api(write_options=SYNCHRONOUS)
query_api = client.query_api()




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
      print( baro)
    senseHatPoints =  influxdb_client.Point("SenseHat").field("Barometer",(baro))
    write_api.write(bucket=(influxv2Bucket),org=(influxv2Org),record=senseHatPoints)
    baroPOINTS=senseHatPoints
     
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
    dracalBaroSL=dracalBaroFloat/pow(1-((Altitude)/44330.0),5.255)
    if DEBUG==True:
       print(dracalBaroSL)
    dracalPoints =  influxdb_client.Point("Dracal").field("Barometer",(dracalBaroSL)).field("Thermometer",(dracalTempFloat)).field("UnCorrectedBarometer",(dracalBaroFloat))
    write_api.write(bucket=(influxv2Bucket),org=(influxv2Org),record=dracalPoints)
    baroPOINTS=dracalPoints

# get last temp C
#tmpC=client.query(tempQueryC + thermometer)

tmpQuery = 'from(bucket:"weather")\
       |> range(start: -1m)\
       |> filter(fn:(r) => r._measurement == ' +(thermometer)+')\
       |> filter(fn:(r) => r._field == "temperature_C")\
       |> group()\
       |> last()'


tmpResult = query_api.query(org=(influxv2Org), query=tmpQuery)
results = []
for table in tmpResult:
    for record in table.records:
          results.append(record.get_value())

tmpC=results
tmpString=str(tmpC)
tempC=tmpString.replace("[","").replace("]","")
tempC=tempC.strip()
tempC=float(tempC)
tempPint=(tempC * units.degC)

## Get last humidity

tmpQuery = 'from(bucket:"weather")\
       |> range(start: -1m)\
       |> filter(fn:(r) => r._measurement == ' +(humidityStation)+')\
       |> filter(fn:(r) => r._field == "humidity")\
       |> group()\
       |> last()'


tmpResult = query_api.query(org=(influxv2Org), query=tmpQuery)
results = []
for table in tmpResult:
   for record in table.records:
    results.append(record.get_value())

tmpString=str(results)
humidityP=tmpString.replace("[","").replace("]","")
humidityPF=float(humidityP)
humidityPint=(humidityPF * units.percent)

## Get last windspeed

tmpQuery = 'from(bucket:"weather")\
       |> range(start: -1m)\
       |> filter(fn:(r) => r._measurement == ' +(windStation)+')\
       |> filter(fn:(r) => r._field == "wind_avg_km_h")\
       |> group()\
       |> last()'


tmpResult = query_api.query(org=(influxv2Org), query=tmpQuery)
results = []
for table in tmpResult:
    for record in table.records:
          results.append(record.get_value())


tmpString=(results)
tmpString=str(tmpString)
windKPH=tmpString.replace("[","").replace("]","")
windKPH=float(windKPH)
windPint=(windKPH * units.kilometer_per_hour)

## Get last winddir
tmpQuery = 'from(bucket:"weather")\
       |> range(start: -1m)\
       |> filter(fn:(r) => r._measurement == ' +(windStation)+')\
       |> filter(fn:(r) => r._field == "wind_dir_deg")\
       |> group()\
       |> last()'

tmpResult = query_api.query(org=(influxv2Org), query=tmpQuery)
results = []
for table in tmpResult:
    for record in table.records:
          results.append(record.get_value())

tmpString=str(results)
windDIR=tmpString.replace("[","").replace("]","")
windDIR=float(windDIR)
windDIRPint=(windDIR * units.degrees)

## get last windgust

tmpQuery = 'from(bucket:"weather")\
       |> range(start: -1m)\
       |> filter(fn:(r) => r._measurement == ' +(windStation)+')\
       |> filter(fn:(r) => r._field == "wind_max_km_h")\
       |> group()\
       |> last()'
tmpResult = query_api.query(org=(influxv2Org), query=tmpQuery)
results = []
for table in tmpResult:
    for record in table.records:
          results.append(record.get_value())

tmpString=str(results)
windGustKPH=tmpString.replace("[","").replace("]","")
windGustKPH=float(windGustKPH)
windGustPint=(windGustKPH * units.kilometer_per_hour)

## get last barometer

tmpQuery = 'from(bucket:"weather")\
       |> range(start: -1m)\
       |> filter(fn:(r) => r._measurement == ' +(baroStation)+')\
       |> filter(fn:(r) => r._field == "Barometer")\
       |> group()\
       |> last()'


tmpResult = query_api.query(org=(influxv2Org), query=tmpQuery)
results = []
for table in tmpResult:
    for record in table.records:
          results.append(record.get_value())

tmpString=str(results)
baroINHG=tmpString.replace("[","").replace("]","")
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
       print (heatindexPint, tempPint)
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



dewpointPOINTS  = influxdb_client.Point("Dewpoint").field("dewpoint",(dewPointPint.magnitude))
heatIndexPOINTS = influxdb_client.Point("Heatindex").field("heatindex",(heatindexPint.magnitude))
WBTIndexPOINTS  = influxdb_client.Point("Wet Bulb Temp").field("Wet Bulb Temp",(wetBulbPint.magnitude))
windchillPOINTS = influxdb_client.Point("Windchill").field("Windchill",(windchillPint.magnitude))

if DEBUG==False:
    
    write_api.write(bucket=(influxv2Bucket),org=(influxv2Org),record=dewpointPOINTS)
    write_api.write(bucket=(influxv2Bucket),org=(influxv2Org),record=heatIndexPOINTS)
    write_api.write(bucket=(influxv2Bucket),org=(influxv2Org),record=WBTIndexPOINTS)
    write_api.write(bucket=(influxv2Bucket),org=(influxv2Org),record=windchillPOINTS)

if DEBUG==True:
    print ("DEBUG: Not sending to INFLUX")
    print ("     This is what would have been sent")
    print ("     dewpoint : ",dewpointPOINTS)
    print ("     heatIndex: ",heatIndexPOINTS)
    print ("     Wet Bulb : ",WBTIndexPOINTS)
    print ("     Windchill : ",windchillPOINTS)
    print ("     Barometer : ",baroPOINTS)
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
