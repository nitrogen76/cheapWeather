#!/usr/bin/python3

## attempt at zambretti algorithm
## Leo Green
## leo@unixjerk.org
##

## Imports
import datetime
import configparser
import metpy.calc as mpcalc
from metpy.units import units
from influxdb import InfluxDBClient

# Get config info
config = configparser.ConfigParser(allow_no_value=True)
config.read('/etc/cheapWeather.ini')
### Variables
softwareVersion='&softwaretype=cheapWeather%20version%20pikel'

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

forecastDict={1:'Settled Fine',\
              2:'Fine Weather',\
              3:'Fine, Becoming Less Settled',\
              4:'Fairly Fine, Showery Later',\
              5:'Showery, Becoming More Unsettled',\
              6:'Unsettled, Rain Later',\
              7:'Rain at Times, Worse Later',\
              8:'Rain at Times, Becoming Very Unsettled',\
              9:'Very Unsettled, Rain',\
              10:'Settled Fine',\
              11:'Fine Weather',\
              12:'Fine, Possibly Showers',\
              13:'Fairly Fine, Showers Likely',\
              14:'Showery, Bright Intervals',\
              15:'Changeable, Some Rain',\
              16:'Unsettled, Rain at Times',\
              17:'Rain at Frequent Intervals',\
              18:'Very Unsettled, Rain',\
              19:'Stormy, Much Rain',\
              20:'Settled Fine',\
              21:'Fine Weather',\
              22:'Becoming Fine',\
              23:'Fairly Fine, Improving',\
              24:'Fairly Fine, Possibly Showers Early',\
              25:'Showery Early, Improving',\
              26:'Changeable, Mending',\
              27:'Rather Unsettled, Clearing Later',\
              28:'Unsettled, Probably Improving',\
              29:'Unsettled, Short Fine Intervals',\
              30:'Very Unsettled, Finer at Times',\
              31:'Stormy, Possibly Improving',\
              32:'Stormy, Much Rain',}

## Initialize influx
client = InfluxDBClient(host=(influxHost), port=8086, username=(influxUser), password=(influxPass), database=(influxDB))


## Function to fix wonky influx module outputs
def fixInfluxOutput(input):
    output=str(input)
    output=output.split(':')[5].split('}')[0]
    output=output.strip()
    output=float(output)
    return output

## Set these but make it flexible for other uses
measure='Dracal'
field='Barometer'
delta=30
## Get barometer differences
## 
## function to get average barometer reading between two time periods

def getBaroDiffQuery(measurement,field,start,delta):

    if start == 0:
       startDT=datetime.datetime.now()
       startTS=int(startDT.timestamp()*1000)
    else: 
       startDT=datetime.datetime.now() - datetime.timedelta(minutes=start)
       startTS=int(startDT.timestamp()*1000)
    end=start+delta
    deltaDT=datetime.datetime.now() - datetime.timedelta(minutes=end)
    deltaTS=int(deltaDT.timestamp()*1000)
    deltaSTR=str(deltaTS)
    startSTR=str(startTS)
    first= client.query ('SELECT last("' + field +'")  FROM "' + measurement +'"')
    last  = client.query ('SELECT first("' + field +'")  FROM "' + measurement +'" WHERE time >=  '+ deltaSTR +'ms and time <= ' + startSTR +'ms' )
    first =fixInfluxOutput(first)
    last  =fixInfluxOutput(last)
    output=first-last
    return float(output)

def getQuery(field,measurement,varType):
    output=client.query('select last("' + field + '") FROM ' + measurement )
    output=fixInfluxOutput(output)
    if varType == 'int':
       output=int(output)
    elif varType == 'float':
       output = float(output)
    else:
       output=float(output)
    return output

pressNow       = getQuery('Barometer','Dracal','float')
pressDiff      = getBaroDiffQuery(measure,field,0,180)
windDir        = getQuery('wind_dir_deg',windStation,'int')
pressNowPint   = (pressNow * units.inHg)
pressDiffPint  = (pressDiff * units.inHg)
## Change this up to millibars because that's what the algorithm uses natively


## Trend requirement for dropping pressure
if pressDiffPint.to('millibars').magnitude <= -1.6:
   print ("trend for falling pressure met, going to next check")
   print ("pressDiff: ",round(pressDiffPint.to('millibars'),2))
## pressure requirement for dropping trend
   if pressNowPint.to('millibars').magnitude > 985 and pressNowPint.to('millibars').magnitude  < 1050:
      print ("Pressure requirement for falling trend met.  Use this.")
      print ("pressNow: ",round(pressNowPint.to('millibars'),2))
## Calculate Z
      z = 127-0.12*pressNowPint.to('millibars').magnitude
      print ("Z for this step is: ",z)

   else:
      print ("trend for falling pressure NOT met")
      print ("pressNow: ",round(pressNowPint.to('millibars'),2),"and pressDiff: ",round(pressDiffPint.to('millibars'),2))

## Trend requirement for rising trend
elif pressDiffPint.to('millibars').magnitude >= 1.6:
    print ("Trend requirement for rising trend met.  Go to next check")
    print ("pressDiff: ",round(pressDiffPint.to('millibars'),2))
 ## Pressure requirement for rising trend
    if pressNow.to('millibars').magnitude > 947 and pressNow.to('millibars').magnitude < 1030:
       print ("Pressure requirement for rising trend met.  use me.")
       print ("pressNow: ",round(pressNowPint.to('millibars'),2))
## Calculate Z
       z = 185-0.16*pressNowPint.to('millibars').magnitude
       print ("Z for this step is: ",z)
    else:
       print ("Trend for rising not met")
       print ("pressNow: ",round(pressNowPint.to('millibars'),2),"and pressDiff: ",round(pressDiffPint.to('millibars'),2))

## Steady pressure
elif  pressDiffPint.to('millibars').magnitude >= -1.6 and pressDiff.to('millibars').magnitude <= 1.6:
   print ("Trend met for steady trend. going to next check.")
   print ("pressDiff: ",pressDiffPint.to('millibars'))
## Pressure requirement for steady trend
   if  pressNowPint.to('millibars').magnitude >947 and pressNowPint.to('millibars').magnitude < 1030:
        print ("Pressure requirement for steady trend met. Use me.")
        print ("pressNow: ",pressNowPint.to('millibars'))
## Calculate Z  
        z = 144-0.13 *pressNowPint.to('millibars').magnitude
        print ("Z for this step is: ",z)

   else:
        print ("Trend for steady not met")

else:
    print ("No requirements met, don't use anything")
    print ("pressNow: ",round(pressNowPint.to('millibars'),2),"and pressDiff: ",round(pressDiffPint.to('millibars'),2))


if (windDir >= 135) and (windDir <= 225): 
   print ("applying z+2 for wind direction of ", windDir)
   z=z+2 
elif (windDir >= 315) or (windDir <=45):
   print ("applying no z manipulation for wind direction of ",windDir)
   z=z+0
else:
   print ("applying z+1 for wind direction of ",windDir)
   z=z+1


z = round(z)
print ('Z = ',z)
print ("Your forecast is: ",forecastDict[z])

