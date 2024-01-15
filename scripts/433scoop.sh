#!/bin/bash
## This isn't really usable as a script, it's just here to show you commandline examples.
rtl_433 -d:00000433 -M time:unix:usec:utc -R 142 -R 40 -R 73 -R 113 -R 42 -M ID -M protocol -M level -C si -F "influx://${Your influx host}:8086/write?db=${your influx dbname}&p=${your influx password}TC&u=${your influx user}"
## I'll do this better in python with the .ini files soon.
