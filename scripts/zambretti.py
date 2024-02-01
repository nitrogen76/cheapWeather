#!/usr/bin/python3

##
## zambretti forecast 
##
##
## Going back to v1, cuz v2/flux is awful.

## Imports
import datetime
import argparse
import metpy.calc as mpcalc
from metpy.units import units

parser = argparse.ArgumentParser('parse input for a zambretti forecast')
parser.add_argument('-b',action='store',help='current barometer in inHg')
parser.add_argument('-d',action='store',help='barometer difference between 3 hrs in inHg')
parser.add_argument('-w',action='store',help='Wind direction in Degrees')
parser.add_argument('--debug',action='store_true',help='Show Debug info')

args=parser.parse_args()
DEBUG=(args.debug)
baro=float(args.b)
diff=float(args.d)
wind=int(args.w)

baro=(baro * units.inHg)
diff=(diff * units.inHg)


def zambretti(currentBaro,diffBaro,windDir):
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
## Trend requirement for dropping pressure
    if DEBUG==True:
       print (currentBaro,diffBaro,wind)
    if diffBaro.to('millibars').magnitude <= -1.6:
       if DEBUG == True:
          print ("trend for falling pressure met, going to next check")
          print ("pressDiff: ",round(diffBaro.to('millibars'),2))
## pressure requirement for dropping trend
       if currentBaro.to('millibars').magnitude > 985 and currentBaro.to('millibars').magnitude  < 1050:
           if DEBUG==True:
              print ("Pressure requirement for falling trend met.  Use this.")
              print ("pressNow: ",round(currentBaro.to('millibars'),2))
## Calculate Z
       z = 127-0.12*currentBaro.to('millibars').magnitude
       if DEBUG==True:
             print ("Z for this step is: ",round(z,2))
       else:
          if DEBUG==True:
             print ("trend for falling pressure NOT met")
             print ("pressNow: ",round(currentBaro.to('millibars'),2),"and pressDiff: ",round(diffBaro.to('millibars'),2))
             z=0

## Trend requirement for rising trend
    elif diffBaro.to('millibars').magnitude >= 1.6:
         if DEBUG==True:
            print ("Trend requirement for rising trend met.  Go to next check")
            print ("pressDiff: ",round(diffBaro.to('millibars'),2))
 ## Pressure requirement for rising trend
         if currentBaro.to('millibars').magnitude > 947 and currentBaro.to('millibars').magnitude < 1030:
          if DEBUG==True:
             print ("Pressure requirement for rising trend met.  use me.")
             print ("pressNow: ",round(currentBaro.to('millibars'),2))
## Calculate Z
          z = 185-0.16*currentBaro.to('millibars').magnitude
          if DEBUG==True:
             print ("Z for this step is: ",round(z,2))
         else:
              if DEBUG==True:
                 print ("Trend for rising not met")
                 print ("pressNow: ",round(currentBaro.to('millibars'),2),"and pressDiff: ",round(diffBaro.to('millibars'),2))
                 z=0

## Steady pressure
    elif  diffBaro.to('millibars').magnitude >= -1.6 and diffBaro.to('millibars').magnitude <= 1.6:
          if DEBUG==True:
             print ("Trend met for steady trend. going to next check.")
             print ("pressDiff: ",round(diffBaro.to('millibars'),2))
## Pressure requirement for steady trend
          if  currentBaro.to('millibars').magnitude >947 and currentBaro.to('millibars').magnitude < 1030:
              if DEBUG==True:
                 print ("Pressure requirement for steady trend met. Use me.")
                 print ("pressNow: ",round(currentBaro.to('millibars'),2))
## Calculate Z
              z = 144-0.13 *currentBaro.to('millibars').magnitude
              if DEBUG==True:
                 print ("Z for this step is: ",round(z,2))

          else:
              if DEBUG==True:
                 print ("Trend for steady not met")
                 z=0

    else:
        if DEBUG==True:
           print ("No requirements met, don't use anything")
           print ("pressNow: ",round(currentBaro.to('millibars'),2),"and pressDiff: ",round(diffBaro.to('millibars'),2))


    if (windDir >= 135) and (windDir <= 225):
        if DEBUG==True:
           print ("applying z+2 for wind direction of ", windDir)
        z=z+2
    elif (windDir >= 315) or (windDir <=45):
        if DEBUG==True:
           print ("applying no z manipulation for wind direction of ",windDir)
        z=z+0
    else:
        if DEBUG==True:
           print ("applying z+1 for wind direction of ",windDir)
        z=z+1


    z = round(z)
    if DEBUG==True:
       print ('Your value for Z(rounded!) is = ',z)
       print ("Your forecast is: ",forecastDict[z])
    return (z,forecastDict[z])

zambResponse=zambretti(baro,diff,wind)
print ("     Your zambretti code is: ",zambResponse[0])
print ("     Your zambretti forecast is ",zambResponse[1])
