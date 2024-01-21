#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "TenkiDevice.h"
#include "../common/usbtenki_cmds.h"
#include "globals.h"

TenkiDevice::TenkiDevice()
{
	serno = NULL;
	num_channels = 0;
	memset(&tenki_info, 0 ,sizeof(tenki_info));
	memset(&channel_data, 0 ,sizeof(channel_data));
}

TenkiDevice::TenkiDevice(const char *serial)
{
	serno = strdup(serial);
	num_channels = 0;
	memset(&tenki_info, 0 ,sizeof(tenki_info));

	tenki_hdl = usbtenki_openBySerial(serial, &tenki_info);
	if (tenki_hdl != NULL) {
		status = TENKI_DEVICE_STATUS_OK;
		initChannels();
//		updateChannelData();
	} else {
		status = TENKI_DEVICE_STATUS_UNABLE_TO_OPEN;
	}

}

TenkiDevice::~TenkiDevice()
{
	if (tenki_hdl) {
		usbtenki_closeDevice(tenki_hdl);
		tenki_hdl = 0;
	}
	if (serno) {
		free(serno);
	}
}

void TenkiDevice::setSerial(const char *serial)
{
	if (serno) {
		free(serno);
	}

	serno = strdup(serial);
}

void TenkiDevice::initChannels()
{
	if (!tenki_hdl)
		return;

	num_channels = usbtenki_getNumChannels(tenki_hdl);
	if (num_channels > MAX_CHANNELS)
		num_channels = MAX_CHANNELS;

	usbtenki_listChannels(tenki_hdl, channel_data, MAX_CHANNELS);

	// Add the virtual channels we can obtain from the real ones to
	// the list.
	usbtenki_addVirtualChannels(channel_data, &num_channels, MAX_CHANNELS);
}

int TenkiDevice::updateChannelData()
{
	int i, res, error=0, need_retry=0;
	struct USBTenki_channel tmpdata;

	do
	{
		// Having no handle at this point means there was an error. This is
		// either an immediate retry (need_retry=1) or a retry delayed
		// to the next capture cycle.
		if (!tenki_hdl) {
			tenki_hdl = usbtenki_openBySerial(serno, &tenki_info);
			if (tenki_hdl != NULL) {
				status = TENKI_DEVICE_STATUS_OK;
				initChannels();
			} else {
				// just get out	
				status = TENKI_DEVICE_STATUS_UNABLE_TO_OPEN;
				return -1;
			}

			// success!
		}

		// Everything ok. Try to read the channel(s).
//		printf("%s updating...\n", serno);
		for (need_retry=0,error=0,i=0; i<num_channels; i++) {
			if (channel_data[i].chip_id == USBTENKI_CHIP_NONE)
				continue;

			memcpy(&tmpdata, &channel_data[i], sizeof(struct USBTenki_channel));
			tmpdata.status = USBTENKI_CHN_STATUS_UNDEFINED;

			res = usbtenki_readChannel(tenki_hdl, &tmpdata, g_usbtenki_flags);
			if (res) {
				error = 1;
				break;
			}

			memcpy(&channel_data[i], &tmpdata, sizeof(struct USBTenki_channel));
		}

		if (error) {
//			printf("Error reading '%s', reopening...\n", serno);
			usbtenki_closeDevice(tenki_hdl);
			tenki_hdl = 0;

			// If we are here from a retry, don't retry again.
			if (need_retry) {
				break;
			} else {
//				printf("Retry...\n");
				need_retry = 1;
			}
		}
	}
	while(need_retry);

	if (tenki_hdl) {
		usbtenki_processVirtualChannels(tenki_hdl, channel_data, num_channels, g_usbtenki_flags);
	}

	return 0;
}

int TenkiDevice::isChannelHidden(int id)
{
	return (channel_data[id].chip_id == USBTENKI_CHIP_NONE);
}

const char *TenkiDevice::getSerial(void)
{
	return serno;

	//return tenki_info.str_serial;
}

int TenkiDevice::getNumChannels(void)
{
	return num_channels;
}

int TenkiDevice::getVisibleChannels(void)
{
	int i, nc = getNumChannels(), vc = 0;

	for (i=0; i<nc; i++) {
		if (!isChannelHidden(i)) {
			vc++;
		}
	}

	return vc;
}

int TenkiDevice::getStatus(void)
{
	return status;
}

USBTenki_channel *TenkiDevice::getChannelData(int id)
{
	return &channel_data[id];
}

const char *TenkiDevice::getProductName(void)
{
	return tenki_info.str_prodname;
}

void TenkiDevice::getFirmwareVersion(int *major, int *minor)
{
	if (major)
		*major = tenki_info.major;
	if (minor)
		*minor = tenki_info.minor;
}
