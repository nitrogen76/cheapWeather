#!/usr/bin/python3

##
## Influx to Wunderground
##
##
## Going back to v1, cuz v2/flux is awful.

## Imports
import datetime
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

## set some variables that some functions will use
now = datetime.datetime.now()
year= int(now.strftime('%Y'))
month=int(now.strftime('%m'))
day=  int(now.strftime('%d'))
hourago = (datetime.datetime.now() - datetime.timedelta(hours=1))
morning=datetime.datetime(year,month,day,0,0,0)
morningTS= morning.timestamp()*1000
morningTS=int(morningTS)
morning=str(morningTS)
houragoTS=hourago.timestamp()*1000
houragoTS=int(houragoTS)
hourago=str(houragoTS)

## Argumenmts
parser = argparse.ArgumentParser('description: generate some values every minute from values in influx, and also send data to wunderground.')
parser.add_argument('-d','--debug',action='store_true', help='Show debug information')
parser.add_argument('-n','--no_wunderground',action='store_false',help='Do not use wunderground')
args=parser.parse_args()
DEBUG=(args.debug)
useWunder=(args.no_wunderground)

## Set useWunder 
## FIXME: use bools instead
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
softwareVersion='&softwaretype=cheapWeather%20version%20pikel'

## If we aren't using wunderground, we don't need to get the variables
## FIXME: make this so if an entry is blank, it's not used

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

## Get config from cheapWeather.ini
thermometer=config.get('Query','thermometer')
humidityStation=config.get('Query','humiditystation')
windStation=config.get('Query','windstation')
baroStation=config.get('Query','barostation')
rainStation=config.get('Query','rainstation')
useDracal=config.get('Baro','useDracal')
dracalPath=config.get('Baro','dracalPath')
dracalSwitches=config.get('Baro','dracalSwitches')
useSensehat=config.get('Baro','useSenseHat')
Altitude=config.get('Baro','myAltitude')
Altitude=float(Altitude)
useWunderground=config.get('Wunderground','useWunderground')
pollutionMeasurement=config.get('Pollution','pollutionMeasurement')
pollutionLocation=config.get('Pollution','locationID')
pm01=config.get('Pollution','pm01')
pm02=config.get('Pollution','pm25')
pm10=config.get('Pollution','pm10')

## Initialize some values needed later.
altitudePint=(Altitude * units.meters)
minWindChillPint=(50 * units.degF)
minHeatIndexPint=(80 * units.degF)
minWindChillWindPint=(3 * units.mile_per_hour)


## initialize influxDB V1
client = InfluxDBClient(host=(influxHost), port=8086, username=(influxUser), password=(influxPass), database=(influxDB))

def stationToMSL(hpa,tempC,altM):
    output = hpa + ((hpa * 9.80665 * altM)/(287 * (273 + tempC + altM/400)))
    return output

## Function to fix wonky influx module outputs
def fixInfluxOutput(input):
    output=str(input)
    output=output.split(':')[5].split('}')[0]
    output=output.strip()
    output=float(output)
    return output

## Function to make a generic query from influx for a generic value
def getQuery(field,measurement,varType):
    output=client.query('select last(' + field + ') FROM ' + measurement + ' GROUP BY time(-1h)')
    output=fixInfluxOutput(output)
    if varType == 'int':
       output=int(output)
    elif varType == 'float':
       output = float(output)
    else:
       output=float(output)
    return output

## Specific query to get pollution values from Airgradient API scraping 
## via Telegraf

def getPollutionQuery(field,measurement,location):
    output=client.query('select last(' + field + ') FROM "' + measurement + '" where  "locationId"::field = ' + location + ' GROUP BY time(-1h)')
    output=fixInfluxOutput(output)
    if DEBUG==True:
       print ('DEBUG: pollution output' + field , output)
    return output

