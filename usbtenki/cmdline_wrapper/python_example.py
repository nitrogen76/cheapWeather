#!/usr/bin/python

import sys,subprocess

# Note: usbtenkiget assumed to be in the path
# Arguments passed to -i (0,1,2) here need to be updated to fit
# your scenario. You may also specify a serial number by adding
# the -s argument.

# If usbtenkiget exits with a non-zero values, the subprocess.CalledProcessError
# exception will be raised. Catch it.
try:
    p = subprocess.check_output(["usbtenkiget","-i","0,1,2"])
except subprocess.CalledProcessError:
    print "usbtenkiget error"
    sys.exit(1)

fields = p.split(",");

# This example expects the following output:
#
# 24.48, 61.56, 100.40
#
# Where fields are temperature, rh and pressure.
#
# Detect errors by checking if the exact expected number
# fields was returned.
if len(fields) < 3:
    print "Error reading sensor"
    sys.exit(2)

# Convert the fields from strings to floating point values.
# This step is necessary, otherwise math on values will not
# be possible.
temperature = float(fields[0])
rh = float(fields[1])
pressure = float(fields[2])

# Display values
print "Temperature (C):", temperature
print "RH......... (%):", rh
print "Pressure..(kPa):", pressure

print "Temperature (F):",temperature*9/5+32

sys.exit(0)
