#!/bin/bash

function error_exit {
	echo -e "\033[31;1m * * * ERROR * * * :\033[0m $2"
	exit $1
}

SERIAL=`usbtenkiget -l  | grep "Serial" | cut -d ':' -f 3 | cut -d "'" -f 2`

echo "Using serial: $SERIAL"

CUR=0
INC=100

command -v bc || error_exit 1 "bc required but not found"

while true; do
	if [ $INC -eq 0 ]
	then
		echo -e "\033[32;1m * * OK * * \033[0m Calibration completed";
		echo
		exit 0
	fi

	./usbtenkiset -s $SERIAL set_rtd_cal -- $CUR || error_exit 1 "Could not set calibration value"

	VAL=`./usbtenkiget -i 0 -s $SERIAL`

	echo Calibration set to $CUR gives us $VAL

	POSITIVE=`echo "$VAL > 0" | bc`
	if [ $POSITIVE -ne 1 ]
	then
		let "INC = $INC / 2"
		let "CUR = $CUR - $INC"
		continue;
	fi

	let "CUR = $CUR + $INC"
done
