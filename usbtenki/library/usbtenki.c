/* usbtenki: A library for accessing USBTenki sensors.
 * Copyright (C) 2018 Dracal Technologies inc.
 * Copyright (C) 2007-2018  Raphael Assenat
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
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <math.h>
#include <ctype.h>
#include <errno.h>
#include <unistd.h>

#include "usbtenki.h"
#include "usbtenki_priv.h"
#include "usbtenki_cmds.h"
#include "usbtenki_units.h"

#define MAX_USB_ATTEMPTS	3

#if defined(WINDOWS_VERSION) | defined(WIN32)
#include <windows.h>
#define usleep(t) Sleep(t/1000)
#endif

int g_usbtenki_verbose=0;
int g_usbtenki_num_attempts = 3;

#ifndef USE_OLD_LIBUSB
#include <libusb.h>
libusb_context *g_libusbctx;
#endif

#define VIDPID_NOT_HANDLED		0
#define VIDPID_HANDLED			1 // Recognized by VID/PID, name irrelevant.
#define VIDPID_HANDLED_STRING	2 // Shared VID/PID, needs name test
char isHandledVidPid(unsigned short vid, unsigned short pid)
{
	// The raphnet.net ID
	if ((vid == 0x1781) && (pid == 0x0a98)) {
		return VIDPID_HANDLED_STRING;
	}
	// The Dracal technoloiges inc. ID range
	if (vid == 0x289b) {
		if ((pid >= 0x0500) && (pid <= 0x5FF)) {
			return VIDPID_HANDLED;
		}
	}
	return VIDPID_NOT_HANDLED;
}

char isNameHandled(const char *str)
{
	if (strcmp(str, "USBTenki")==0)
		return 1;
	if (strcmp(str, "USB_Temp")==0)
		return 1;
	return 0;
}

char matchSerialNumber(const char *str)
{
	char *m = getenv("USBTENKI_SHOW_ONLY");
	if (!m) {
		return 1;
	}
	return strcmp(m, str) == 0;
}

int usbtenki_init(void)
{
	if (getenv("USBTENKI_VERBOSE")) {
		g_usbtenki_verbose = 1;
	}

#ifdef USE_OLD_LIBUSB
	usb_init();
#else
	libusb_init(&g_libusbctx);
#endif
	return 0;
}

void unsbtenki_shutdown(void)
{
#ifdef USE_OLD_LIBUSB
#else
	libusb_exit(g_libusbctx);
#endif
}

static unsigned char xor_buf(unsigned char *buf, int len)
{
	unsigned char x=0;
	while (len--) {
		x ^= *buf;
		buf++;
	}
	return x;
}

#ifdef USE_OLD_LIBUSB
static void usbtenki_initListCtx(struct USBTenki_list_ctx *ctx)
{
	memset(ctx, 0, sizeof(struct USBTenki_list_ctx));
}

struct USBTenki_list_ctx *usbtenki_allocListCtx(void)
{
	struct USBTenki_list_ctx *ctx;
	ctx = calloc(1, sizeof(struct USBTenki_list_ctx));
	return ctx;
}

void usbtenki_freeListCtx(struct USBTenki_list_ctx *ctx)
{
	if (ctx)
		free(ctx);
}

/**
 * \brief List instances of our usbtenki device on the USB busses.
 * \param dst Destination buffer for device serial number/id.
 * \param dstbuf_size Destination buffer size.
 */
USBTenki_device usbtenki_listDevices(struct USBTenki_info *info, struct USBTenki_list_ctx *ctx)
{
	struct usb_bus *start_bus;
	struct usb_device *start_dev;
	char isHandled;

	memset(info, 0, sizeof(struct USBTenki_info));

	if (ctx->dev && ctx->bus)
		goto jumpin;

	if (g_usbtenki_verbose)
			printf("Start listing\n");

	usb_find_busses();
	usb_find_devices();
	start_bus = usb_get_busses();

	if (start_bus==NULL) {
		if (g_usbtenki_verbose) {
			printf("No busses found!\n");
		}
		return NULL;
	}

	for (ctx->bus = start_bus; ctx->bus; ctx->bus = ctx->bus->next)
	{
		if (g_usbtenki_verbose) {
			printf("Bus '%s'\n", ctx->bus->dirname);
		}

		start_dev = ctx->bus->devices;
		for (ctx->dev = start_dev; ctx->dev; ctx->dev = ctx->dev->next)
		{
			isHandled = isHandledVidPid(ctx->dev->descriptor.idVendor, ctx->dev->descriptor.idProduct);

			if (isHandled)
			{
				usb_dev_handle *hdl;
				hdl = usb_open(ctx->dev);
				if (!hdl) {
					if (g_usbtenki_verbose)
						printf("Failed to open device. Error '%s'\n", usb_strerror());
					continue;
				}

				usb_get_string_simple(hdl, ctx->dev->descriptor.iProduct,
									info->str_prodname, 256);

				// Check the name if we need to (for shared vid/pid)
				if (isHandled == VIDPID_HANDLED_STRING)
				{
					if (!isNameHandled(info->str_prodname))
					{
						if (g_usbtenki_verbose) {
							printf("Ignored: %s\n", info->str_prodname);
						}
						usb_close(hdl);
						continue;
					}
				}

				usb_get_string_simple(hdl, ctx->dev->descriptor.iSerialNumber,
									info->str_serial, 256);

				if (!matchSerialNumber(info->str_serial)) {
					usb_close(hdl);
					continue;
				}

				info->minor = ctx->dev->descriptor.bcdDevice & 0xff;
				info->major = (ctx->dev->descriptor.bcdDevice & 0xff00) >> 8;

				usb_close(hdl);
				return ctx->dev;
			}

jumpin:
			// prevent 'error: label at end of compound statement'
			continue;
		}
	}

	return NULL;
}

USBTenki_dev_handle usbtenki_openDevice(USBTenki_device tdev)
{
	struct usb_dev_handle *hdl;
	int res;
	int i;

	if (g_usbtenki_verbose) {
		printf("Opening USB device %s\n", ((struct usb_device*)tdev)->filename);
	}

	hdl = usb_open(tdev);
	if (!hdl) {
		printf("Failed to open device. Error '%s'\n", usb_strerror());
		return NULL;
	}

	res = usb_set_configuration(hdl, USBTENKI_DEV_TO_USB_DEVICE(tdev)->config->bConfigurationValue);
	if (res < 0) {
		printf("USB Error (usb_set_configuration: %s)\n", usb_strerror());
		usb_close(hdl);
		return NULL;
	}

	res = usb_claim_interface(hdl, 0);
	if (res<0) {
		printf("Failed to claim interface. Error '%s'\n", usb_strerror());
		usb_close(hdl);
		return NULL;
	}

	return hdl;
}

void usbtenki_closeDevice(USBTenki_dev_handle hdl)
{
	usb_release_interface(hdl, 0);
	usb_close(hdl);
}

/**
 * \brief Search for an USBTenki device with a specific serial and open it.
 * \param serial Case-sensitive serial number
 * \param info Pointer to store device info. Pass a NULL if you don't need it.
 */
USBTenki_dev_handle usbtenki_openBySerial(const char *serial, struct USBTenki_info *info)
{
	struct USBTenki_list_ctx devlistctx;
	struct USBTenki_info inf;
	usb_dev_handle *hdl;
	struct usb_device *cur_dev, *dev=NULL;
	int res;

	usb_find_busses();
	usb_find_devices();

	usbtenki_initListCtx(&devlistctx);

	while ((cur_dev=usbtenki_listDevices(&inf, &devlistctx)))
	{
		if (strcmp(serial, inf.str_serial)==0) {
			if (info) {
				memcpy(info, &inf, sizeof (struct USBTenki_info));
			}
			dev = cur_dev;
			if (g_usbtenki_verbose) {
				printf("usbtenki.c: Found device '%s'\n", serial);
			}
			break;
		}
	}

	if (!dev)
		return NULL;

	hdl = usb_open(dev);
	if (!hdl) {
		fprintf(stderr, "usbtenki.c: USB_Error: %s\n", usb_strerror());
		return NULL;
	}

	if (g_usbtenki_verbose)
		printf("usbtenki.c: Setting configuration\n");

	res = usb_set_configuration(hdl, cur_dev->config->bConfigurationValue);
	if (res < 0) {
		printf("USB Error (usb_set_configuration: %s)\n", usb_strerror());
		usb_close(hdl);
		return NULL;
	}

	if (g_usbtenki_verbose)
		printf("usbtenki.c: Claiming interface\n");

	res = usb_claim_interface(hdl, 0);
	if (res < 0) {
		fprintf(stderr, "usbtenki.c: USB Error (usb_claim_interface: %s)\n", usb_strerror());
		usb_close(hdl);
		return NULL;
	}

	return (USBTenki_dev_handle) hdl;
}

int usbtenki_command(USBTenki_dev_handle hdl, unsigned char cmd,
										int id, unsigned char *dst, int dst_max_size)
{
	unsigned char buffer[32];
	unsigned char xor;
	int n, i;
	int datlen;
	static int first = 1, trace = 0;
	int attempts;

	if (first) {
		if (getenv("USBTENKI_TRACE")) {
			trace = 1;
		}
		first = 0;
	}

	for (attempts = 0; attempts < MAX_USB_ATTEMPTS; attempts++)
	{
		n =	usb_control_msg(hdl,
			USB_TYPE_VENDOR | USB_RECIP_DEVICE | USB_ENDPOINT_IN, /* requesttype */
			cmd, 	/* request*/
			id, 				/* value */
			0, 					/* index */
			(char*)buffer, sizeof(buffer), 5000);

		if (trace) {
			printf("req: 0x%02x, val: 0x%02x, idx: 0x%02x <> %d: ",
				cmd, id, 0, n);
			if (n>0) {
				for (i=0; i<n; i++) {
					printf("%02x ", buffer[i]);
				}
			}
			printf("\n");
		}

		if (n == -ETIMEDOUT) {
			printf("timeout\n");
		}
		if (n == -EPIPE) {
			printf("broken pipe\n");
		}

		if (n > 0) {
			break;
		}
	}

	if (n<0) {
		fprintf(stderr, "USB control message error: %s\n", usb_strerror());
		return -1;
	}

	/* Validate size first */
	if (n>dst_max_size) {
		fprintf(stderr, "Too much data received! (%d)\n", n);
		return -3;
	} else if (n<2) {
		fprintf(stderr, "Not enough data received! (%d)\n", n);
		return -4;
	}

	/* dont count command and xor */
	datlen = n - 2;

	/* Check if reply is for this command */
	if (buffer[0] != cmd) {
		fprintf(stderr, "Reply does not match request (0x%02x)\n", buffer[0]);
		return -5;
	}

	/* Check xor */
	xor = xor_buf(buffer, n);
	if (xor) {
		fprintf(stderr, "Communication corruption occured!\n");
		return -2;
	}

	if (datlen) {
		memcpy(dst, buffer+1, datlen);
	}

	return datlen;
}
#else

