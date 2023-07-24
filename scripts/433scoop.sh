#!/bin/bash
## This isn't really usable as a script, it's just here to show you commandline examples.
rtl_433 -d:00000433 -M time:unix:usec:utc -R 142 -R 40 -R 73 -R 113 -R 42 -M protocol -M level -C si -F "influx://influxDB.ass.nurgle.net:8086/write?db=rtl433&p=2sLkJHH8Uh0Y36pTC&u=weather" &
## I'll do this better in python with the .ini files soon.
