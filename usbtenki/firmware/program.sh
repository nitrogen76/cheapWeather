#!/bin/bash

SERIAL=/dev/avr
UISP="uisp -dprog=stk500 -dserial=$SERIAL -dpart=ATmega8"
FUSE_H=0xc1
FUSE_L=0x1f

clear
pwd
echo Selected firmware: $1
echo "-- Ready. Press ENTER to program --"
read

while true
do

	while true
	do
		$UISP --wr_fuse_h=$FUSE_H --wr_fuse_l=$FUSE_L
		MAKERES=$?

		echo "Fuses result: $MAKERES"

		if [ $MAKERES -ne 0 ]
		then
			echo -en "\\033[31;1mFuse prog. Failed!\\033[0m;\n"
			echo "-- Press ENTER to RETRY --"
			read
			continue
		else
			echo -en "\\033[32;1mProg OK\\033[0m;\n"
		fi

		$UISP --erase --upload --verify if=$1
		
		echo "firmware result: $MAKERES"

		if [ $MAKERES -ne 0 ]
		then
			echo -en "\\033[31;1mProg. Failed!\\033[0m;\n"
			echo "-- Press ENTER to RETRY --"
			read
			continue
		else
			echo -en "\\033[32;1mProg OK\\033[0m;\n"
			break
		fi

	done

	echo "**********"
	echo "please check above for any error messages!"
	echo

	pwd
	echo "-- Ready. Press ENTER to program next board --"
	read
	clear

done