int usbtenki_command(USBTenki_dev_handle hdl, unsigned char cmd,
										int id, unsigned char *dst, int dst_max_size)
{
	unsigned char buffer[32];
	unsigned char xor;
	int n, i;
	int datlen;
	static int first = 1, trace = 0;
	int attempts;

	if (first) {
		if (getenv("USBTENKI_TRACE")) {
			trace = 1;
		}
		first = 0;
	}

	for (attempts = 0; attempts < MAX_USB_ATTEMPTS; attempts++)
	{
		n =	libusb_control_transfer(hdl,
			LIBUSB_REQUEST_TYPE_VENDOR | LIBUSB_RECIPIENT_DEVICE | LIBUSB_ENDPOINT_IN, /* requesttype */
			cmd, 	/* request*/
			id, 				/* value */
			0, 					/* index */
			buffer, sizeof(buffer), 5000);

		if (trace) {
			printf("req: 0x%02x, val: 0x%02x, idx: 0x%02x <> %d: ",
				cmd, id, 0, n);
			if (n>0) {
				for (i=0; i<n; i++) {
					printf("%02x ", buffer[i]);
				}
			}
			printf("\n");
		}

		if (n == -ETIMEDOUT) {
			printf("timeout\n");
		}
		if (n == -EPIPE) {
			printf("broken pipe\n");
		}

		if (n > 0) {
			break;
		}
	}

	if (n<0) {
		fprintf(stderr, "USB control message error: %s\n", libusb_strerror(n));
		return -1;
	}

	/* Validate size first */
	if (n<2) {
		fprintf(stderr, "Not enough data received! (%d)\n", n);
		return -4;
	}

	/* dont count command and xor */
	datlen = n - 2;

	if (datlen > dst_max_size) {
		fprintf(stderr, "Too much data received! (%d)\n", n);
		return -3;
	}

	/* Check if reply is for this command */
	if (buffer[0] != cmd) {
		fprintf(stderr, "Reply does not match request (0x%02x)\n", buffer[0]);
		return -5;
	}

	/* Check xor */
	xor = xor_buf(buffer, n);
	if (xor) {
		fprintf(stderr, "Communication corruption occured!\n");
		return -2;
	}

	if (datlen) {
		memcpy(dst, buffer+1, datlen);
	}

	return datlen;
}

void usbtenki_closeDevice(USBTenki_dev_handle hdl)
{
	libusb_device_handle *dev_handle = (libusb_device_handle*)hdl;

	if (dev_handle) {
		libusb_close(dev_handle);
	}
}

USBTenki_dev_handle usbtenki_openDevice(USBTenki_device tdev)
{
	libusb_device *dev = (libusb_device*)tdev;
	libusb_device_handle *dev_handle;
	int res;

	res = libusb_open(dev, &dev_handle);
	if (res != 0) {
		fprintf(stderr, "libusb_open failed: %s\n", libusb_strerror(res));
		return NULL;
	}

	return (USBTenki_dev_handle)dev_handle;
}

USBTenki_dev_handle usbtenki_openBySerial(const char *serial, struct USBTenki_info *info)
{
	struct USBTenki_list_ctx *devlistctx;
	struct USBTenki_info inf;
	USBTenki_device cur_dev;

	devlistctx = usbtenki_allocListCtx();
	if (!devlistctx) {
		return NULL;
	}

	while ((cur_dev=usbtenki_listDevices(&inf, devlistctx)))
	{
		if (strcmp(serial, inf.str_serial)==0) {
			if (info) {
				memcpy(info, &inf, sizeof (struct USBTenki_info));
			}

			usbtenki_freeListCtx(devlistctx);
			return usbtenki_openDevice(cur_dev);
		}
	}

	usbtenki_freeListCtx(devlistctx);

	return NULL;
}

struct USBTenki_list_ctx *usbtenki_allocListCtx(void)
{
	struct USBTenki_list_ctx *ctx = calloc(sizeof(struct USBTenki_list_ctx), 1);
	if (!ctx) {
		return ctx;
	}
	return ctx;
}

void usbtenki_freeListCtx(struct USBTenki_list_ctx *ctx)
{
	if (ctx) {
		if (ctx->devices) {
			libusb_free_device_list(ctx->devices, 0); // TODO : unref_devices?
		}
		free(ctx);
	}
}

USBTenki_device usbtenki_listDevices(struct USBTenki_info *info, struct USBTenki_list_ctx *ctx)
{
	libusb_device *dev;
	struct libusb_device_descriptor devdesc;
	libusb_device_handle *dev_handle;
	int res;

	if (!info || !ctx) {
		return NULL;
	}

	if (!ctx->devices) {
		ctx->n_devices = libusb_get_device_list(g_libusbctx, &ctx->devices);
		if (ctx->n_devices <= 0) {
			return NULL;
		}
	}

	while (ctx->cur_device_index < ctx->n_devices) {
		dev = ctx->devices[ctx->cur_device_index];
		ctx->cur_device_index++;

		res = libusb_get_device_descriptor(dev, &devdesc);
		if (res != 0) {
			fprintf(stderr, "get device descriptor failed: %s\n", libusb_strerror(res));
			return NULL;
		}

		if (isHandledVidPid(devdesc.idVendor, devdesc.idProduct)) {

			if (info)
			{
				res = libusb_open(dev, &dev_handle);
				if (res != 0) {
					fprintf(stderr, "Could not open device: %s\n", libusb_strerror(res));
					continue;
				}

				libusb_get_string_descriptor_ascii(dev_handle, devdesc.iProduct, (unsigned char*)info->str_prodname, sizeof(info->str_prodname));
				libusb_get_string_descriptor_ascii(dev_handle, devdesc.iSerialNumber, (unsigned char*)info->str_serial, sizeof(info->str_serial));
				info->major = devdesc.bcdDevice >> 8;
				info->minor = ((devdesc.bcdDevice & 0xf0)>>4) * 10 + (devdesc.bcdDevice & 0xf);
				libusb_close(dev_handle);
			}

			return (USBTenki_device)dev;
		}
	}

	return NULL;
}

#endif


int usbtenki_getCalibration(USBTenki_dev_handle hdl, int id, unsigned char *dst, int dst_max_size)
{
	int res;

	res = usbtenki_command(hdl, USBTENKI_GET_CALIBRATION, id, dst, dst_max_size);

	return res;
}

int usbtenki_getRaw(USBTenki_dev_handle hdl, int id, unsigned char *dst, int dst_max_size)
{
	return usbtenki_command(hdl, USBTENKI_GET_RAW, id, dst, dst_max_size);
}

int usbtenki_getNumChannels(USBTenki_dev_handle hdl)
{
	unsigned char dst[8];
	int res;

	res = usbtenki_command(hdl, USBTENKI_GET_NUM_CHANNELS, 0, dst, sizeof(dst));
	if (res<0)
		return res;
	if (res<1) /* Illegal for this command */
		return res;
	return dst[0];
}

int usbtenki_getChipID(USBTenki_dev_handle hdl, int id)
{
	unsigned char dst[8];
	int res;

	res = usbtenki_command(hdl, USBTENKI_GET_CHIP_ID, id, dst, sizeof(dst));
	if (res<0)
		return res;
	if (res!=1) /* Illegal for this command */
		return res;
	return dst[0];
}

float usbtenki_convertConcentration(float c, int src_fmt, int dst_fmt, int *returned_units)
{
	double tmp_ppb;
	float out;

	// If no conversion is needed due to identical units, return the value as-is.
	// If no conversion is needed since native sensor units are requested, return the value as-is.
	if ((src_fmt == dst_fmt) || (dst_fmt == TENKI_UNIT_SENSOR_DEFAULT ) ) {
		if (returned_units) {
			*returned_units = src_fmt;
		}
		return c;
	}

	switch (src_fmt)
	{
		case TENKI_UNIT_PPB:
			tmp_ppb = c;
			break;
		case TENKI_UNIT_PPM:
			tmp_ppb = c * 1000;
			break;
		case TENKI_UNIT_PERCENT:
			tmp_ppb = c / 100 * 1000000000;
			break;
		default:
			// unsupported? Leave it as-is.
			if (returned_units) {
				*returned_units = src_fmt;
			}
			return c;
	}

	switch (dst_fmt)
	{
		case TENKI_UNIT_PPB:
			out = tmp_ppb;
			break;
		case TENKI_UNIT_PPM:
			out = tmp_ppb / 1000;
			break;
		case TENKI_UNIT_PERCENT:
			out = tmp_ppb * 100 / 1000000000;
			break;
		default:
			if (returned_units) {
				*returned_units = src_fmt;
			}
			return c;
	}

	if (returned_units) {
		*returned_units = dst_fmt;
	}

	return out;
}

float usbtenki_convertFrequency(float freq, int src_fmt, int dst_fmt)
{
	double hz;

	if (src_fmt == dst_fmt)
		return freq;

	switch (src_fmt)
	{
		case TENKI_UNIT_MILLIHZ:
			hz = freq / 1000;
			break;
		case TENKI_UNIT_HZ:
			hz = freq;
			break;
		case TENKI_UNIT_KHZ:
			hz = freq * 1000;
			break;
		case TENKI_UNIT_MHZ:
			hz = freq * 1000000;
			break;
		case TENKI_UNIT_RPM:
			hz = freq / 60;
			break;
		default:
			return freq;
	}

	switch (dst_fmt)
	{
		case TENKI_UNIT_MILLIHZ:
			return hz * 1000;
		case TENKI_UNIT_HZ:
			return hz;
		case TENKI_UNIT_KHZ:
			return hz / 1000;
		case TENKI_UNIT_MHZ:
			return hz / 1000000;
		case TENKI_UNIT_RPM:
			return hz * 60;
	}

	return freq;
}

float usbtenki_convertPressure(float pressure, int src_fmt, int dst_fmt)
{
	float pascals;

	if (src_fmt == dst_fmt)
		return pressure;

	switch (src_fmt)
	{
		case TENKI_UNIT_KPA:
			pascals = pressure * 1000.0;
			break;

		case TENKI_UNIT_HPA:
			pascals = pressure * 100.0;
			break;

		case TENKI_UNIT_BAR:
			pascals = pressure * 100000.0;
			break;

		case TENKI_UNIT_AT:
			pascals = pressure * 98066.5;
			break;

		case TENKI_UNIT_ATM:
			pascals = pressure * 101325;
			break;

		case TENKI_UNIT_TORR:
			pascals = pressure * 133.322;
			break;

		case TENKI_UNIT_PSI:
			pascals = pressure * 6894.76;
			break;

		case TENKI_UNIT_INHG:
			pascals = pressure * 3386.389;
			break;

		default:
			return pressure;
	}

	switch (dst_fmt)
	{
		case TENKI_UNIT_KPA: return pascals / 1000.0;
		case TENKI_UNIT_HPA: return pascals / 100.0;
		case TENKI_UNIT_BAR: return pascals / 100000.0;
		case TENKI_UNIT_AT: return pascals / 98066.5;
		case TENKI_UNIT_ATM: return pascals / 101325.0;
		case TENKI_UNIT_TORR: return pascals / 133.322;
		case TENKI_UNIT_PSI: return pascals / 6894.76;
		case TENKI_UNIT_INHG: return pascals / 3386.389;
	}

	return pressure;
}