## Function to get rain differences between a specific time and "now"
## FIXME: These are currently set in variables up top
## but should probably be more interactive for more use cases
def getRainDiffQuery(measurement,delta):
    max=client.query('SELECT max("rain_mm")  FROM ' + measurement +' WHERE time >=  '+ delta +'ms and time <= now()' )
    max=fixInfluxOutput(max)
    min=client.query('SELECT min("rain_mm")  FROM ' + measurement +' WHERE time >=  '+ delta +'ms and time <= now()' )
    min=fixInfluxOutput(min)
    output=(max-min)
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
rainDiffHourPint=(getRainDiffQuery(rainStation,hourago) * units.millimeter)
rainDiffDayPint=(getRainDiffQuery(rainStation,morning) * units.millimeter)
lux=(getQuery('light_lux',windStation,int))
uv=(getQuery('uvi',windStation,int))
## lux/685 is the conversion factor for green light
#solarRadiation=(lux/685)
## lux/126.7 is for regular solar radiation
## FIXME: Build this into the function eventually.
## i think most meterologists prefer the 126.7 number the more I read.
## I got the 126.7 factor from AmbientWeather's website:
## https://ambientweather.com/faqs/question/view/id/1452/
solarRadiation=(lux/126.7)


## In debug mode, show us what was configured
if DEBUG==True:
   print("DEBUG: Configuration options passed from cheapWeather.ini")
   print("       useSensehat= ", useSensehat)
   print("       Altitude= ", Altitude)
   print("       useDracal= ", useDracal)
   print("       dracalSwitches= ", dracalSwitches)
   print("       dracalPath= ", dracalPath)
## if wundergrund was used, pick those variables up
## FIXME: make it so blank variables are ignored and
## will set these automatically
   if useWunder == True:
      print("       useWunderground= ", useWunderground)
      print("       wunderground user= ", wundergroundUser)
   print("       thermometer= ", thermometer)
   print("       humidity station= ", humidityStation)
   print("       wind station= ", windStation)
   print("       baro station= ", baroStation)

## Set this if we need to use it as above
if useWunder==True or useWunderground==1:
   WUurl = "https://weatherstation.wunderground.com/weatherstation/updateweatherstation.php?"
   WUcreds = "ID=" + wundergroundUser + "&PASSWORD="+ wundergroundPass

## Get barometer info and write it to the database
## Sensehat probably doesnt work, but it's garbage, please dont use it
## because i broke mine trying to test it.

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

    client.write_points(baroJSON)
else:
    if DEBUG==True:
       print("no Sensehat")
       print("useDracal is: " + useDracal)
if useDracal == "1":
    #run usbtenkiget to get the barometer reading with 19 decimals of precision
    result=subprocess.run([(dracalPath),(dracalSwitches), "-x6" ], capture_output=True)
    if result.returncode !=0:
        print ("Problem with",dracalPath,dracalPath," .  Do you have the executable?  is the device connected?")
        sys.exit(1)
    if result == None:
        print (dracalPath," returned a null value.  Is the device connected?")
        sys.exit(2)
    dracalBaro=(result.stdout).strip()
    dracalBaroFloat=float(dracalBaro)
    if DEBUG==True:
       print (dracalBaroFloat)
    #This sensor has a temp sensor, lets grab that data also, why not?
    temp=subprocess.run([(dracalPath),(dracalSwitches), "-x6","-i1" ], capture_output=True)
    dracalTemp=(temp.stdout)
    dracalTempFloat=float(dracalTemp)
    stationPressurePint=(dracalBaroFloat * units.inHg)
    stationTempPint    =(dracalTempFloat * units.degC)
    baroSL=stationToMSL(stationPressurePint.to('hPa').magnitude,stationTempPint.magnitude,altitudePint.magnitude)
#    baroSL=dracalBaroFloat/pow(1-((Altitude)/44330.0),5.255)
    print ('old conversion: ',dracalBaroFloat/pow(1-((Altitude)/44330.0),5.255))
    barohpaPint=(baroSL * units.hPa)
    print (barohpaPint)
    baroPint=barohpaPint.to('inHg')
    print ('baropint: ',baroPint)
    uncorrectedBaroPint=(dracalBaroFloat * units.inHg)
    if DEBUG==True:
       print(baroPint)
## We have to create our own JSON.
## How quaint!
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

## If temp is above what makes sense for windchill, set it to the temp.
if tempPint.magnitude >= minWindChillPint.to('degC').magnitude:
    windchillPint = tempPint
    if DEBUG==True:
        print ("DEBUG: temp value, ",(tempPint).to('degC')," too high to generate a windchill", (minWindTempPint).to('degC'))

