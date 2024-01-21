/* usbtenkiget: A command-line tool for USBTenki sensors.
 * Copyright (C) 2007-2017  Raphael Assenat <raph@raphnet.net>
 * Copyright (C) 2018-2020  Dracal technologees inc.
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
#include <unistd.h>

#if defined(WINDOWS_VERSION) || defined(WIN32)
#include <windows.h>
#define usleep(t) Sleep((t)/1000)
#endif

#include <math.h>

#include "usbtenki.h"
#include "../common/usbtenki_version.h"
#include "../common/usbtenki_cmds.h"
#include "../library/usbtenki_units.h"
#include "timestamp.h"

#define DEFAULT_CHANNEL_ID		0
#define DEFAULT_NUM_SAMPLES 	1
#define DEFAULT_LOG_INTERVAL	1000
#define MAX_CHANNELS			256
#define DEFAULT_DECIMAL_DIGITS	2


int g_verbose = 0;

struct USBTenki_unitPreferences g_unit_prefs = {
	.temperature = TENKI_UNIT_CELCIUS,
	.pressure = TENKI_UNIT_KPA,
	.frequency = TENKI_UNIT_HZ,
	.current = TENKI_UNIT_AMPS,
	.power = TENKI_UNIT_WATTS,
	.length = TENKI_UNIT_METERS,
	.concentration = TENKI_UNIT_SENSOR_DEFAULT,
};

int g_pretty = 0;
int g_full_display_mode = 0;
int g_decimal_digits = DEFAULT_DECIMAL_DIGITS;
int g_7bit_clean = 0;
int g_log_mode = 0;
int g_log_interval = DEFAULT_LOG_INTERVAL;
int g_must_run = 1;
int g_legacy_errors = 0;
const char *g_log_file = NULL;
FILE *g_log_fptr = NULL;
long g_flags = 0;

int g_num_attempts = 1;

int processChannels(USBTenki_dev_handle hdl, int *requested_channels, int num_req_chns);


static void printUsage(void)
{
	printf("Usage: ./usbtenkiget [options]\n");
	printf("\nValid options:\n");
	printf("    -V           Display version information\n");
	printf("    -v           Verbose mode\n");
	printf("    -h           Displays help\n");
	printf("    -l           List and display info about available sensors\n");
	printf("    -f           Full list mode. (shows unused/unconfigured channels)\n");
	printf("    -s serno     Use USB sensor with matching serial number. Default: Use first.\n");
	printf("    -i id<,id,id...>  Use specific channel(s) id(s) or 'a' for all. Default: %d\n", DEFAULT_CHANNEL_ID);
	printf("    -R num       If an USB command fails, retry it num times before bailing out\n");
	printf("    -T unit      Select the temperature unit to use. Default: Celcius\n");
	printf("    -P unit      Select the pressure unit to use. Default: kPa\n");
	printf("    -F unit      Select the frequency unit to use. Default: Hz\n");
	printf("    -M unit      Select the length unit to use. Default: m\n");
	printf("    -C unit      Select the concentration unit to use. Default: Sensor default\n");
	printf("    -p           Enable pretty output\n");
	printf("    -7           Use 7 bit clean output (no fancy degree symbols)\n");
	printf("    -L logfile   Log to specified file (use - for none)\n");
	printf("    -I interval  Log interval. In milliseconds. Default: %d\n", DEFAULT_LOG_INTERVAL);
	printf("    -x num       Set number of digits to display after the decimal separator. Default: %d\n", DEFAULT_DECIMAL_DIGITS);
	printf("    -o option    Enable specified option. (you may use multiple -o)\n");
	printf("    -S value     Set standard sea level pressure (Pascals) used to compute altitude. Default: 101325\n");

	printf("\nValid temperature units:\n");
	printf("    Celcius, c, Fahrenheit, f, Kelvin, k\n");
	printf("\nValid pressure units:\n");
	printf("    kPa, hPa, bar, at (98.0665 kPa), atm (101.325 kPa), Torr, psi, inHg\n");
	printf("\nValid frequency units:\n");
	printf("    mHz, Hz, kHz, MHz, rpm\n");
	printf("\nValid length units:\n");
	printf("    mm, cm, dm, m, mil, in, ft, yd\n");
	printf("\nValid concentration units:\n");
	printf("    ppb, ppm, percent\n");
	printf("\nOptions:\n");
	printf("    no_humidex_range     Calculate humidex even if input values are out of range.\n");
	printf("    no_heat_index_range  Calculate heat index even if the input values are out of range.\n");
	printf("    old_sht75            Use the old SHT RH compensention coefficients.\n");
	printf("    legacy_errors        Output channel errors in the old (unspecific) way.\n");
	printf("                         For instance: The string 'err' instead of 'ProbeDisconnected'\n");

	printf("\nErrors:\n");
	printf("\nWhen an error occurs reading a channel, the value is replaced by an error string:\n");
	printf("    Undefined            Unknown/undefined error.\n");
	printf("    Saturated            Sensor (or resulting value) is saturated and unusable.\n");
	printf("    SensorError          The physical sensor or interface circuitry is not working properly\n");
	printf("    ProbeDisconnected    Indicates that the probe is disconnected or cable is cut/open\n");
	printf("    OutOfRange           The reading falls outside the sensor possible or supported range\n");
	printf("    InvalidData          The data received from the sensor did not make sense or was incomplete\n");
	printf("\n");

	printf("Note: If pretty output is enabled (see -p) there will be spaces in the error messages. See also\n");
	printf("the 'legacy_errors' options to restore the old behaviour of returning 'err', regarless\n");
	printf("of what the specific error was.\n");

	printf("\nReturn value:\n");
	printf(" - On success, usbtenkiget returns 0.\n");
	printf(" - If the requested serial number (see -s) was not found, or if no devices were found (-f and -l) a non-zero value is returned.\n");
}

static void printVersion(void)
{
	printf("USBTenkiget version %s\n", USBTENKI_VERSION);
	printf("Copyright (C) 2007-2017, Raphael Assenat\n");
	printf("Copyright (C) 2018-2020, Dracal technologies inc.\n\n");
	printf("This software comes with ABSOLUTELY NO WARRANTY;\n");
	printf("You may redistribute copies of it under the terms of the GNU General Public License\n");
	printf("http://www.gnu.org/licenses/gpl.html\n");
}


int main(int argc, char **argv)
{
	USBTenki_dev_handle hdl;
	USBTenki_device cur_dev, dev=NULL;
	int res, i;
	struct USBTenki_list_ctx *listContext;
	struct USBTenki_info info;

	char *use_serial = NULL;
	int list_mode = 0;

	int requested_channels[MAX_CHANNELS];
	int num_requested_channels= 1;
	int use_all_channels = 0;

	requested_channels[0] = DEFAULT_CHANNEL_ID;

#ifdef WIN32
	/* Under windows, line buffering (_IOLBF) has fullly buffered behaviour.. No
	 * choice but to completely disable buffering... */
	setvbuf(stdout, NULL, _IONBF, 0);