float usbtenki_convertTemperature(float temperature, int src_fmt, int dst_fmt)
{
	float converted = temperature;

	switch (src_fmt)
	{
		case TENKI_UNIT_CELCIUS:
			switch(dst_fmt)
			{
				case TENKI_UNIT_FAHRENHEIT:
					converted = (temperature * 1.8) + 32.0;
					break;
				case TENKI_UNIT_KELVIN:
					converted = temperature + 273.15;
					break;
			}
			break;

		case TENKI_UNIT_FAHRENHEIT:
			switch(dst_fmt)
			{
				case TENKI_UNIT_CELCIUS:
					converted = (temperature - 32.0) / 1.8;
					break;
				case TENKI_UNIT_KELVIN:
					converted = (temperature + 459.67) / 1.8;
					break;
			}
			break;

		case TENKI_UNIT_KELVIN:
			switch(dst_fmt)
			{
				case TENKI_UNIT_CELCIUS:
					converted = temperature - 273.15;
					break;

				case TENKI_UNIT_FAHRENHEIT:
					converted = (temperature * 1.8) - 459.67;
					break;
			}
			break;
	}

	return converted;
//	printf("%.2f\n", converted);
}

float usbtenki_convertLength(float length, int src_fmt, int dst_fmt)
{
	float converted = length;
	float meters = 0;

	// First convert to meters
	switch (src_fmt)
	{
		case TENKI_UNIT_METERS:
			meters = length;
			break;
		case TENKI_UNIT_DECIMETERS:
			meters = length / 10;
			break;
		case TENKI_UNIT_CENTIMETERS:
			meters = length / 100;
			break;
		case TENKI_UNIT_MILLIMETERS:
			meters = length / 1000;
			break;
		case TENKI_UNIT_MICROMETERS:
			meters = length / 1000000;
			break;

		case TENKI_UNIT_INCHES:
			// 1 inch = 25.4mm = 2.54cm = 0.254dm = 0.0254m
			meters = length * 0.0254;
			break;

		case TENKI_UNIT_FEET:
			meters = length * 12 * 0.0254;
			break;

		case TENKI_UNIT_YARDS:
			meters = length * 3 * 12 * 0.0254;
			break;

		case TENKI_UNIT_MILS:
			meters = (length / 1000) * 0.0254;
			break;
	}

	// Now convert meters to target unit
	switch (dst_fmt)
	{
		case TENKI_UNIT_METERS:
			converted = meters;
			break;

		case TENKI_UNIT_DECIMETERS:
			converted = meters * 10;
			break;

		case TENKI_UNIT_CENTIMETERS:
			converted = meters * 100;
			break;

		case TENKI_UNIT_MILLIMETERS:
			converted = meters * 1000;
			break;

		case TENKI_UNIT_MICROMETERS:
			converted = meters * 1000000;
			break;

		case TENKI_UNIT_INCHES:
			converted = meters / 0.0254;
			break;

		case TENKI_UNIT_MILS:
			converted = meters / 0.0254 * 1000;
			break;

		case TENKI_UNIT_FEET:
			converted = meters / 0.0254 / 12;
			break;

		case TENKI_UNIT_YARDS:
			converted = meters / 0.0254 / 12 / 3;
			break;
	}

	return converted;
}

float usbtenki_convertVoltage(float v, int src_fmt, int dst_fmt)
{
	float converted = v;

	switch (src_fmt) {
		case TENKI_UNIT_VOLTS:
			switch (dst_fmt)
			{
				case TENKI_UNIT_MILLIVOLT:
					converted = v * 1000;
					break;
			}
			break;

		case TENKI_UNIT_MILLIVOLT:
			switch (dst_fmt)
			{
				case TENKI_UNIT_VOLTS:
					converted = v / 1000;
					break;
			}
			break;
	}

	return converted;
}

float usbtenki_convertCurrent(float c, int src_fmt, int dst_fmt)
{
	float converted = c;

	switch (src_fmt)
	{
		case TENKI_UNIT_AMPS:
			switch (dst_fmt)
			{
				case TENKI_UNIT_MILLIAMPS:
					converted = c * 1000;
					break;
			}
			break;

		case TENKI_UNIT_MILLIAMPS:
			switch (dst_fmt)
			{
				case TENKI_UNIT_AMPS:
					converted = c / 1000;
					break;
			}
			break;
	}

	return converted;
}

float usbtenki_convertPower(float p, int src_fmt, int dst_fmt)
{
	float converted = p;

	switch (src_fmt)
	{
		case TENKI_UNIT_KILOWATTS:
			switch (dst_fmt)
			{
				case TENKI_UNIT_WATTS:
					converted = p * 1000;
					break;

				case TENKI_UNIT_MILLIWATTS:
					converted = p * 1000 * 1000;
					break;
			}
			break;

		case TENKI_UNIT_WATTS:
			switch (dst_fmt)
			{
				case TENKI_UNIT_KILOWATTS:
					converted = p / 1000;
					break;
				case TENKI_UNIT_MILLIWATTS:
					converted = p * 1000;
					break;
			}
			break;

		case TENKI_UNIT_MILLIWATTS:
			switch (dst_fmt)
			{
				case TENKI_UNIT_KILOWATTS:
					converted = p / 1000 / 1000;
					break;
				case TENKI_UNIT_WATTS:
					converted = p / 1000;
					break;
			}
			break;
	}

	return converted;
}

const char *chipToString(int id)
{
	switch(id)
	{
		case USBTENKI_CHIP_ADT7410:
			return "ADT7410 I2C Temperature sensor";
		case USBTENKI_CHIP_SE95:
			return "SE95 I2C Temperature sensor";
		case USBTENKI_CHIP_MCP9800:
			return "MCP980x I2C Temperature sensor";
		case USBTENKI_CHIP_LM75:
			return "LM75 I2C Temperature sensor";
		case USBTENKI_CHIP_LM92:
			return "LM92 I2C Temperature sensor";
		case USBTENKI_CHIP_SHT_TEMP:
			return "Sensirion SHT1x/7x Temperature";
		case USBTENKI_CHIP_SHT_RH:
			return "Sensirion SHT1x/7x Relative Humidity";
		case USBTENKI_CHIP_BS02_TEMP:
			return "BS02 Temperature";
		case USBTENKI_CHIP_BS02_RH:
			return "BS02 Relative Humidity";
		case USBTENKI_CHIP_CC2_RH:
			return "CC2 Relative Humidity";
		case USBTENKI_CHIP_CC2_T:
			return "CC2 Temperature";
		case USBTENKI_CHIP_SHT31_T:
			return "SHT31 Temperature";
		case USBTENKI_CHIP_SHT31_RH:
			return "SHT31 Relative Humidity";
		case USBTENKI_CHIP_SHT35_T:
			return "SHT35 Temperature";
		case USBTENKI_CHIP_SHT35_RH:
			return "SHT35 Relative Humidity";
		case USBTENKI_CHIP_SCD30_T:
			return "SCD30 Temperature";
		case USBTENKI_CHIP_SCD30_RH:
			return "SCD30 Relative Humidity";

		case USBTENKI_CHIP_SHT31_T_INTERNAL:
			return "Internal Temperature";
		case USBTENKI_CHIP_SHT31_RH_INTERNAL:
			return "Internal Relative Humidity";
		case USBTENKI_CHIP_SHT31_T_EXTERNAL:
			return "External Temperature";
		case USBTENKI_CHIP_SHT31_RH_EXTERNAL:
			return "External Relative Humidity";

		case USBTENKI_CHIP_CCS811_TVOC:
			return "CCS811 TVOC PPB";

		case USBTENKI_CHIP_CCS811_eCO2:
			return "CCS811 eCO2 PPM";

		case USBTENKI_CHIP_SCD30_CO2:
			return "SCD30 CO2 GAS PPM";

		case USBTENKI_CHIP_CO2_PPM:
			return "CO2 GAS PPM";

		case USBTENKI_CHIP_CO2_DXC200_FILTERED:
			return "CO2 GAS PPM (filtered)";
		case USBTENKI_CHIP_CO2_DXC200_INSTANT:
			return "CO2 GAS PPM (instant)";

		case USBTENKI_CHIP_SPS30_MC_PM1_0:
			return "Mass Concentration PM1.0";
		case USBTENKI_CHIP_SPS30_MC_PM2_5:
			return "Mass Concentration PM2.5";
		case USBTENKI_CHIP_SPS30_MC_PM4_0:
			return "Mass Concentration PM4.0";
		case USBTENKI_CHIP_SPS30_MC_PM10:
			return "Mass Concentration PM10";
		case USBTENKI_CHIP_SPS30_NC_PM0_5:
			return "Number Concentration PM0.5";
		case USBTENKI_CHIP_SPS30_NC_PM1_0:
			return "Number Concentration PM1.0";
		case USBTENKI_CHIP_SPS30_NC_PM2_5:
			return "Number Concentration PM2.5";
		case USBTENKI_CHIP_SPS30_NC_PM4_0:
			return "Number Concentration PM4.0";
		case USBTENKI_CHIP_SPS30_NC_PM10:
			return "Number Concentration PM10";

		case USBTENKI_CHIP_SPS30_TYP_PART_SIZE:
			return "Typical Particle Size";

		case USBTENKI_CHIP_TSL2561_IR_VISIBLE:
			return "TSL2561 Channel 0 (IR+Visibile)";
		case USBTENKI_CHIP_TSL2561_IR:
			return "TSL2561 Channel 1 (IR only)";
		case USBTENKI_CHIP_TSL2561_IR_VISIBLE_16X:
			return "TSL2561 Channel 0 (IR+Visibile) 16X gain";
		case USBTENKI_CHIP_TSL2561_IR_16X:
			return "TSL2561 Channel 1 (IR only) 16X gain";

		case USBTENKI_CHIP_TSL2568_IR_VISIBLE:
			return "TSL2568 Channel 0 (IR+Visibile)";
		case USBTENKI_CHIP_TSL2568_IR:
			return "TSL2568 Channel 1 (IR only)";
		case USBTENKI_CHIP_TSL2568_IR_VISIBLE_16X:
			return "TSL2568 Channel 0 (IR+Visibile) 16X gain";
		case USBTENKI_CHIP_TSL2568_IR_16X:
			return "TSL2568 Channel 1 (IR only)";

		case USBTENKI_MCU_ADC0:
			return "Microcontroller ADC channel 0";
		case USBTENKI_MCU_ADC1:
			return "Microcontroller ADC channel 1";
		case USBTENKI_MCU_ADC2:
			return "Microcontroller ADC channel 2";
		case USBTENKI_MCU_ADC3:
			return "Microcontroller ADC channel 3";
		case USBTENKI_MCU_ADC4:
			return "Microcontroller ADC channel 4";
		case USBTENKI_MCU_ADC5:
			return "Microcontroller ADC channel 5";

		case USBTENKI_CHIP_MPXV7002:
			return "MPXV7002 Differential air pressure sensor";

		case USBTENKI_CHIP_MLH_A:
			return "MLH150-A Pressure sensor";

		case USBTENKI_CHIP_MPX4115:
			return "MPX4115 Absolute air pressure sensor";

		case USBTENKI_CHIP_MP3H6115A:
			return "MP3H6115A Absolute air pressure sensor";

		case USBTENKI_CHIP_VOLTS:
			return "Ratiometric volts from ADC";

		case USBTENKI_CHIP_VOLTS_REVERSE:
			return "Inverted ratiometric volts from ADC";

		case USBTENKI_CHIP_DRACAL_EM1_BUS_VOLTAGE:
			return "Bus voltage";

		case USBTENKI_CHIP_DRACAL_EM1_SHUNT_VOLTAGE:
			return "Shunt voltage";

		case USBTENKI_CHIP_DRACAL_EM1_POWER:
			return "Power";

		case USBTENKI_CHIP_DRACAL_EM1_CURRENT:
			return "Current";

		case USBTENKI_CHIP_TACHOMETER:
			return "Tachometer";

		case USBTENKI_CHIP_RTD300_PT100_2W:
			return "2-Wire PT100 Temperature sensor";

		case USBTENKI_CHIP_RTD300_PT100_3W:
			return "3-Wire PT100 Temperature sensor";

		case USBTENKI_CHIP_PT100_RTD:
			return "PT100 Temperature sensor";

		case USBTENKI_CHIP_TMC200_COLD:
		case USBTENKI_CHIP_THC_COLD:
			return "Thermocouple cold junction temperature";
		case USBTENKI_CHIP_THC_HOT:
			return "Thermocouple hot junction temperature";
		case USBTENKI_CHIP_TMC200_TYPE_K:
		case USBTENKI_CHIP_THC_TYPE_K:
			return "Type-K Thermocouple";
		case USBTENKI_CHIP_TMC200_TYPE_J:
		case USBTENKI_CHIP_THC_TYPE_J:
			return "Type-J Thermocouple";
		case USBTENKI_CHIP_TMC200_TYPE_T:
		case USBTENKI_CHIP_THC_TYPE_T:
			return "Type-T Thermocouple";
		case USBTENKI_CHIP_TMC200_TYPE_N:
		case USBTENKI_CHIP_THC_TYPE_N:
			return "Type-N Thermocouple";
		case USBTENKI_CHIP_TMC200_TYPE_S:
		case USBTENKI_CHIP_THC_TYPE_S:
			return "Type-S Thermocouple";
		case USBTENKI_CHIP_TMC200_TYPE_E:
		case USBTENKI_CHIP_THC_TYPE_E:
			return "Type-E Thermocouple";
		case USBTENKI_CHIP_TMC200_TYPE_B:
		case USBTENKI_CHIP_THC_TYPE_B:
			return "Type-B Thermocouple";
		case USBTENKI_CHIP_TMC200_TYPE_R:
		case USBTENKI_CHIP_THC_TYPE_R:
			return "Type-R Thermocouple";

		case USBTENKI_CHIP_MLX90614_TA:
			return "MLX90615 Ambiant temperature";
		case USBTENKI_CHIP_MLX90614_TOBJ:
			return "MLX90614 Object temperature";

		case USBTENKI_CHIP_MS5611_P:
			return "MS5611 Pressure";
		case USBTENKI_CHIP_MS5611_T:
			return "MS5611 Temperature";

		case USBTENKI_CHIP_VEML6075_UVA:
			return "VEML6075 UVA";
		case USBTENKI_CHIP_VEML6075_UVB:
			return "VEML6075 UVB";
		case USBTENKI_CHIP_VEML6030_ALS:
			return "VEML6030 ALS";
		case USBTENKI_CHIP_VEML6030_WHITE:
			return "VEML6030 WHITE";

		/* Virtual channels and chipID have the same vales */
		case USBTENKI_VIRTUAL_INTERNAL_DEW_POINT:
			return "Internal Dew point";
		case USBTENKI_VIRTUAL_DEW_POINT:
			return "Dew point";
		case USBTENKI_VIRTUAL_HUMIDEX:
			return "Humidex";
		case USBTENKI_VIRTUAL_HEAT_INDEX:
			return "Heat index";

		case USBTENKI_VIRTUAL_SHT75_COMPENSATED_RH:
			return "Relative Humidity (Temp. compensated)";

		case USBTENKI_VIRTUAL_TSL2561_LUX:
			return "TSL2561 Lux";

		case USBTENKI_VIRTUAL_TSL2568_LUX:
		 	return "TSL2568 Lux";

		case USBTENKI_VIRTUAL_ALTITUDE:
			return "Altitude";

		case USBTENKI_CHIP_NONE:
			return "Unused/unconfigured";

		case USBTENKI_CHIP_RED:
			return "Red light intensity";
		case USBTENKI_CHIP_GREEN:
			return "Green light intensity";
		case USBTENKI_CHIP_BLUE:
			return "Blue light intensity";
		case USBTENKI_CHIP_IR:
			return "IR light intensity";
		case USBTENKI_CHIP_HEXCOLOR:
			return "Hexadecimal color";

		case USBTENKI_CHIP_MATH:
			return "Math";
	}
	return "unknown";
}