## If windchill is higher than the temperature, thats a bug in
## Metpy, and we don't use the masking function.

if windchillPint.magnitude > tempPint.magnitude:
   windchillPint=tempPint
   if DEBUG==True:
      print ("DEBUG: Windchill was higher than temperature, which is wrong")

## Debugging output of values generated
if DEBUG==True:
   print('DEBUG: Values grabbed and generated')
   print('     TempF' , tempPint.to('degF'))
   print('     Humidity' , humidityPint)
   print('     Wind ', windPint.to('mile_per_hour'))
   print('     Direction ' ,  windDIRPint)
   print('     Gust ' ,  windGustPint.to('mile_per_hour'))
   print('     Sea Level Corrected Barometer ' , baroPint)
   print('     Station Pressure, ', uncorrectedBaroPint)
   print('     heat index: ',heatindexPint.to('degF'))
   print('     Windchill: ', windchillPint.to('degF'))
   print('     hourly rain= ',rainDiffHourPint)
   print('     rain for today= ',rainDiffDayPint)
   print('     Altitude: ', altitudePint)
   print('     luxquery: ',lux)
   print('     Solar Radiation: ',solarRadiation,'wm/2')
   print('     UV Index: ',uv)

## Create JSON output for influx V1 because V2/flux sucks 
## Again, the python modules are kinda iffy
## hopefully the v3 stuff is nicer when it comes out
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


## Write points to the database unless we're in DEBUG mode
if DEBUG==False:
    client.write_points(dewpointJSON)
    client.write_points(heatIndexJSON)
    client.write_points (WBTIndexJSON)
    client.write_points (windchillJSON)

## If we're in DEBUG mode, show what JSON we'd have sent
if DEBUG==True:
    print ("DEBUG: Not sending to INFLUX")
    print ("     This is what would have been sent")
    print ("     dewpoint : ",dewpointJSON)
    print ("     heatIndex: ",heatIndexJSON)
    print ("     Wet Bulb : ",WBTIndexJSON)
    print ("     Windchill : ",windchillJSON)
    print ("     Barometer : ",baroJSON)

## IF we're using wunderground, lets build the
## url we're going to use to report

## FIXME: make this more elegant, and if we're not sending fields
## don't put them here

if useWunder == True:
           wundergroundRequest=(WUurl + WUcreds +\
           "&dateutc=now&action=updateraw" +\
           "&humidity="       + str(humidityPint.magnitude) +\
           "&tempf="          + str(tempPint.to('degF').magnitude) +\
           "&winddir="        + str(windDIRPint.magnitude) +\
           "&windspeedmph= "  + str(windPint.to('mile_per_hour').magnitude) +\
           "&windgustmph="    + str(windGustPint.to('mile_per_hour').magnitude) +\
           "&baromin="        + str(baroPint.magnitude) +\
           "&dewptf="         + str(dewPointPint.to('degF').magnitude) +\
           "&rainin="         + str(rainDiffHourPint.to('inch').magnitude) +\
           "&dailyrainin="    + str(rainDiffDayPint.to('inch').magnitude) +\
           "&AqPM2.5="        + str(PM02Pint.magnitude) +\
           "&AqPM10="         + str(PM10Pint.magnitude) +\
           "&UV="             + str(round((uv))) +\
           "&solarradiation=" + str(solarRadiation) +\
           softwareVersion)

## Send the url we generated to the website if we're using wunderround and NOT
## in debug mode
if useWunder==True and DEBUG==False:
   httpstatus=requests.get(wundergroundRequest)
   print(("Received " + str(httpstatus.status_code) + " " + str(httpstatus.text)))

## If we're in DEBUG mode, show the URL we generated and would have sent.
if DEBUG==True and useWunder==True:
      print ("DEBUG: wunderground URL Generated but not sent in DEBUG mode")
      print ("     ",wundergroundRequest)

## If we're not using wunderground, make it clear in debug mode.
if useWunder==False and DEBUG==True:
   print ("DEBUG: Wunderground send disabled")


## FIXME
## When this gets more robust make it an application
## that'll run by itself and do calculations more than
## one minute apart