#else
	setlinebuf(stdout);
#endif

	while (-1 != (res=getopt(argc, argv, "Vvhlfs:i:T:P:p7R:L:I:F:o:x:S:M:C:")))
	{
		switch (res)
		{
			case 'R':
				g_num_attempts = atoi(optarg) + 1;
				break;
			case 'x':
				g_decimal_digits = atoi(optarg);
				if (g_decimal_digits < 0) {
					fprintf(stderr, "Error, argument -x must be >= 0\n");
				}
				break;
			case 'V':
				printVersion();
				return 0;
			case 'v':
				g_verbose = 1;
				g_flags |= USBTENKI_FLAG_VERBOSE;
				break;
			case 'h':
				printUsage();
				return 0;
			case '7':
				g_7bit_clean = 1;
				break;
			case 'l':
				list_mode = 1;
				break;
			case 'f':
				g_full_display_mode = 1;
				break;
			case 's':
				use_serial = optarg;
				break;
			case 'i':
				{
					char *p;
					char *e;

					if (*optarg == 'a')
					{
						num_requested_channels = 0;
						use_all_channels = 1;
						break;
					}


					num_requested_channels = 0;
					p = optarg;
					while(1) {
						if (num_requested_channels >= MAX_CHANNELS) {
							fprintf(stderr,"too many channels\n");
							return -1;
						}

						requested_channels[num_requested_channels] = strtol(p, &e, 0);
						if (e==p) {
							fprintf(stderr, "Error in channel list\n");
							return -1;
						}

						num_requested_channels++;

						if (*e==0)
							break;

						if (*e==',') {
							e++;
						}
						else {
							fprintf(stderr, "Error in channel list\n");
							return -1;
						}

						p = e;
					}
				}
				break;

			case 'T':
				if (strcasecmp(optarg, "Celcius")==0 ||
						strcasecmp(optarg, "C")==0)
					g_unit_prefs.temperature = TENKI_UNIT_CELCIUS;
				else if (strcasecmp(optarg, "Fahrenheit")==0 ||
						strcasecmp(optarg, "F")==0)
					g_unit_prefs.temperature = TENKI_UNIT_FAHRENHEIT;
				else if (strcasecmp(optarg, "Kelvin")==0 ||
						strcasecmp(optarg, "K")==0)
					g_unit_prefs.temperature = TENKI_UNIT_KELVIN;
				else {
					fprintf(stderr, "Unknown temperature format: '%s'\n",
										optarg);
					return -1;
				}
				break;

			case 'F':
				{
					struct {
						const char *name;
						int fmt;
					} tbl[] = {
						{ "mHz", TENKI_UNIT_MILLIHZ },
						{ "Hz", TENKI_UNIT_HZ },
						{ "kHz", TENKI_UNIT_KHZ },
						{ "MHz", TENKI_UNIT_MHZ },
						{ "rpm", TENKI_UNIT_RPM },
					};

					for (i=0; i<ARRAY_SIZE(tbl); i++) {
						if (strcasecmp(tbl[i].name, optarg)==0) {
							g_unit_prefs.frequency = tbl[i].fmt;
							break;
						}
					}
					if (i==ARRAY_SIZE(tbl)) {
						fprintf(stderr,
							"Unknown frequency unit: '%s'\n", optarg);
					}
				}
				break;

			case 'P':
				{
					struct {
						const char *name;
						int fmt;
					} tbl[] = {
						{ "kpa", TENKI_UNIT_KPA },
						{ "hpa", TENKI_UNIT_HPA },
						{ "bar", TENKI_UNIT_BAR },
						{ "at", TENKI_UNIT_AT },
						{ "atm", TENKI_UNIT_ATM },
						{ "torr", TENKI_UNIT_TORR },
						{ "psi", TENKI_UNIT_PSI },
						{ "inhg", TENKI_UNIT_INHG },
					};

					for (i=0; i<ARRAY_SIZE(tbl); i++) {
						if (strcasecmp(tbl[i].name, optarg)==0) {
							g_unit_prefs.pressure = tbl[i].fmt;
							break;
						}
					}
					if (i==ARRAY_SIZE(tbl)) {
						fprintf(stderr,
							"Unknown pressure format: '%s'\n", optarg);
					}
				}
				break;

			case 'M':
				{
					struct {
						const char *name;
						int fmt;
					} tbl[] = {
						{ "m", TENKI_UNIT_METERS },
						{ "dm", TENKI_UNIT_DECIMETERS },
						{ "cm", TENKI_UNIT_CENTIMETERS },
						{ "mm", TENKI_UNIT_MILLIMETERS },
						{ "mil", TENKI_UNIT_MILS },
						{ "mils", TENKI_UNIT_MILS },
						{ "in", TENKI_UNIT_INCHES },
						{ "ft", TENKI_UNIT_FEET },
						{ "yd", TENKI_UNIT_YARDS },
					};

					for (i=0; i<ARRAY_SIZE(tbl); i++) {
						if (strcasecmp(tbl[i].name, optarg)==0) {
							g_unit_prefs.length = tbl[i].fmt;
							break;
						}
					}
					if (i==ARRAY_SIZE(tbl)) {
						fprintf(stderr,
							"Unknown length unit: '%s'\n", optarg);
					}
				}
				break;

			case 'C':
				{
					struct {
						const char *name;
						int fmt;
					} tbl[] = {
						{ "ppb", TENKI_UNIT_PPB },
						{ "ppm", TENKI_UNIT_PPM },
						{ "percent", TENKI_UNIT_PERCENT },
						{ "sensor", TENKI_UNIT_SENSOR_DEFAULT },
					};

					for (i=0; i<ARRAY_SIZE(tbl); i++) {
						if (strcasecmp(tbl[i].name, optarg)==0) {
							g_unit_prefs.concentration = tbl[i].fmt;
							break;
						}
					}
					if (i==ARRAY_SIZE(tbl)) {
						fprintf(stderr,
							"Unknown concentration unit: '%s'\n", optarg);
					}
				}
				break;


			case 'o':
				{
					struct {
						const char *name;
						long flag;
						int *set;
					} tbl[] = {
						{ "no_heat_index_range", USBTENKI_FLAG_NO_HEAT_INDEX_RANGE },
						{ "no_humidex_range", USBTENKI_FLAG_NO_HUMIDEX_RANGE },
						{ "old_sht75", USBTENKI_FLAG_USE_OLD_SHT75_COMPENSATION },
						{ "legacy_errors", 0, &g_legacy_errors },
					};

					for (i=0; i<ARRAY_SIZE(tbl); i++) {
						if (strcasecmp(tbl[i].name, optarg)==0) {
							g_flags |= tbl[i].flag;
							if (tbl[i].set) {
								*tbl[i].set = 1;
							}
							break;
						}
					}
					if (i==ARRAY_SIZE(tbl)) {
						fprintf(stderr,
							"Unknown option: '%s'\n", optarg);
					}
				}

				break;

			case 'p':
				g_pretty = 1;
				break;

			case 'L':
				g_log_file = optarg;
				break;

			case 'I':
				{
					char *e;
					g_log_interval = strtol(optarg, &e, 0);
					if (e==optarg) {
						fprintf(stderr, "Invalid log interval\n");
						return -1;
					}
				}
				break;

			case 'S':
				{
					char *e;
					double slp;

					slp = strtod(optarg, &e);
					if (e==optarg) {
						fprintf(stderr, "Invalid pressure\n");
						return -1;
					}
					usbtenki_set_seaLevelStandardPressure(slp);
				}
				break;

			case '?':
				fprintf(stderr, "Unknown argument specified\n");
				return -1;
		}
	}

	if (g_verbose) {
		printf("Arguments {\n");
		printf("  verbose: yes\n");
		printf("  use_all_channels: %d\n", use_all_channels);
		printf("  channel id(s): ");
		for (i=0; i<num_requested_channels; i++) {
			printf("%d ", requested_channels[i]);
		}
		printf("\n");
		printf("  list_mode: %d\n", list_mode);
		if (use_serial)
			printf("  use_serial: %s\n", use_serial);
		if (g_log_file) {
			printf("  log file: %s\n", g_log_file);
			printf("  log interval (ms): %d\n", g_log_interval);
		}
		printf("}\n");
	}

	usbtenki_init();

