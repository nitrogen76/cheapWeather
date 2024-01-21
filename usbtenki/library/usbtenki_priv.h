#ifndef _usbtenki_priv_h__
#define _usbtenki_priv_h__

#ifdef USE_OLD_LIBUSB
#include <usb.h>
#else
#include "libusb.h"
#endif

struct USBTenki_list_ctx {
#ifdef USE_OLD_LIBUSB
	struct usb_bus *bus;
	struct usb_device *dev;
#else
	libusb_device **devices;
	ssize_t n_devices;
	int cur_device_index;
#endif
};


#endif