const char *chipToShortString(int id)
{
	switch(id)
	{
		case USBTENKI_CHIP_ADT7410:
		case USBTENKI_CHIP_SE95:
		case USBTENKI_CHIP_MCP9800:
		case USBTENKI_CHIP_LM75:
		case USBTENKI_CHIP_LM92:
		case USBTENKI_CHIP_SHT_TEMP:
		case USBTENKI_CHIP_BS02_TEMP:
		case USBTENKI_CHIP_PT100_RTD:
		case USBTENKI_CHIP_TMC200_COLD:
		case USBTENKI_CHIP_TMC200_TYPE_K:
		case USBTENKI_CHIP_TMC200_TYPE_J:
		case USBTENKI_CHIP_TMC200_TYPE_N:
		case USBTENKI_CHIP_TMC200_TYPE_E:
		case USBTENKI_CHIP_TMC200_TYPE_T:
		case USBTENKI_CHIP_TMC200_TYPE_B:
		case USBTENKI_CHIP_TMC200_TYPE_S:
		case USBTENKI_CHIP_TMC200_TYPE_R:
		case USBTENKI_CHIP_RTD300_PT100_3W:
		case USBTENKI_CHIP_RTD300_PT100_2W:
		case USBTENKI_CHIP_MLX90614_TOBJ:
		case USBTENKI_CHIP_MLX90614_TA:
		case USBTENKI_CHIP_MS5611_T:
		case USBTENKI_CHIP_CC2_T:
		case USBTENKI_CHIP_SHT31_T:
		case USBTENKI_CHIP_SHT35_T:
		case USBTENKI_CHIP_THC_HOT:
		case USBTENKI_CHIP_THC_COLD:
		case USBTENKI_CHIP_THC_TYPE_K:
		case USBTENKI_CHIP_THC_TYPE_J:
		case USBTENKI_CHIP_THC_TYPE_T:
		case USBTENKI_CHIP_THC_TYPE_N:
		case USBTENKI_CHIP_THC_TYPE_S:
		case USBTENKI_CHIP_THC_TYPE_E:
		case USBTENKI_CHIP_THC_TYPE_B:
		case USBTENKI_CHIP_THC_TYPE_R:
		case USBTENKI_CHIP_SCD30_T:
		case USBTENKI_CHIP_SHT31_T_INTERNAL:
		case USBTENKI_CHIP_SHT31_T_EXTERNAL:
			return "Temperature";

		case USBTENKI_CHIP_TSL2561_IR_VISIBLE:
		case USBTENKI_CHIP_TSL2568_IR_VISIBLE:
			return "Visible and IR";

		case USBTENKI_CHIP_TSL2561_IR_VISIBLE_16X:
		case USBTENKI_CHIP_TSL2568_IR_VISIBLE_16X:
			return "Visible and IR (16x gain)";

		case USBTENKI_CHIP_TSL2561_IR:
		case USBTENKI_CHIP_TSL2568_IR:
			return "IR";

		case USBTENKI_CHIP_TSL2561_IR_16X:
		case USBTENKI_CHIP_TSL2568_IR_16X:
			return "IR (16x gain)";

		case USBTENKI_CHIP_VEML6075_UVA:
			return "UVA";
		case USBTENKI_CHIP_VEML6075_UVB:
			return "UVB";
		case USBTENKI_CHIP_VEML6030_ALS:
			return "Ambiant light";
		case USBTENKI_CHIP_VEML6030_WHITE:
			return "White light";

		case USBTENKI_VIRTUAL_SHT75_COMPENSATED_RH:
		case USBTENKI_CHIP_SHT_RH:
		case USBTENKI_CHIP_BS02_RH:
		case USBTENKI_CHIP_CC2_RH:
		case USBTENKI_CHIP_SHT31_RH:
		case USBTENKI_CHIP_SHT35_RH:
		case USBTENKI_CHIP_SCD30_RH:
		case USBTENKI_CHIP_SHT31_RH_INTERNAL:
		case USBTENKI_CHIP_SHT31_RH_EXTERNAL:
			return "Relative Humidity";

		case USBTENKI_MCU_ADC0:
		case USBTENKI_MCU_ADC1:
		case USBTENKI_MCU_ADC2:
		case USBTENKI_MCU_ADC3:
		case USBTENKI_MCU_ADC4:
		case USBTENKI_MCU_ADC5:
			return "Raw ADC output";

		case USBTENKI_CHIP_MS5611_P:
		case USBTENKI_CHIP_MPXV7002:
		case USBTENKI_CHIP_MPX4115:
		case USBTENKI_CHIP_MP3H6115A:
		case USBTENKI_CHIP_MLH_A:
			return "Pressure";

		case USBTENKI_CHIP_DRACAL_EM1_BUS_VOLTAGE:
		case USBTENKI_CHIP_DRACAL_EM1_SHUNT_VOLTAGE:
		case USBTENKI_CHIP_VOLTS_REVERSE:
		case USBTENKI_CHIP_VOLTS:
			return "Voltage";

		case USBTENKI_CHIP_DRACAL_EM1_POWER:
			return "Power";

		case USBTENKI_CHIP_DRACAL_EM1_CURRENT:
			return "Current";


		case USBTENKI_CHIP_D6F_V03A1:
			return "Air speed";

		case USBTENKI_CHIP_TACHOMETER:
			return "Frequency";

		case USBTENKI_CHIP_CO2_DXC200_FILTERED:
		case USBTENKI_CHIP_CO2_DXC200_INSTANT:
		case USBTENKI_CHIP_CO2_PPM:
		case USBTENKI_CHIP_SCD30_CO2:
			return "CO2 Gas PPM";

		case USBTENKI_CHIP_CCS811_eCO2:
		case USBTENKI_CHIP_CCS811_TVOC:
			return "Gas PPM";

		case USBTENKI_CHIP_RED:
		case USBTENKI_CHIP_GREEN:
		case USBTENKI_CHIP_BLUE:
		case USBTENKI_CHIP_IR:
			return chipToString(id);

		case USBTENKI_CHIP_SPS30_MC_PM1_0:
		case USBTENKI_CHIP_SPS30_MC_PM2_5:
		case USBTENKI_CHIP_SPS30_MC_PM4_0:
		case USBTENKI_CHIP_SPS30_MC_PM10:
		case USBTENKI_CHIP_SPS30_NC_PM0_5:
		case USBTENKI_CHIP_SPS30_NC_PM1_0:
		case USBTENKI_CHIP_SPS30_NC_PM2_5:
		case USBTENKI_CHIP_SPS30_NC_PM4_0:
		case USBTENKI_CHIP_SPS30_NC_PM10:
			return "Concentration";
		case USBTENKI_CHIP_SPS30_TYP_PART_SIZE:
			return "Size";



		/* Virtual channels and chipID share the same namespace */
		case USBTENKI_VIRTUAL_DEW_POINT:
		case USBTENKI_VIRTUAL_INTERNAL_DEW_POINT:
		case USBTENKI_VIRTUAL_EXTERNAL_DEW_POINT:
			return "Dew point";
		case USBTENKI_VIRTUAL_HUMIDEX:
			return "Humidex";
		case USBTENKI_VIRTUAL_HEAT_INDEX:
			return "Heat index";
		case USBTENKI_VIRTUAL_TSL2568_LUX:
		case USBTENKI_VIRTUAL_TSL2561_LUX:
			return "Lux";

		case USBTENKI_VIRTUAL_ALTITUDE:
			return "Height";

		case USBTENKI_CHIP_NONE:
			return "N/A";

		case USBTENKI_CHIP_HEXCOLOR:
			return "Color";

		case USBTENKI_CHIP_MATH:
			return "Math";
	}
	return "unknown";
}

