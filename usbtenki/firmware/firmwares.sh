#!/bin/bash

while true
do

	echo "-------------------- USBTENKI --------------------"
	echo " a) adc.hex              ADC inputs only. PTH-01-P"
	echo " b) mcp9800.hex          MCP9800 Sensors"
	echo " c) sensirion.hex        SHT75 to USB cable"
	echo " d) sensirion_pc23.hex   PTH-01 with sensirion"
	echo " e) tsl2561.hex          TSL2561 to USB cable (single sensor)"
	echo " f) tsl2568.hex          TSL2568 to USB cable (single sensor)"
	echo " g) bs02.hex             BS02 sensor (sht clone) to USB cable"
	echo " h) lm75.hex             LM75 to USB"
	echo " i) se95.hex             SE95 to USB"
	echo " j) adt7410.hex          ADT7410 to USB"
	echo
	read -p "Your choice? " CHOICE

	case $CHOICE in
		a|A) ./program.sh adc.hex 	;;
		b|B) ./program.sh mcp9800.hex 	;;
		c|C) ./program.sh sensirion.hex 	;;
		d|D) ./program.sh sensirion_pc23.hex 	;;
		e|E) ./program.sh tsl2561.hex 	;;
		f|F) ./program.sh tsl2568.hex 	;;
		g|G) ./program.sh bs02.hex 	;;
		h|H) ./program.sh lm75.hex 	;;
		i|I) ./program.sh se95.hex 	;;
		j|J) ./program.sh adt7410.hex 	;;
	esac

done

