/* usbtenkiget: A command-line tool for USBTenki sensors.
 * Copyright (C) 2007-2016  Raphael Assenat <raph@raphnet.net>
 * Copyright (C) 2018-2020  Dracal technologies inc.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <getopt.h>

#include "usbtenki.h"
#include "../common/usbtenki_cmds.h"
#include "../common/usbtenki_version.h"

int g_verbose = 0;

static void printVersion(void)
{
	printf("Usbtenkisetup version %s\n", USBTENKI_VERSION);
	printf("Copyright (C) 2007-2017, Raphael Assenat\n");
	printf("Copyright (C) 2018-2020, Dracal technologies inc.\n\n");
	printf("This software comes with ABSOLUTELY NO WARRANTY;\n");
	printf("You may redistribute copies of it under the terms of the GNU General Public License\n");
	printf("http://www.gnu.org/licenses/gpl.html\n");
}

static void printUsage(void)
{
	printf("Usage: ./usbtenkiset [options] command value\n");
	printf("\nValid options:\n");
	printf("    -V          Display version information\n");
	printf("    -v          Run in verbose mode\n");
	printf("    -h          Displays help\n");
	printf("    -s serno    Use USB sensor with matching serial number. Required.\n");
	printf("    -f          Operates on the first device found.\n");
	printf("\nValid commands:\n");
	printf("    setadcchip  adc_id chip\n");
	printf("    setserial   serial (6 characters)\n");
	printf("    setref      ref_id (0=AVCC, 1=AREF)\n");
	printf("    set_rtd_cal	value (-32768 to 32767)\n");
	printf("    em1_config  max_current  calibration\n");
	printf("    do_zero          Device specific effect.\n");
	printf("    dxc200_kz ppm    Set zero in known gas concentration (for DXC-200)\n");
	printf("    sht31_rate	rate (0: 0.5 MPS, 1: 1 MPS, 2: 2 MPS, 3: 4 MPS, 4: 10 MPS)\n");
	printf("                (MPS is measurements per second)\n");
	printf("    set_thermocouple_type channel type\n");
	printf("       (where channel is the port number [counted from 0])\n");
	printf("       (where type is the thermocouple type K, J, T, N, S, E, B, or R)\n");
	printf("    get_thermocouple_type channel\n");
	printf("    unlock_cal code   Unlock calibration using code\n");
	printf("    bootloader                 Enter bootloader mode\n");
	printf("                               (not supported by all devices)\n");
	printf("    set_protocol  protocol_id  Switch the device to another protocol.\n");
	printf("                               0: Standard (usbtenki)\n");
	printf("                               1: NMEA, USB CDC-ACM (Virtual COMM port)\n");
	printf("                               (not supported by all devices)\n");
	printf("    reset           Restart the firmware (not supported by all devices)\n");
}

#define MAX_EXTRA_ARGS	8

int main(int argc, char **argv)
{
	int res, i;
	int use_first = 0;
	char *use_serial = NULL;
	int n_extra_args=0;
	char *eargv[MAX_EXTRA_ARGS];
	USBTenki_dev_handle hdl = NULL;
	struct usb_device *cur_dev, *dev=NULL;
	struct USBTenki_list_ctx *listContext;
	struct USBTenki_info info;
	unsigned char repBuf[8];
	int retval = 0;

	while (-1 != (res=getopt(argc, argv, "Vvhfs:")))
	{
		switch (res)
		{
			case 'f':
				use_first = 1;
				break;
			case 'v':
				g_verbose = 1;
				break;
			case 'V':
				printVersion();
				return 0;
			case 'h':
				printUsage();
				return 0;
			case 's':
				use_serial = optarg;
				break;
		}
	}

	n_extra_args = argc-optind;
	if (!use_serial && !use_first) {
		fprintf(stderr, "Serial number is required.\n");
		return 1;
	}

	if (g_verbose) {
		printf("Extra args: %d\n", n_extra_args);
	}

	for (i=optind; i<argc; i++) {
		eargv[i-optind] = argv[i];
		if (g_verbose)
			printf("  %d: %s\n", i-optind, eargv[i-optind]);
	}

	usbtenki_init();

	listContext = usbtenki_allocListCtx();

	while ((cur_dev=usbtenki_listDevices(&info, listContext))) {
		if (use_serial) {
			if (strcmp(use_serial, info.str_serial)==0) {
				dev = cur_dev;
				break;
			}
		}
		if (use_first) {
			dev = cur_dev;
			break;
		}
	}

	usbtenki_freeListCtx(listContext);

	if (!dev) {
		fprintf(stderr, "Could not locate device with serial '%s'. Try usbtekiget -l\n",
							use_serial);
		return 1;
	}

	if (n_extra_args<1) {
		fprintf(stderr, "No command specified\n");
		return 1;
	}

	hdl = usbtenki_openDevice(dev);
	if (!hdl) {
		fprintf(stderr, "Cannot open device\n");
		return 2;
	}

	/**************** Setref ****************/
	if (strcmp(eargv[0], "setref")==0) {
		int ref_id;
		char *e;

		/* printf("    setadcchip  adc_id chip\n"); */

		if (n_extra_args<2) {
			fprintf(stderr, "Missing arguments to command\n");
			retval = 1;
			goto cleanAndExit;
		}

		ref_id = strtol(eargv[1], &e, 0);
		if (e==eargv[1]) {
			fprintf(stderr, "Bad ref id\n");
			retval = 1;
			goto cleanAndExit;
		}

	if (g_verbose)
			printf("Setting adc ref to %d\n", ref_id);

		res = usbtenki_command(hdl, USBTENKI_SET_ADC_REF,
								(ref_id & 0xff), repBuf, sizeof(repBuf));
		if (res!=0) {
			fprintf(stderr, "Error setting adc ref to %d\n",
								ref_id);
			retval = 2;
		}

		goto cleanAndExit;
	}

	/**************** Do zero ****************/
	if (strcmp(eargv[0], "do_zero")==0) {

		res = usbtenki_command(hdl, USBTENKI_ZERO, 0, repBuf, sizeof(repBuf));
		if (res!=0) {
			fprintf(stderr, "Error doing zero.n");
			retval = 2;
		}

		goto cleanAndExit;
	}



	/**************** Setref ****************/
	if (strcmp(eargv[0], "set_rtd_cal")==0) {
		int ref_id;
		char *e;

		/* printf("    setadcchip  adc_id chip\n"); */

		if (n_extra_args<2) {
			fprintf(stderr, "Missing arguments to command\n");
			retval = 1;
			goto cleanAndExit;
		}

		ref_id = strtol(eargv[1], &e, 0);
		if (e==eargv[1]) {
			fprintf(stderr, "Bad ref id\n");
			retval = 1;
			goto cleanAndExit;
		}
		
	if (g_verbose) 
			printf("Setting rtd calibration to %d\n", ref_id);

		res = usbtenki_command(hdl, USBTENKI_SET_RTD_CORR, 
								(ref_id & 0xffff), repBuf, sizeof(repBuf));
		if (res!=0) {
			fprintf(stderr, "Error setting adc calibration to %d\n",
								ref_id);
			retval = 2;
		}

		goto cleanAndExit;
	}

	/**************** Setadcchip ****************/
	if (strcmp(eargv[0], "setadcchip")==0) {
		int adc_id, chip_id;
		char *e;

		/* printf("    setadcchip  adc_id chip\n"); */

		if (n_extra_args<3) {
			fprintf(stderr, "Missing arguments to command\n");
			retval = 1;
			goto cleanAndExit;
		}

		adc_id = strtol(eargv[1], &e, 0);
		if (e==eargv[1]) {
			fprintf(stderr, "Bad adc id\n");
			retval = 1;
			goto cleanAndExit;
		}

		chip_id = strtol(eargv[2], &e, 0);
		if (e==eargv[1]) {
			fprintf(stderr, "Bad chip id\n");
			retval = 1;
			goto cleanAndExit;
		}

		if (g_verbose) 
			printf("Setting adc channel %d to chip_id %d\n", adc_id, chip_id);

		res = usbtenki_command(hdl, USBTENKI_SET_ADC_CHIP, 
								(adc_id & 0xff) | ((chip_id&0xff)<<8), repBuf, sizeof(repBuf));
		if (res!=0) {
			fprintf(stderr, "Error setting channel %d to chip_id %d\n",
								adc_id, chip_id);
			retval = 2;
		}

		goto cleanAndExit;
	}


	/***************** Setserial ***************/
	if (strcmp(eargv[0], "setserial")==0) {
		int i, len;

		if (n_extra_args<2) {
			fprintf(stderr, "No serial number specified\n");
			retval = 1;
			goto cleanAndExit;
		}

		len = strlen(eargv[1]);
		if (len != 6) {
			fprintf(stderr, "Serial number must be 6 character long\n");
			retval = 1;
			goto cleanAndExit;
		}

		for (i=0; i<len; i++) {	
			if (g_verbose) 
				printf("Setting serial number character '%c'\n", eargv[1][i]);

			res = usbtenki_command(hdl, USBTENKI_SET_SERIAL, 
									(i&0xff) | (eargv[1][i]<<8), repBuf, sizeof(repBuf));
			if (res!=0) {
				fprintf(stderr, "Error writing character '%c'. (%d)\n",
								eargv[1][i], res);
				retval = 2;
				goto cleanAndExit;
			}
		}
		/* index 0xff means store to eeprom. */
		res = usbtenki_command(hdl, USBTENKI_SET_SERIAL, 
								(0xff) | (eargv[1][i]<<8), repBuf, sizeof(repBuf));
		if (res != 0) {
			retval = 2;
			goto cleanAndExit;
		}
		
		goto cleanAndExit;
	}

	/*************** EM1 config ******************/
	if (strcmp(eargv[0], "em1_config")==0) {
		int max_current;
		int calibration;
		char *e;
		
		/* printf("    em1_config  max_current  calibration\n"); */

		if (n_extra_args<3) {
			fprintf(stderr, "Missing arguments to command\n");
			retval = 1;
			goto cleanAndExit;
		}

		max_current = strtol(eargv[1], &e, 0);
		if (e==eargv[1]) {
			fprintf(stderr, "Invalid max current\n");
			retval = 1;
			goto cleanAndExit;
		}

		calibration = strtol(eargv[2], &e, 0);
		if (e==eargv[1]) {
			fprintf(stderr, "Invalid calibration\n");
			retval = 1;
			goto cleanAndExit;
		}

		if (g_verbose) 
			printf("Setting em1 config: Max current=%d,  calibration=0x%04x\n", max_current, calibration);

		res = usbtenki_command(hdl, USBTENKI_SET_EM1_CALIBRATION, 
								calibration, repBuf, sizeof(repBuf));
		if (res!=0) {
			fprintf(stderr, "Error setting calibration\n");
			retval = 2;
		}

		res = usbtenki_command(hdl, USBTENKI_SET_EM1_MAX_CURRENT, 
								max_current, repBuf, sizeof(repBuf));
		if (res!=0) {
			fprintf(stderr, "Error setting calibration\n");
			retval = 2;
		}

		goto cleanAndExit;
	}

	/**************** SHT31 rate *****************/
	if (strcmp(eargv[0], "sht31_rate")==0) {
		int rate_value;
		char *e;

		if (n_extra_args<2) {
			fprintf(stderr, "Missing arguments to command\n");
			retval = 1;
			goto cleanAndExit;
		}

		rate_value = strtol(eargv[1], &e, 0);
		if (e==eargv[1] || rate_value < 0 || rate_value > 4) {
			fprintf(stderr, "Bad rate value\n");
			retval = 1;
			goto cleanAndExit;
		}

		if (g_verbose)
			printf("Setting SHT31 poll rate to value %d\n", rate_value);

		res = usbtenki_command(hdl, USBTENKI_SET_SHT31_RATE, rate_value, repBuf, sizeof(repBuf));
		if (res!=0) {
			fprintf(stderr, "Error setting SHT31 poll rate to value %d\n",
								rate_value);
			retval = 2;
		}

		goto cleanAndExit;
	}

	/************ Configuring thermocouple type **************/
	if (strcmp(eargv[0], "set_thermocouple_type")==0) {
		int chn, type;
		char *e;

		if (n_extra_args<3) {
			fprintf(stderr, "Missing arguments to command\n");
			retval = 1;
			goto cleanAndExit;
		}

		chn = strtol(eargv[1], &e, 0);
		if (e == eargv[1] || chn < 0) {
			fprintf(stderr, "Bad channel value\n");
			retval = 1;
			goto cleanAndExit;
		}

		type = thermocoupleStringToType(eargv[2]);
		if (type < THERMOCOUPLE_TYPE_MIN || type > THERMOCOUPLE_TYPE_MAX) {
			fprintf(stderr, "Bad thermocouple type\n");
			retval = 1;
			goto cleanAndExit;
		}

		if (g_verbose) {
			printf("Setting thermocouple[%d] to type %d\n", chn, type);
		}

		res = usbtenki_command(hdl, USBTENKI_SET_THERMOCOUPLE_TYPE, chn | type << 8, repBuf, sizeof(repBuf));
		if (res != 0) {
			fprintf(stderr, "Error configuring thermocouple\n");
			retval = 2;
		}

		goto cleanAndExit;
	}

	/************ Reading back the configured thermocouple type ***************/
	if (strcmp(eargv[0], "get_thermocouple_type")==0) {
		int chn;
		char *e;

		if (n_extra_args < 2) {
			fprintf(stderr, "Missing arguments to command\n");
			retval = 1;
			goto cleanAndExit;
		}

		chn = strtol(eargv[1], &e, 0);
		if (e == eargv[1] || chn < 0) {
			fprintf(stderr, "Bad channel value\n");
			retval = 1;
			goto cleanAndExit;
		}

		res = usbtenki_command(hdl, USBTENKI_GET_THERMOCOUPLE_TYPE, chn, repBuf, sizeof(repBuf));
		if (res < 1) {
			fprintf(stderr, "Error reading thermocouple configuration\n");
			retval = 2;
			goto cleanAndExit;
		}

		printf("Thermocouple type configured for channel %d: Type %s\n", chn,
				thermocoupleTypeToString(repBuf[0]));

		goto cleanAndExit;
	}

	/**************** Set zero in known gas concentration (DXC-200 CO2 Sensor) ****************/
	if (strcmp(eargv[0], "dxc200_kz")==0) {
		int ppm;
		char *e;

		if (n_extra_args<2) {
			fprintf(stderr, "Missing arguments to command\n");
			retval = 1;
			goto cleanAndExit;
		}

		ppm = strtol(eargv[1], &e, 0);
		if (e == eargv[1]) {
			fprintf(stderr, "Bad PPM value\n");
			retval = 1;
			goto cleanAndExit;
		}

		printf("Setting zero in known gas concentration of %d ppm\n", ppm);

		res = usbtenki_command(hdl, USBTENKI_ZERO_IN_KNOWN_PPM, ppm, repBuf, sizeof(repBuf));
		if (res != 0) {
			fprintf(stderr, "Error setting zero in known concentration\n");
			retval = 2;
		}

		goto cleanAndExit;
	}

	/**************** Unlock calibration ****************/
	if (strcmp(eargv[0], "unlock_cal")==0) {
		int code;
		char *e;

		if (n_extra_args<2) {
			fprintf(stderr, "Missing arguments to command\n");
			retval = 1;
			goto cleanAndExit;
		}

		code = strtol(eargv[1], &e, 0);
		if (e == eargv[1]) {
			fprintf(stderr, "Malformed code\n");
			retval = 1;
			goto cleanAndExit;
		}

		res = usbtenki_command(hdl, USBTENKI_UNLOCK_CALIBRATION, code, repBuf, sizeof(repBuf));
		if (res != 0) {
			fprintf(stderr, "Error setting zero in known concentration\n");
			retval = 2;
		}

		goto cleanAndExit;
	}

	/**************** set_protocol *****************/
	if (strcmp(eargv[0], "set_protocol")==0) {
		int protocol_id;
		char *e;

		if (n_extra_args<2) {
			fprintf(stderr, "Missing arguments to command\n");
			retval = 1;
			goto cleanAndExit;
		}

		protocol_id = strtol(eargv[1], &e, 0);
		if (e==eargv[1] || protocol_id < 0 || protocol_id > 255) {
			fprintf(stderr, "Bad protocol id\n");
			retval = 1;
			goto cleanAndExit;
		}

		if (g_verbose)
			printf("Setting protocol id to %d\n", protocol_id);

		res = usbtenki_command(hdl, USBTENKI_SET_PROTOCOL, protocol_id, repBuf, sizeof(repBuf));
		if (res!=0) {
			fprintf(stderr, "Error setting protocol id to %d\n",
								protocol_id);
			retval = 2;
		}

		goto cleanAndExit;
	}

	/**************** Reset *****************/
	if (strcmp(eargv[0], "reset")==0) {
		usbtenki_command(hdl, USBTENKI_RESET_FIRMWARE, 0, repBuf, sizeof(repBuf));

		goto cleanAndExit;
	}

	/**************** Bootloader *****************/
	if (strcmp(eargv[0], "bootloader")==0) {
		usbtenki_command(hdl, USBTENKI_BOOTLOADER, 0xB007, repBuf, sizeof(repBuf));

		goto cleanAndExit;
	}

	fprintf(stderr, "Unknow command '%s'\n", eargv[0]);

	return 1;

cleanAndExit:
	if (hdl)
		usbtenki_closeDevice(hdl);

	return retval;
}