const char *unitToString(int unit, int no_fancy_chars)
{
	switch(unit)
	{
		case TENKI_UNIT_PERCENT:
		case TENKI_UNIT_RH: return "%";

		/* Note: The degree symbol may appear incorrectly as two characters
		 * depending on your encoding here */
		case TENKI_UNIT_CELCIUS: return no_fancy_chars ? "C" : "°C";
		case TENKI_UNIT_KELVIN: return no_fancy_chars ? "K" : "°K";
		case TENKI_UNIT_FAHRENHEIT: return no_fancy_chars ? "F" : "°F";

		case TENKI_UNIT_RAW: return "(raw)";
		case TENKI_UNIT_KPA: return "kPa";
		case TENKI_UNIT_HPA: return "hPa";
		case TENKI_UNIT_BAR: return "bar";
		case TENKI_UNIT_AT: return "at";
		case TENKI_UNIT_ATM: return "atm";
		case TENKI_UNIT_TORR: return "Torr";
		case TENKI_UNIT_PSI: return "psi";
		case TENKI_UNIT_INHG: return "inHg";
		case TENKI_UNIT_PPM: return "ppm";
		case TENKI_UNIT_PPB: return "ppb";
		case TENKI_UNIT_VOLTS: return "V";
		case TENKI_UNIT_MILLIVOLT: return "mV";
		case TENKI_UNIT_MILLIWATTS: return "mW";
		case TENKI_UNIT_WATTS: return "W";
		case TENKI_UNIT_KILOWATTS: return "kW";
		case TENKI_UNIT_AMPS: return "A";
		case TENKI_UNIT_MILLIAMPS: return "mA";
		case TENKI_UNIT_LUX: return "lx";
		case TENKI_UNIT_METER_SEC: return "m/sec";
		case TENKI_UNIT_MILLIHZ: return "mHz";
		case TENKI_UNIT_HZ: return "Hz";
		case TENKI_UNIT_KHZ: return "kHz";
		case TENKI_UNIT_MHZ: return "MHz";
		case TENKI_UNIT_RPM: return "rpm";
		case TENKI_UNIT_METERS: return "m";
		case TENKI_UNIT_DECIMETERS: return "dm";
		case TENKI_UNIT_CENTIMETERS: return "cm";
		case TENKI_UNIT_MILLIMETERS: return "mm";
		case TENKI_UNIT_MICROMETERS: return no_fancy_chars ? "um" : "μm";
		case TENKI_UNIT_MILS: return "mil";
		case TENKI_UNIT_INCHES: return "in";
		case TENKI_UNIT_FEET: return "ft";
		case TENKI_UNIT_YARDS: return "yd";
		case TENKI_UNIT_ARBITRARY: return "arb. unit";
		case TENKI_UNIT_uG_PER_M3: return no_fancy_chars ? "ug/m^3" : "μg/m³";
		case TENKI_UNIT_COUNT_PER_CM3: return no_fancy_chars ? "#/cm^3" : "#/cm³";
		case TENKI_UNIT_uW_PER_CM2: return no_fancy_chars ? "uW/cm^2" : "μW/cm²";
	}

	return "";
}

const char *thermocoupleTypeToString(int type)
{
	switch (type)
	{
		case THERMOCOUPLE_TYPE_K: return "K";
		case THERMOCOUPLE_TYPE_J: return "J";
		case THERMOCOUPLE_TYPE_T: return "T";
		case THERMOCOUPLE_TYPE_N: return "N";
		case THERMOCOUPLE_TYPE_S: return "S";
		case THERMOCOUPLE_TYPE_E: return "E";
		case THERMOCOUPLE_TYPE_B: return "B";
		case THERMOCOUPLE_TYPE_R: return "R";
	}
	return "?";
}

int thermocoupleStringToType(const char *type)
{
	if (!type) {
		return -1;
	}
	if (strlen(type) != 1) {
		return -1;
	}

	switch(tolower(type[0])) {
		case 'k': return THERMOCOUPLE_TYPE_K;
		case 'j': return THERMOCOUPLE_TYPE_J;
		case 't': return THERMOCOUPLE_TYPE_T;
		case 'n': return THERMOCOUPLE_TYPE_N;
		case 's': return THERMOCOUPLE_TYPE_S;
		case 'e': return THERMOCOUPLE_TYPE_E;
		case 'b': return THERMOCOUPLE_TYPE_B;
		case 'r': return THERMOCOUPLE_TYPE_R;
	}

	return -1;
}

int usbtenki_readChannel(USBTenki_dev_handle hdl, struct USBTenki_channel *chn, unsigned long flags)
{
	return usbtenki_readChannelList(hdl, &chn->channel_id, 1, chn, 1, 1, flags);
}

/**
 * \param hdl Handle
 * \param channel_ids Array of channel IDs
 * \param num The number of channel IDs
 * \param dst The destination array
 * \param dst_total The total number of channels in 'dst'
 *
 * dst must have been setup by usbtenki_listChannels() first!
 *
 * This function only reads real channels (no virtuals)
 */
int usbtenki_readChannelList(USBTenki_dev_handle hdl, const int channel_ids[], int num, struct USBTenki_channel *dst, int dst_total, int num_attempts, unsigned long flags)
{
	int i, j, res;
	int n;
	unsigned char caldata[32];
	int caldata_len = 0;
	int caldata_chip = -1;

	for (i=0; i<num; i++)
	{
		// skip virtual channels
		if (channel_ids[i]>=USBTENKI_VIRTUAL_START)
			continue;

		// Look for the destination
		for (j=0; j<dst_total; j++) {
			if (channel_ids[i] == dst[j].channel_id) {
				break;
			}
		}
		if (j==dst_total) {
			fprintf(stderr, "Invalid channel ID (%d) requested\n", channel_ids[i]);
			continue;
		}

		if (dst[j].status != USBTENKI_CHN_STATUS_UNDEFINED)
			continue; /* already done */

		for (n=0; n<num_attempts; n++) {
			if (flags & USBTENKI_FLAG_VERBOSE) {
				printf("usbtenki_getRaw %d/%d chn %d attempt %d\n", i+1, num, dst[j].channel_id, n+1);
				usleep(100000);
			}
			dst[j].raw_length = usbtenki_getRaw(hdl, dst[j].channel_id, dst[j].raw_data, sizeof(dst[j].raw_data));
			if (dst[j].raw_length<0) {
				usleep(200);
				continue;
			}
			break;
		}

		/* all attempts failed? */
		if (n==num_attempts) {
			return -1;
		}

		// handle chips with calibration data
		if (dst[j].chip_id == USBTENKI_CHIP_PT100_RTD) {
//			printf("Fetching PT100 calibration data...\n");
			for (n=0; n<num_attempts; n++) {
				caldata_len = usbtenki_getCalibration(hdl, 0, caldata, sizeof(caldata));
				if (caldata_len<0) {
					usleep(200);
					continue;
				}
				caldata_chip = USBTENKI_CHIP_PT100_RTD;
				break;
			}

			/* all attempts failed? */
			if (n==num_attempts) {
				return -1;
			}
		}

		if (	(caldata_chip != USBTENKI_CHIP_MS5611_P) &&
				(	(dst[j].chip_id == USBTENKI_CHIP_MS5611_P) ||
					(dst[j].chip_id == USBTENKI_CHIP_MS5611_T)
				)
			)
		{
			int offset = 0;

			// A PT100 RTD interface with a MS5611 pressure sensor has
			// its MS5611 calibration after the PT100 calibration data...
			if ((dst[j].chip_id == USBTENKI_CHIP_MS5611_P) && dst[j].channel_id == 1) {
				offset = 1;
			}
			if ((dst[j].chip_id == USBTENKI_CHIP_MS5611_T) && dst[j].channel_id == 2) {
				offset = 1;
			}

			//printf("Fetching MS5611 calibration data (offset: %d)...\n", offset);
			for (n=0; n<num_attempts; n++) {
				caldata_len = usbtenki_getCalibration(hdl, offset + 0, caldata, sizeof(caldata)-offset);
				if (caldata_len<0) {
					usleep(200);
					continue;
				}
				break;
			}
			/* all attempts failed? */
			if (n==num_attempts) {
				return -1;
			}

			for (n=0; n<num_attempts; n++) {
				int l;
				l = usbtenki_getCalibration(hdl, offset + 1, caldata + caldata_len, sizeof(caldata)-offset);
				if (l<0) {
					usleep(200);
					continue;
				}
				caldata_len += l;
				break;
			}
			/* all attempts failed? */
			if (n==num_attempts) {
				return -1;
			}
			caldata_chip = USBTENKI_CHIP_MS5611_P;
//			printf("Received %d bytes of calibration\n", caldata_len);
		}

		res = usbtenki_convertRaw(&dst[j], flags, caldata, caldata_len);
		if (res==-1) {
			if (g_usbtenki_verbose) {
				fprintf(stderr, "Failed to convert raw value from chip %d, channel: %d\n",
								dst[j].chip_id, dst[j].channel_id);
			}
		}
	}

	return 0;
}

/**
 * \brief Populate an array of struct USBTEnki_channel from a device
 * \return The number of channels
 * This does not read the channels.
 */
int usbtenki_listChannels(USBTenki_dev_handle hdl, struct USBTenki_channel *dstArray, int arr_size)
{
	int n_channels;
	int i;

	n_channels = usbtenki_getNumChannels(hdl);
	for (i=0; i<n_channels && i<arr_size; i++){
		memset(dstArray, 0, sizeof(struct USBTenki_channel));
		dstArray->channel_id = i;
		dstArray->chip_id = usbtenki_getChipID(hdl, i);
		dstArray->status = USBTENKI_CHN_STATUS_UNDEFINED;

		dstArray++;
	}

	if (n_channels > arr_size)
		fprintf(stderr, "warning: Channel list truncated\n");

	return n_channels;
}

/**
 * \brief Add a virtual channel to a list of channels.
 * \param channels The channel list
 * \param num_channels Pointer to an integer representing the number of channel currently in list.
 * \param max_channels The maximum number of channels that can be present in 'channels'
 * \param channel The channel to add.
 */
static int addVirtualChannel(struct USBTenki_channel *channels, int *num_channels,
						int max_channels, struct USBTenki_channel *channel)
{
	if (*num_channels >= max_channels) {
		fprintf(stderr, "warning: Not enough space for all virtual channels\n");
		return -1;
	}

	if (g_usbtenki_verbose)
		printf("Adding channel to index %d\n", *num_channels);

	memcpy(&channels[*num_channels], channel, sizeof(struct USBTenki_channel));
	(*num_channels)++;

	return 0;
}

/**
 * \brief Search the list of channels for the channel_id corresponding to a specific chip id.
 */
static int chipIdToChannelId(struct USBTenki_channel *channels, int num_channels, int chip_id)
{
	int i;
	for (i=0; i<num_channels; i++)
	{
		if (channels[i].chip_id == chip_id) {
			if (g_usbtenki_verbose)
				printf("Chip %d is at channel %d\n", chip_id,
					channels[i].channel_id);
			return channels[i].channel_id;
		}
	}
	return -1;
}