reopen:
	dev = NULL;

	listContext = usbtenki_allocListCtx();
	if (!listContext) {
		fprintf(stderr, "Error: Failed to allocate device listing context.\n");
		return -1;
	}

	while ((cur_dev=usbtenki_listDevices(&info, listContext)))
	{
		if (use_serial) {
			if (strcmp(use_serial, info.str_serial)==0) {
				dev = cur_dev;
			}
			else
				continue; // ignore other serials
		}
		else {
			dev = cur_dev; // Last found will be used
		}

		if (list_mode || g_verbose) {
			struct USBTenki_channel tmpchannels[MAX_CHANNELS];
			int n_channels, i;

			printf("Found: '%s', ", info.str_prodname);
			printf("Serial: '%s', ", info.str_serial);
			printf("Version %d.%d, ", info.major,
									info.minor);
		
			if (list_mode)
			{
				hdl = usbtenki_openDevice(dev);
				if (!hdl) {
					printf("\n");
					printf("    Error, cannot open device\n");
					continue;
				}


				n_channels = usbtenki_listChannels(hdl, tmpchannels, MAX_CHANNELS);
				if (n_channels==-1) {
					continue;
				}

				usbtenki_addVirtualChannels(tmpchannels, &n_channels, MAX_CHANNELS);
				printf("Channels: %d\n", n_channels);
				for (i=0; i<n_channels; i++) {
					if (tmpchannels[i].channel_id >= USBTENKI_VIRTUAL_START) {
						printf("    Virtual Channel %d: %s [%s]\n",
								tmpchannels[i].channel_id,
								chipToString(tmpchannels[i].chip_id),
								chipToShortString(tmpchannels[i].chip_id));
					}
					else {
						if (!g_full_display_mode && 
							tmpchannels[i].chip_id == USBTENKI_CHIP_NONE)
							continue;

						printf("    Channel %d: %s [%s]\n",
								tmpchannels[i].channel_id,
								chipToString(tmpchannels[i].chip_id),
								chipToShortString(tmpchannels[i].chip_id));
					}
				}

				usbtenki_closeDevice(hdl);
			}
		}
	}

	usbtenki_freeListCtx(listContext);

	if (!dev) {
		if (use_serial)
			fprintf(stderr, "Device with serial '%s' not found\n", use_serial);
		else {
			fprintf(stderr, "No device found\n");
		}

		if (g_log_file) {
			fprintf(stderr, "Log mode is on, hence retrying in 100ms...\n");
			usleep(100000);
			goto reopen;
		}
		return 1;
	}

	if (list_mode) {
		return 0;
	}

	hdl = usbtenki_openDevice(dev);
	if (!hdl) {
		fprintf(stderr, "Failed to open device.\n");
		return 1;
	}

	if (g_log_file)
	{
		printf("Log mode on.\n");
		if (!g_log_fptr) {
			if (strcmp(g_log_file, "-"))
			{
				g_log_fptr = fopen(g_log_file, "a");
				if (!g_log_file) {
					fprintf(stderr, "Failed to open log file\n");
					usbtenki_closeDevice(hdl);
					return -1;
				}
				printf("Opened file '%s' for logging. Append mode.\n", g_log_file);

#ifdef WIN32
				setvbuf(stdout, NULL, _IONBF, 0);
#else
				setlinebuf(g_log_fptr);
#endif
			}
			else {
				printf("Logging to stdout\n");
			}
		}

		// yyyy-mm-dd hh:mm:ss.000
		while (g_must_run)
		{
			int res;

			if (g_log_fptr) {
				printTimeStamp(g_log_fptr);
				fprintf(g_log_fptr, ", ");
			}

			res = processChannels(hdl, requested_channels, num_requested_channels);

			if (g_log_fptr) {
				fprintf(g_log_fptr, "\n");
				fflush(g_log_fptr);
			}

			usleep(g_log_interval * 1000);

			if (res<0) {
				fprintf(stderr, "Data acquisition error. Re-opening port...\n");
				usbtenki_closeDevice(hdl);
				goto reopen;
			}
		}

		printf("Closing log file.\n");
		if (g_log_fptr) {
			fflush(g_log_fptr);
			fclose(g_log_fptr);
		}
	}
	else {
		processChannels(hdl, requested_channels, num_requested_channels);
	}

	usbtenki_closeDevice(hdl);

	return 0;
}

