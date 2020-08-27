#!/bin/bash
##
## Run this "script" out of cron every minute to send data to Wunderground.

WUNDERGROUND_UID=Placeholder
WUNDERGROUND_PWD=PLACEHOLDER
INFLUX_UID=
INFLUX_PWD=
INFLUX_DB=

----- Cut here ----
tempF=0
winddir=0
windspeed=0
windgust=0
humidity=0


tempF=`influx -database ${INFLUX_DB} -username ${INFLUX_UID} -password ${INFLUX_PWD}  -execute 'SELECT last("temperature_C") *1.8+32   FROM "${THERMOMETER}"'  --format=csv |sed 1d  |cut -d ',' -f 3,4`

humidity=`influx -database ${INFLUX_DB} -username ${INFLUX_UID} -password ${INFLUX_PWD} -execute 'SELECT last("humidity")   FROM "${THERMOMETER}"'  --format=csv |sed 1d  |cut -d ',' -f 3,4`

windspeed=`influx -database ${INFLUX_DB} -username ${INFLUX_UID} -password ${INFLUX_PWD} -execute 'SELECT last("wind_avg_km_h") * 0.6213712 FROM "${WEATHERSTATION}"'  --format=csv |sed 1d  |cut -d ',' -f 3,4`

winddir=`influx -database ${INFLUX_DB} -username ${INFLUX_UID} -password ${INFLUX_PWD}  -execute 'SELECT last("wind_dir_deg")  FROM "${WEATHERSTATION}"'  --format=csv |sed 1d  |cut -d ',' -f 3,4`

windgust=`influx -database ${INFLUX_DB} -username ${INFLUX_UID} -password ${INFLUX_PWD} -execute 'SELECT top("wind_avg_km_h", 1) * 0.6213712 FROM "${WEATHERSTATION}" WHERE time > now() - 15m'  --format=csv |sed 1d  |cut -d ',' -f 3,4`


curl "https://weatherstation.wunderground.com/weatherstation/updateweatherstation.php?ID=${WUNDERGROUND_UID}&PASSWORD=${WUNDERGROUND_PWD}&dateutc=now&tempf=${tempF}&winddir=${winddir}&windspeedmph=${windspeed}&windgustmph=${windgust}&humidity=${humidity}&weather=&clouds=&softwaretype=vws%20versionxx&action=updateraw"