/**
 * Returns a pointer to a specific channel_id from a list of channels, optionally
 * reading data from the device if the channel's data was not yet valid.
 * */
static struct USBTenki_channel *getValidChannel(USBTenki_dev_handle hdl, struct USBTenki_channel *channels, int num_channels, int requested_channel_id, unsigned long flags)
{
	int i, res;	

	for (i=0; i<num_channels; i++)
	{
		if (channels[i].channel_id == requested_channel_id)
		{
			if (g_usbtenki_verbose)
				printf("%s: found channel id %d at index %d\n" , __FUNCTION__,
							requested_channel_id, i);

			if (channels[i].status == USBTENKI_CHN_STATUS_VALID) {
				if (g_usbtenki_verbose)
					printf("Data already valid for this channel.\n");
				return &channels[i];
			}

			res = usbtenki_readChannelList(hdl, &requested_channel_id, 1, channels, num_channels, g_usbtenki_num_attempts, flags);
			if (res!=0) {
				fprintf(stderr, "Failed to read channel %d data from device! (%d)\n",
					requested_channel_id, res);
				return NULL;
			}

			if (channels[i].status == USBTENKI_CHN_STATUS_VALID) {
				if (g_usbtenki_verbose)
					printf("Data is now valid for this channel.\n");
				return &channels[i];
			}

		}
	}
	return NULL;
}

static struct USBTenki_channel *getValidChannelFromChip(USBTenki_dev_handle hdl, struct USBTenki_channel *channels, int num_channels, int requested_chip_id, unsigned long flags)
{
	int channel_id;

	channel_id = chipIdToChannelId(channels, num_channels, requested_chip_id);
	if (channel_id < 0)
		return NULL;
	return getValidChannel(hdl, channels, num_channels, channel_id, flags);
}

static struct USBTenki_channel *getValidChannelFromChip_list(USBTenki_dev_handle hdl, struct USBTenki_channel *channels, int num_channels, int requested_chip_ids[], int num_requested_chip_ids, unsigned long flags)
{
	int i;
	struct USBTenki_channel *chn;

	for (i=0; i<num_requested_chip_ids; i++) {
		chn = getValidChannelFromChip(hdl, channels, num_channels, requested_chip_ids[i], flags);
		if (chn)
			return chn;
	}
	return NULL;
}


static double HeightFromPressure(double P, double static_pressure_P)
{
	// constants
	double g0 = 9.80665; // Gravitatinal acceleration in m/s^2
	double M = 0.0289644; // Earth's air molar mass in kg/mol
	double R = 8.31432; // Universal gas constant N*m/mol*K

	// Values for 0-11000 meters
	//double Pb = 101325; // static pressure (pascals)
	double Pb = static_pressure_P; // static pressure (pascals)
	double Lb = -0.0065; // K/m
	double Tb = 288.15; // standard temperature
//	double hb = 0; // height at bottom of layer 'b'

	// temp
	double x = (g0 * M) / (R * Lb);
	double y = pow((P / Pb), 1/x);

	return (Tb / y - Tb) / Lb;
}

static double standard_sea_level_pressure = 101325;

void usbtenki_set_seaLevelStandardPressure(double slp_P)
{
	standard_sea_level_pressure = slp_P;
}

/* \brief Compute virtual channels based on real channels (already read), possiblity reading dependant channels.
 */
int usbtenki_processVirtualChannels(USBTenki_dev_handle hdl, struct USBTenki_channel channels[], int num_channels, unsigned long flags)
{
	return usbtenki_processSomeVirtualChannels(hdl, channels, num_channels, NULL, -1, flags);
}