int processVirtualChannels(USBTenki_dev_handle hdl, struct USBTenki_channel *channels,
							int num_channels, const int reqst_chns[], int num_req_chns)
{
	return usbtenki_processSomeVirtualChannels(hdl, channels, num_channels, reqst_chns, num_req_chns, g_flags);
}

int processChannels(USBTenki_dev_handle hdl, int *requested_channels, int num_req_chns)
{
	struct USBTenki_channel channels[MAX_CHANNELS];
	int num_channels=0;
	int res, i;

	/* Add channels supported by the device to the array */
	res = usbtenki_listChannels(hdl, channels, MAX_CHANNELS);
	if (res<0) {
		return -1;
	}
	num_channels = res;
	if (g_verbose)
		printf("Device has a total of %d channels\n", num_channels);

	/* Add virtual channels to the list, depending on the real
	 * channels already present. (eg: Dew point and humidex can
	 * only be calculated with temperature and humidity readings. */
	usbtenki_addVirtualChannels(channels, &num_channels, MAX_CHANNELS);

	/* When user request all channels, num_req_chns is 0. Generate a
	 * list of requested channels using all available real and
	 * virtual channels */
	if (!num_req_chns) {
		if (g_verbose)
			printf("All channels requested\n");

		for (i=0; i<num_channels; i++) {

			if (!g_full_display_mode &&
					channels[i].chip_id == USBTENKI_CHIP_NONE)
				continue; // skip unused channels unless in full list

			requested_channels[num_req_chns] = channels[i].channel_id;
			num_req_chns++;
		}
	}

	/* Check if the channels requested channel(s) are available */
	for (i=0; i<num_req_chns; i++)
	{
		 int j;
		 for (j=0; j<num_channels; j++) {
		 	if (channels[j].channel_id == requested_channels[i])
				break;
		 }
		 if (j==num_channels) {
		 	fprintf(stderr, "Requested channel %d does not exist.\n",
								requested_channels[i]);
			return -1;
		 }
	}

	if (g_verbose)
		printf("Reading real channels...\n");

	/* Read all requested, real channels */
	res = usbtenki_readChannelList(hdl, requested_channels, num_req_chns,
													channels, num_channels, g_num_attempts, g_flags);
	if (res<0) {
		return -1;
	}

	if (g_verbose)
		printf("%d requested channels read successfully\n", num_req_chns);


	/* Compute virtual channels. The may read additional non-user requeted
	 * data from the device. */
	if (g_verbose)
		printf("Computing virtual channels, possibly reading more...\n");
	processVirtualChannels(hdl, channels, num_channels, requested_channels, num_req_chns);


	/* Finally, display all the channels in the requested format */
	for (i=0; i<num_req_chns; i++) {
		int j;
		struct USBTenki_channel *chn;
		char fmt[16];

		/* find the requested channel */
		chn = NULL;
		for (j=0; j<num_channels; j++) {
			chn = &channels[j];
			if (chn->channel_id == requested_channels[i])
				break;
		}

		if (!chn) {
			fprintf(stderr, "Internal error..\n");
			res = -2;
			return -2;
		}

		usbtenki_convertUnits(chn, &g_unit_prefs);

		sprintf(fmt, "%%.%df", g_decimal_digits);

		if (g_pretty) {
			printf("%s: ", chipToShortString(chn->chip_id));
			if (chn->status != USBTENKI_CHN_STATUS_VALID) {
				printf("%s\n", g_legacy_errors ? "err" : usbtenki_getChannelStatusString(chn));
			} else {
				if (chn->converted_unit == TENKI_UNIT_HEXCOLOR) {
					int color = chn->converted_data;
					printf("#%06x\n", color);
				} else {
					printf(fmt, chn->converted_data);
				}

				printf(" %s\n", unitToString(chn->converted_unit, g_7bit_clean));
			}
		}
		else {
			if (chn->status != USBTENKI_CHN_STATUS_VALID) {
				printf("%s", g_legacy_errors ? "err" : usbtenki_getChannelStatusStringNoSpaces(chn));
			} else {
				printf(fmt , chn->converted_data);
			}
			if (i<num_req_chns-1)
				printf(", ");
		}

		if (g_log_fptr) {
			fprintf(g_log_fptr, fmt, chn->converted_data);
				if (i<num_req_chns-1)
					fprintf(g_log_fptr, ", ");
		}


	}
	if (!g_pretty)
		printf("\n");

	return 0;
}

