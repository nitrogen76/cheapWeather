#!/usr/bin/python

## Script to pull information from Bryant AC from
## https://github.com/nebulous/infinitude

import configparser
import json
import requests
import dateutil.parser
import dateutil.parser
from datetime import datetime
import influxdb_client
from influxdb_client.client.write_api import SYNCHRONOUS


#### This is using info from another project.  
#### If you're grabbing this from my repo
#### just comment this out, or use the config from
#### nitrogen76/cheapWeather

# Get config info
config = configparser.ConfigParser()
config.read('/etc/cheapWeather.ini')

#### You can replace these variables with the actual settings
#### otherwise, just grab the config file from nitrogen76/cheapWeather

### Variables
influxv2Url=config.get('InfluxV2','url')
influxv2Bucket=config.get('InfluxV2','bucket')
influxv2Org=config.get('InfluxV2','org')
influxv2Token=config.get('InfluxV2','token')

## Requests

r=requests.get('http://hvac.dna.nurgle.net:3000/api/status')
requestODU=requests.get('http://hvac.dna.nurgle.net:3000/api/status/odu')
requestIDU=requests.get('http://hvac.dna.nurgle.net:3000/api/status/idu')
requestOAT=requests.get('http://hvac.dna.nurgle.net:3000/api/status/oat')
requestZone1=requests.get('http://hvac.dna.nurgle.net:3000/api/status/1')
requestZone2=requests.get('http://hvac.dna.nurgle.net:3000/api/status/2')
requestMessage=requests.get('http://hvac.dna.nurgle.net:3000/notifications.json')
## convert json into strings
rawJsonData= str(r.text)
rawJsonODU= str(requestODU.text)
rawJsonIDU= str(requestIDU.text)
rawJsonOAT= str(requestOAT.text)
rawJsonZone1= str(requestZone1.text)
rawJsonZone2= str(requestZone2.text)
rawMessage=str(requestMessage.text)
## load json strings into dicts for processing
jsonData=json.loads(rawJsonData)
jsonODU=json.loads(rawJsonODU)
jsonIDU=json.loads(rawJsonIDU)
jsonOAT=json.loads(rawJsonOAT)
jsonZone1=json.loads(rawJsonZone1)
jsonZone2=json.loads(rawJsonZone2)
jsonMessage=json.loads(rawMessage)
## Get filter usage percentage
filterLevel = jsonData['filtrlvl']
filterLevel=str(filterLevel)
filterLevel= (filterLevel.replace('[','').replace(']','').replace("'",""))

##print ("Message Timestamp ",jsonMessage[notifications])
messageTimestamp=str((jsonMessage['notifications'][0]['notification'][0]['timestamp'][0]))
message=str((jsonMessage['notifications'][0]['notification'][0]['message'][0]))
##print(messageTimestamp,': ',message)

parsed_time=dateutil.parser.parse(messageTimestamp)
##print (parsed_time,type(parsed_time))

## Get current outdoor unit stage
oduStat= jsonODU["odu"]
stage=str(oduStat['opstat'])
stage= (stage.replace('[','').replace(']','').replace("'","").replace("Stage ","").replace("off","0").replace("dehumidify","0.5"))

## Get current indoor unit info
iduStat=jsonIDU['idu']
cfm=str(iduStat['cfm'])
cfm=(cfm.replace('[','').replace(']','').replace("'","").replace("Stage ",""))

## Get outdoor unit temperature
oat=jsonOAT['oat']

## get zone 1 actual temperature
zone1Temp=str(jsonZone1['rt'])
zone1Temp=(zone1Temp.replace('[','').replace(']','').replace("'",""))

## get zone 2 actual temperature
zone2Temp= str(jsonZone2['rt'])
zone2Temp= (zone2Temp.replace('[','').replace(']','').replace("'",""))


## Prepare influx
client = influxdb_client.InfluxDBClient(url=(influxv2Url), token=(influxv2Token), org=(influxv2Org), bucket=(influxv2Bucket))
write_api = client.write_api(write_options=SYNCHRONOUS)
query_api = client.query_api()

## This is awful

stage=float(stage)
filterLevel=float(filterLevel)
cfm=float(cfm)
oat=float(oat)
zone1Temp=float(zone1Temp)
zone2Temp=float(zone2Temp)
## Debug printing
#print ("Stage",stage)
#print ("Filter Usage",filterLevel)
#print ("CFM", cfm)
#print ("Outdoor Temp", oat)
#print ("Downstairs Temp",zone1Temp)
#print ("Upstairs Temp",zone2Temp)

bryantPoints=influxdb_client.Point("Bryant").field("Stage",(stage)).field("Filter",(filterLevel)).field("CFM",(cfm)).field("Outdoor Temp",(oat)).field("Upstairs",(zone1Temp)).field("Downstairs",(zone2Temp))

##print (bryantPoints)

write_api.write(bucket=(influxv2Bucket),org=(influxv2Org),record=bryantPoints)