int usbtenki_processSomeVirtualChannels(USBTenki_dev_handle hdl, struct USBTenki_channel channels[], int num_channels, const int requested_channels[], int num_req_chns, unsigned long flags)
{
	int i, j;
	struct USBTenki_channel *chn;
	int temp_sources[] = { USBTENKI_CHIP_SHT_TEMP, USBTENKI_CHIP_BS02_TEMP,
		USBTENKI_CHIP_CC2_T, USBTENKI_CHIP_SHT31_T, USBTENKI_CHIP_SHT35_T, USBTENKI_CHIP_SHT31_T_EXTERNAL };
	int rh_sources[] = { USBTENKI_CHIP_SHT_RH, USBTENKI_CHIP_BS02_RH,
		USBTENKI_CHIP_CC2_RH, USBTENKI_CHIP_SHT31_RH, USBTENKI_CHIP_SHT35_RH, USBTENKI_CHIP_SHT31_RH_EXTERNAL };

	for (i=0; i<num_channels; i++)
	{
		chn = &channels[i];

		if (chn->channel_id < USBTENKI_VIRTUAL_START)
			continue;

		for (j=0; j<num_req_chns; j++) {
			if (chn->channel_id == requested_channels[j])
				break;
		}
		if (j==num_req_chns) {
			if (flags & USBTENKI_FLAG_VERBOSE) {
				printf("Skipping unrequested channel %d\n", chn->channel_id);
			}
			continue;
		}

		switch(chn->channel_id)
			{
				case USBTENKI_VIRTUAL_TSL2568_LUX:
					{
						struct USBTenki_channel *vir_chn, *ir_chn;
						double ch0,ch1,lx;

						ir_chn = getValidChannelFromChip(hdl, channels, num_channels,
														USBTENKI_CHIP_TSL2568_IR, flags);

						vir_chn = getValidChannelFromChip(hdl, channels, num_channels,
														USBTENKI_CHIP_TSL2568_IR_VISIBLE, flags);

						if (ir_chn==NULL || vir_chn==NULL) {
							fprintf(stderr, "Failed to read channels required for computing virtual channel!\n");
							return -1;
						}

						ch0 = vir_chn->converted_data;
						ch1 = ir_chn->converted_data;

						if ((vir_chn->converted_data < 3000) && (ir_chn->converted_data < 3000)) {
							struct USBTenki_channel *vir_chn_g, *ir_chn_g;
							double ch0_g, ch1_g;

							/* Based on these values, a 16x gain would not overflow. */
//							printf("Switching to 16x gain\n");
							ir_chn_g = getValidChannelFromChip(hdl, channels, num_channels,
														USBTENKI_CHIP_TSL2568_IR_16X, flags);

							vir_chn_g = getValidChannelFromChip(hdl, channels, num_channels,
														USBTENKI_CHIP_TSL2568_IR_VISIBLE_16X, flags);

							if (ir_chn_g==NULL || vir_chn_g==NULL) {
								fprintf(stderr, "Failed to read channels required for computing virtual channel!\n");
								return -1;
							}

							ch0_g = vir_chn_g->converted_data;
							ch1_g = ir_chn_g->converted_data;

//							printf("%.f %.f %.f %.f\n", ch0, ch0_g/16.0, ch1, ch1_g/16.0);

							if (ir_chn_g != NULL && vir_chn_g != NULL) {
								// stick to the low gain channels in case of saturation
								if (ir_chn_g->converted_data != 65535 &&
									vir_chn_g->converted_data != 65535) {
									ch0 = ch0_g / 16.0;
									ch1 = ch1_g / 16.0;
								}
							}
						}

						if (ch0 > 65534 || ch1 > 65534) {
							chn->status = USBTENKI_CHN_STATUS_SATURATED;
							chn->converted_data = -1;
							chn->converted_unit = TENKI_UNIT_LUX;
							break;
						}

						/*
						TMB Package
							For 0 < CH1/CH0 < 0.35     Lux = 0.00763  CH0 - 0.01031  CH1
							For 0.35 < CH1/CH0 < 0.50  Lux = 0.00817  CH0 - 0.01188  CH1
							For 0.50 < CH1/CH0 < 0.60  Lux = 0.00723  CH0 - 0.01000  CH1
							For 0.60 < CH1/CH0 < 0.72  Lux = 0.00573  CH0 - 0.00750  CH1
							For 0.72 < CH1/CH0 < 0.85  Lux = 0.00216  CH0 - 0.00254  CH1
							For CH1/CH0 > 0.85         Lux = 0
						*/
						if (ch1/ch0 < 0.35)
							lx = 0.00763 * ch0 - 0.01031 * ch1;
						else if (ch1/ch0 < 0.50)
							lx = 0.00817 * ch0 - 0.01188 * ch1;
						else if (ch1/ch0 < 0.60)
							lx = 0.00723 * ch0 - 0.01000 * ch1;
						else if (ch1/ch0 < 0.72)
							lx = 0.00573 * ch0 - 0.00750 * ch1;
						else if (ch1/ch0 < 0.85)
							lx = 0.00216 * ch0 - 0.00254 * ch1;
						else
							lx = 0.0;

//						printf("ch1: %f, ch0: %f, lx: %f\n", ch1, ch0, lx);

						chn->status = USBTENKI_CHN_STATUS_VALID;
						chn->converted_data = lx;
						chn->converted_unit = TENKI_UNIT_LUX;
					}
					break;


				case USBTENKI_VIRTUAL_TSL2561_LUX:
					{
						struct USBTenki_channel *vir_chn, *ir_chn;
						float ch0,ch1,lx;

						ir_chn = getValidChannelFromChip(hdl, channels, num_channels,
														USBTENKI_CHIP_TSL2561_IR, flags);

						vir_chn = getValidChannelFromChip(hdl, channels, num_channels,
														USBTENKI_CHIP_TSL2561_IR_VISIBLE, flags);

						if (ir_chn==NULL || vir_chn==NULL) {
							fprintf(stderr, "Failed to read channels required for computing virtual channel!\n");
							return -1;
						}

						ch0 = vir_chn->converted_data;
						ch1 = ir_chn->converted_data;

						/*
						 * TMB Package
						 *
						 * For 0 < CH1/CH0 <= 0.50 		Lux = 0.0304 * CH0 - .062 * CH0 * ((CH1/CH0)1.4)
						 * For 0.50 < CH1/CH0 <=  0.61 	Lux = 0.0224 * CH0 - .031 * CH1
						 * For 0.61 < CH1/CH0 <= 0.80 	Lux = 0.0128 * CH0 - .0153 * CH1
						 * For 0.80 < CH1/CH0 <= 1.30 	Lux = 0.00146 *  CH0 - .00112 * CH1
						 * For CH1/CH0 > 1.30			Lux = 0
						 *
						 */
						if (ch1/ch0 < 0.50)
							lx = 0.0304 * ch0 - 0.062 * ch0 * (pow((ch1/ch0),1.4));
						else if (ch1/ch0 < 0.61)
							lx = 0.0224 * ch0 - 0.031 * ch1;
						else if (ch1/ch0 < 0.80)
							lx = 0.0128 * ch0 - 0.0153 * ch1;
						else
							lx = 0.0;

//						printf("ch1: %f, ch0: %f, lx: %f\n", ch1, ch0, lx);

						chn->status = USBTENKI_CHN_STATUS_VALID;
						chn->converted_data = lx;
						chn->converted_unit = TENKI_UNIT_LUX;
					}
					break;

				case USBTENKI_VIRTUAL_SHT75_COMPENSATED_RH:
					{
						struct USBTenki_channel *temp_chn, *rh_chn;
						float T, RH_linear, RH_true;
						float SOrh;

						if (g_usbtenki_verbose)
							printf("Processing dew point virtual channel\n");

						temp_chn = getValidChannelFromChip(hdl, channels, num_channels, USBTENKI_CHIP_SHT_TEMP, flags);
						rh_chn = getValidChannelFromChip(hdl, channels, num_channels, USBTENKI_CHIP_SHT_RH, flags);

						if (temp_chn == NULL || rh_chn == NULL) {
							fprintf(stderr, "Failed to read channels required for computing virtual channel!\n");
							return -1;
						}

						// According to SHT75 Datasheet Version 5 (December 2011)
						T = temp_chn->converted_data;
						RH_linear = rh_chn->converted_data;
						SOrh = rh_chn->raw_value;

						RH_true = (T - 25)*(0.01 + 0.00008 * SOrh)+RH_linear;
						if (RH_true < 0)
							RH_true = 0;
						if (RH_true > 100)
							RH_true = 100;

						chn->status = USBTENKI_CHN_STATUS_VALID;
						chn->converted_data = RH_true;
						chn->converted_unit = TENKI_UNIT_RH;
					}
					break;

				case USBTENKI_VIRTUAL_INTERNAL_DEW_POINT:
					{
						struct USBTenki_channel *temp_chn, *rh_chn;
						float H, Dp, T;
						int temp_sources[] = { USBTENKI_CHIP_SHT31_T_INTERNAL };
						int rh_sources[] = { USBTENKI_CHIP_SHT31_RH_INTERNAL };

						if (g_usbtenki_verbose)
							printf("Processing dew point virtual channel\n");

						temp_chn = getValidChannelFromChip_list(hdl, channels, num_channels, temp_sources, ARRAY_SIZE(temp_sources), flags);
						rh_chn = getValidChannelFromChip_list(hdl, channels, num_channels, rh_sources, ARRAY_SIZE(rh_sources), flags);

						if (temp_chn == NULL || rh_chn == NULL) {
							fprintf(stderr, "Failed to read channels required for computing virtual channel!\n");
							return -1;
						}

						T = temp_chn->converted_data;
						H = (log10(rh_chn->converted_data)-2.0)/0.4343 +
							(17.62*T)/(243.12+T);
						Dp = 243.12 * H / (17.62 - H);

						chn->status = USBTENKI_CHN_STATUS_VALID;
						chn->converted_data = Dp;
						chn->converted_unit = TENKI_UNIT_CELCIUS;
					}
					break;

				case USBTENKI_VIRTUAL_DEW_POINT:
					{
						struct USBTenki_channel *temp_chn, *rh_chn;
						float H, Dp, T;
						int temp_sources[] = { USBTENKI_CHIP_SHT_TEMP, USBTENKI_CHIP_BS02_TEMP, USBTENKI_CHIP_CC2_T, USBTENKI_CHIP_SHT31_T, USBTENKI_CHIP_SHT35_T, USBTENKI_CHIP_SHT31_T_EXTERNAL };
						int rh_sources[] = { USBTENKI_CHIP_SHT_RH, USBTENKI_CHIP_BS02_RH, USBTENKI_CHIP_CC2_RH, USBTENKI_CHIP_SHT31_RH, USBTENKI_CHIP_SHT35_RH, USBTENKI_CHIP_SHT31_RH_EXTERNAL };

						if (g_usbtenki_verbose)
							printf("Processing dew point virtual channel\n");

						temp_chn = getValidChannelFromChip_list(hdl, channels, num_channels, temp_sources, ARRAY_SIZE(temp_sources), flags);
						rh_chn = getValidChannelFromChip_list(hdl, channels, num_channels, rh_sources, ARRAY_SIZE(rh_sources), flags);

						if (temp_chn == NULL || rh_chn == NULL) {
							fprintf(stderr, "Failed to read channels required for computing virtual channel!\n");
							return -1;
						}

						T = temp_chn->converted_data;
						H = (log10(rh_chn->converted_data)-2.0)/0.4343 +
							(17.62*T)/(243.12+T);
						Dp = 243.12 * H / (17.62 - H);

						chn->status = USBTENKI_CHN_STATUS_VALID;
						chn->converted_data = Dp;
						chn->converted_unit = TENKI_UNIT_CELCIUS;
					}
					break;

				case USBTENKI_VIRTUAL_HUMIDEX:
					{
						struct USBTenki_channel *temp_chn, *rh_chn;
						float H, Dp, T, h, e;
						int non_significant = 0;

						if (g_usbtenki_verbose)
							printf("Processing humidex virtual channel\n");

						temp_chn = getValidChannelFromChip_list(hdl, channels, num_channels, temp_sources, ARRAY_SIZE(temp_sources), flags);
						rh_chn = getValidChannelFromChip_list(hdl, channels, num_channels, rh_sources, ARRAY_SIZE(rh_sources), flags);

						if (temp_chn == NULL || rh_chn == NULL) {
							fprintf(stderr, "Failed to read channels required for computing virtual channel!\n");
							return -1;
						}

						T = temp_chn->converted_data;
						H = (log10(rh_chn->converted_data)-2.0)/0.4343 + (17.62*T)/(243.12+T);
						Dp = 243.12 * H / (17.62 - H);

						if (!(flags & USBTENKI_FLAG_NO_HUMIDEX_RANGE))
						{
							if (Dp < 0) {
								// Weatheroffice.gc.ca: We only display humidex values of 25 or higher for a
								// location which reports a dew point temperature above zero (0°C) ...
								non_significant = 1;
							}
							if (T < 20) {
								// ... AND an air temperature of 20°C or more.
								//
								non_significant = 1;
							}
						}

						/* We need dewpoint in kelvins... */
						Dp = usbtenki_convertTemperature(Dp, TENKI_UNIT_CELCIUS, TENKI_UNIT_KELVIN);

						e = 6.11 * exp(5417.7530 * ((1.0/273.16) - (1.0/Dp)));
						h = (5.0/9.0)*(e - 10.0);

						if (!non_significant) {
							chn->converted_data = T + h;
							chn->converted_unit = TENKI_UNIT_CELCIUS;
						} else {
							chn->converted_data = T;
							chn->converted_unit = TENKI_UNIT_CELCIUS;
						}

						chn->status = USBTENKI_CHN_STATUS_VALID;
					}
					break;

				case USBTENKI_VIRTUAL_HEAT_INDEX:
					{
						struct USBTenki_channel *temp_chn, *rh_chn;
						float T, R, HI;
						int out_of_range = 0;

						if (g_usbtenki_verbose)
							printf("Processing heat index virtual channel\n");

						temp_chn = getValidChannelFromChip_list(hdl, channels, num_channels, temp_sources, ARRAY_SIZE(temp_sources), flags);
						rh_chn = getValidChannelFromChip_list(hdl, channels, num_channels, rh_sources, ARRAY_SIZE(rh_sources), flags);

						if (temp_chn == NULL || rh_chn == NULL) {
							fprintf(stderr, "Failed to read channels required for computing virtual channel!\n");
							return -1;
						}

						T = temp_chn->converted_data;
						T =  usbtenki_convertTemperature(T, TENKI_UNIT_CELCIUS,
															TENKI_UNIT_FAHRENHEIT);
						R = rh_chn->converted_data;

						/* Formula source:
						 * Initially: http://www.crh.noaa.gov/jkl/?n=heat_index_calculator (2012: 404 error)
						 *
						 * http://en.wikipedia.org/wiki/Heat_index#Formula
						 */
						HI = 	-42.379 +
								2.04901523 * T +
								10.14333127 * R -
								0.22475541 * T * R -
								6.83783 * pow(10,-3) * pow(T, 2) -
								5.481717 * pow(10,-2) * pow(R, 2) +
								1.22874 * pow(10,-3) * pow(T, 2) * R +
								8.5282 * pow(10,-4) * T * pow(R, 2) -
								1.99 * pow(10,-6) * pow(T,2) * pow(R,2);

						if (!(flags & USBTENKI_FLAG_NO_HEAT_INDEX_RANGE)) {
							if (T < 80 || R < 40) {
								out_of_range = 1;
							}
						}

						if (out_of_range) {
							chn->status = USBTENKI_CHN_STATUS_VALID;
							chn->converted_data = T;
							chn->converted_unit = TENKI_UNIT_FAHRENHEIT;
						} else {
							chn->status = USBTENKI_CHN_STATUS_VALID;
							chn->converted_data = HI;
							chn->converted_unit = TENKI_UNIT_FAHRENHEIT;
						}
					}
					break;

				case USBTENKI_VIRTUAL_ALTITUDE:
					{
						struct USBTenki_channel *pressure_chn;
						double P;

						if (g_usbtenki_verbose)
							printf("Processing altitude virtual channel\n");

						pressure_chn = getValidChannelFromChip(hdl, channels, num_channels, USBTENKI_CHIP_MS5611_P, flags);
						if (!pressure_chn) {
							fprintf(stderr, "Failed to read pressure channel\n");
							return -1;
						}

						P = usbtenki_convertPressure(pressure_chn->converted_data, pressure_chn->converted_unit, TENKI_UNIT_KPA);
						chn->status = USBTENKI_CHN_STATUS_VALID;
						chn->converted_unit = TENKI_UNIT_METERS;
						chn->converted_data = HeightFromPressure(P * 1000, standard_sea_level_pressure);
					}
					break;

				case USBTENKI_CHIP_HEXCOLOR:
					{
						struct USBTenki_channel *red_chn;
						struct USBTenki_channel *blue_chn;
						struct USBTenki_channel *green_chn;
						uint32_t color;
						double max = 0.0;
						uint8_t r,g,b;

						red_chn = getValidChannelFromChip(hdl, channels, num_channels, USBTENKI_CHIP_RED, flags);
						blue_chn = getValidChannelFromChip(hdl, channels, num_channels, USBTENKI_CHIP_BLUE, flags);
						green_chn = getValidChannelFromChip(hdl, channels, num_channels, USBTENKI_CHIP_GREEN, flags);
						if (!red_chn || !blue_chn || !green_chn) {
							return -1;
						}

						max = red_chn->converted_data;
						if (green_chn->converted_data > max)
							max = green_chn->converted_data;
						if (blue_chn->converted_data > max)
							max = blue_chn->converted_data;

						r = red_chn->converted_data / max * 255.0;
						g = green_chn->converted_data / max * 255.0;
						b = blue_chn->converted_data / max * 255.0;

						color = r<<16 | g<<8 | b;
						chn->status = USBTENKI_CHN_STATUS_VALID;
						chn->converted_unit = TENKI_UNIT_HEXCOLOR;
						chn->converted_data = color;
					}
					break;
			}

	}

	return 0;
}

int usbtenki_addVirtualChannels(struct USBTenki_channel *channels, int *num_channels,
															int max_channels)
{
	int i;
	struct USBTenki_channel chn;
	int real_channels = *num_channels;

	memset(&chn, 0, sizeof(struct USBTenki_channel));

	/* Dew point, humidex and heat index from Temp+RH for sensirion sht1x/7x and BS02 */
	if (1)
	{
		int hfound=0, tfound=0;
		for (i=0; i<real_channels; i++)
		{
			if (channels[i].chip_id == USBTENKI_CHIP_SHT_TEMP)
				tfound = channels[i].chip_id;
			if (channels[i].chip_id == USBTENKI_CHIP_SHT_RH)
				hfound = channels[i].chip_id;
			if (channels[i].chip_id == USBTENKI_CHIP_BS02_TEMP)
				tfound = channels[i].chip_id;
			if (channels[i].chip_id == USBTENKI_CHIP_BS02_RH)
				hfound = channels[i].chip_id;
			if (channels[i].chip_id == USBTENKI_CHIP_CC2_T)
				tfound = channels[i].chip_id;
			if (channels[i].chip_id == USBTENKI_CHIP_CC2_RH)
				hfound = channels[i].chip_id;
			if (channels[i].chip_id == USBTENKI_CHIP_SHT31_T)
				tfound = channels[i].chip_id;
			if (channels[i].chip_id == USBTENKI_CHIP_SHT31_RH)
				hfound = channels[i].chip_id;
			if (channels[i].chip_id == USBTENKI_CHIP_SHT35_T)
				tfound = channels[i].chip_id;
			if (channels[i].chip_id == USBTENKI_CHIP_SHT35_RH)
				hfound = channels[i].chip_id;
			if (channels[i].chip_id == USBTENKI_CHIP_SHT31_T_EXTERNAL)
				tfound = channels[i].chip_id;
			if (channels[i].chip_id == USBTENKI_CHIP_SHT31_RH_EXTERNAL)
				hfound = channels[i].chip_id;
		}

		if (hfound && tfound) {
			chn.channel_id = USBTENKI_VIRTUAL_DEW_POINT;
			chn.chip_id = chn.channel_id;
			chn.status = USBTENKI_CHN_STATUS_UNDEFINED;
			chn.converted_data = 0.0;
			chn.converted_unit = 0;
			if (addVirtualChannel(channels, num_channels, max_channels, &chn))
				return -1;

			chn.channel_id = USBTENKI_VIRTUAL_HUMIDEX;
			chn.chip_id = chn.channel_id;

			if (addVirtualChannel(channels, num_channels, max_channels, &chn))
				return -1;

			chn.channel_id = USBTENKI_VIRTUAL_HEAT_INDEX;
			chn.chip_id = chn.channel_id;

			if (addVirtualChannel(channels, num_channels, max_channels, &chn))
				return -1;


			if (tfound == USBTENKI_CHIP_SHT_TEMP &&
				hfound == USBTENKI_CHIP_SHT_RH) {

				chn.channel_id = USBTENKI_VIRTUAL_SHT75_COMPENSATED_RH;
				chn.chip_id = chn.channel_id;
				if (addVirtualChannel(channels, num_channels, max_channels, &chn))
					return -1;
			}
		}
	}

	/* Dew point for internal sensor */
	if (1)
	{
		int hfound=0, tfound=0;

		for (i=0; i<real_channels; i++)
		{
			hfound = channels[i].chip_id;
			if (channels[i].chip_id == USBTENKI_CHIP_SHT31_T_INTERNAL)
				tfound = channels[i].chip_id;
			if (channels[i].chip_id == USBTENKI_CHIP_SHT31_RH_INTERNAL)
				hfound = channels[i].chip_id;
		}

		if (hfound && tfound) {
			chn.channel_id = USBTENKI_VIRTUAL_INTERNAL_DEW_POINT;
			chn.chip_id = chn.channel_id;
			chn.status = USBTENKI_CHN_STATUS_UNDEFINED;
			chn.converted_data = 0.0;
			chn.converted_unit = 0;
			if (addVirtualChannel(channels, num_channels, max_channels, &chn))
				return -1;
		}
	}

	/* Lux calculated using Visible + IR and IR only channels from TSL2561 sensor */
	if (1)
	{
		int vir_found=0, ir_found=0;
		for (i=0; i<real_channels; i++)
		{
			if (channels[i].chip_id == USBTENKI_CHIP_TSL2561_IR_VISIBLE)
				vir_found = 1;
			if (channels[i].chip_id == USBTENKI_CHIP_TSL2561_IR)
				ir_found = 1;
		}

		if (vir_found && ir_found) {
			chn.channel_id = USBTENKI_VIRTUAL_TSL2561_LUX;
			chn.chip_id = chn.channel_id;
			chn.status = USBTENKI_CHN_STATUS_UNDEFINED;
			chn.converted_data = 0.0;
			chn.converted_unit = 0;
			if (addVirtualChannel(channels, num_channels, max_channels, &chn))
				return -1;
		}
	}

	/* Lux calculated using Visible + IR and IR only channels from TSL2568 sensor */
	if (1)
	{
		int vir_found=0, ir_found=0;
		for (i=0; i<real_channels; i++)
		{
			if (channels[i].chip_id == USBTENKI_CHIP_TSL2568_IR_VISIBLE)
				vir_found = 1;
			if (channels[i].chip_id == USBTENKI_CHIP_TSL2568_IR)
				ir_found = 1;
		}

		if (vir_found && ir_found) {
			chn.channel_id = USBTENKI_VIRTUAL_TSL2568_LUX;
			chn.chip_id = chn.channel_id;
			chn.status = USBTENKI_CHN_STATUS_UNDEFINED;
			chn.converted_data = 0.0;
			chn.converted_unit = 0;
			if (addVirtualChannel(channels, num_channels, max_channels, &chn))
				return -1;
		}
	}

	// Experiment: Arbitrary RGB intensity to Hex color. Does not work very well.
	if (NULL != getenv("TENKI_HEXCOLOR"))
	{
		unsigned char wanted[3] = { USBTENKI_CHIP_RED, USBTENKI_CHIP_GREEN, USBTENKI_CHIP_BLUE };
		unsigned int j, found = 0;

		for (i=0; i<real_channels; i++)
		{
			for (j=0; j<sizeof(wanted); j++) {
				if (wanted[j] == channels[i].chip_id) {
					wanted[j] = 0;
					found++;
				}
			}
		}

		if (found == sizeof(wanted)) {
			chn.channel_id = USBTENKI_CHIP_HEXCOLOR;
			chn.chip_id = chn.channel_id;
			chn.status = USBTENKI_CHN_STATUS_UNDEFINED;
			chn.converted_data = 0;
			chn.converted_unit = 0;
			if (addVirtualChannel(channels, num_channels, max_channels, &chn))
				return -1;
		}
	}

	if (1)
	{
		for (i=0; i<real_channels; i++)
		{
			if (channels[i].chip_id == USBTENKI_CHIP_MS5611_P)
			{
				chn.channel_id = USBTENKI_VIRTUAL_ALTITUDE;
				chn.chip_id = chn.channel_id;
				chn.status = USBTENKI_CHN_STATUS_UNDEFINED;
				chn.converted_data = 0.0;
				chn.converted_unit = 0;
				if (addVirtualChannel(channels, num_channels, max_channels, &chn))
					return -1;
				}
		}
	}

	return 0;
}

void usbtenki_convertUnits(struct USBTenki_channel *chn, const struct USBTenki_unitPreferences *units)
{
	/* Perform format conversion */
	switch (chn->converted_unit)
	{
		case TENKI_UNIT_METERS:
		case TENKI_UNIT_DECIMETERS:
		case TENKI_UNIT_CENTIMETERS:
		case TENKI_UNIT_MILLIMETERS:
		case TENKI_UNIT_MILS:
		case TENKI_UNIT_INCHES:
		case TENKI_UNIT_FEET:
		case TENKI_UNIT_YARDS:
			chn->converted_data = usbtenki_convertLength(chn->converted_data,
															chn->converted_unit,
																		units->length);
			chn->converted_unit = units->length;
			break;

		case TENKI_UNIT_FAHRENHEIT:
		case TENKI_UNIT_CELCIUS:
		case TENKI_UNIT_KELVIN:
			chn->converted_data = usbtenki_convertTemperature(chn->converted_data,
																chn->converted_unit,
																		units->temperature);
			chn->converted_unit = units->temperature;
			break;

		case TENKI_UNIT_KPA:
		case TENKI_UNIT_HPA:
		case TENKI_UNIT_BAR:
		case TENKI_UNIT_AT:
		case TENKI_UNIT_ATM:
		case TENKI_UNIT_TORR:
		case TENKI_UNIT_PSI:
		case TENKI_UNIT_INHG:
			chn->converted_data = usbtenki_convertPressure(chn->converted_data,
															chn->converted_unit,
																units->pressure);
			chn->converted_unit = units->pressure;
			break;

		case TENKI_UNIT_MILLIHZ:
		case TENKI_UNIT_HZ:
		case TENKI_UNIT_KHZ:
		case TENKI_UNIT_MHZ:
			chn->converted_data = usbtenki_convertFrequency(chn->converted_data,
															chn->converted_unit,
															units->frequency);
			chn->converted_unit = units->frequency;
			break;

		case TENKI_UNIT_VOLTS:
		case TENKI_UNIT_MILLIVOLT:
			chn->converted_data = usbtenki_convertVoltage(chn->converted_data,
															chn->converted_unit,
															units->voltage);
			chn->converted_unit = units->voltage;
			break;

		case TENKI_UNIT_AMPS:
		case TENKI_UNIT_MILLIAMPS:
			chn->converted_data = usbtenki_convertCurrent(chn->converted_data,
															chn->converted_unit,
															units->current);
			chn->converted_unit = units->current;
			break;

		case TENKI_UNIT_KILOWATTS:
		case TENKI_UNIT_WATTS:
		case TENKI_UNIT_MILLIWATTS:
			chn->converted_data = usbtenki_convertPower(chn->converted_data,
															chn->converted_unit,
															units->power);
			chn->converted_unit = units->power;
			break;

		case TENKI_UNIT_PPB:
		case TENKI_UNIT_PPM:
		case TENKI_UNIT_PERCENT:
			chn->converted_data = usbtenki_convertConcentration(chn->converted_data,
																chn->converted_unit,
																units->concentration,
																&chn->converted_unit);
			break;
	}
}

const char *usbtenki_getChannelStatusStringNoSpaces(const struct USBTenki_channel *chn)
{
	if (!chn) {
		return "Undefined";
	}

	switch (chn->status)
	{
		default:
		case USBTENKI_CHN_STATUS_UNDEFINED:
			return "Undefined";
		case USBTENKI_CHN_STATUS_VALID:
			return "Valid";
		case USBTENKI_CHN_STATUS_SATURATED:
			return "Saturated";
		case USBTENKI_CHN_STATUS_SENSOR_ERROR:
			return "SensorError";
		case USBTENKI_CHN_STATUS_PROBE_DISCONNECTED:
			return "ProbeDisconnected";
		case USBTENKI_CHN_STATUS_OUT_OF_RANGE:
			return "OutOfRange";
		case USBTENKI_CHN_STATUS_INVALID_DATA:
			return "InvalidData";
	}
}

const char *usbtenki_getChannelStatusString(const struct USBTenki_channel *chn)
{
	if (!chn) {
		return "Undefined";
	}

	switch (chn->status)
	{
		default:
		case USBTENKI_CHN_STATUS_UNDEFINED:
			return "Undefined";
		case USBTENKI_CHN_STATUS_VALID:
			return "Valid";
		case USBTENKI_CHN_STATUS_SATURATED:
			return "Saturated";
		case USBTENKI_CHN_STATUS_SENSOR_ERROR:
			return "Sensor Error";
		case USBTENKI_CHN_STATUS_PROBE_DISCONNECTED:
			return "Probe Disconnected";
		case USBTENKI_CHN_STATUS_OUT_OF_RANGE:
			return "Out Of Range";
		case USBTENKI_CHN_STATUS_INVALID_DATA:
			return "Invalid Data";
	}
}

