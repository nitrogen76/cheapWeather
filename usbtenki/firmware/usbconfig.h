/* DracalTenki - Interfacing sensors to USB
 * Copyright (C) 2013 Dracal technologies inc. All right reserved.
 *
 * Uses V-USB licensed to Dracal technologies inc. by Objective Development.
 *
 * Base on original USB-Tenki code licensed to Dracal technolgies inc. by Raphaël Assénat.
 * Original work Copyright (C) 2007-2011  Raphaël Assénat <raph@raphnet.net>
 */
#ifndef __usbconfig_h_included__
#define __usbconfig_h_included__

#define USB_CFG_IOPORTNAME      D
#define USB_CFG_DMINUS_BIT      0
#define USB_CFG_DPLUS_BIT       2
#define USB_CFG_CLOCK_KHZ       (F_CPU/1000)
#define USB_CFG_CHECK_CRC       0

#define USB_CFG_HAVE_INTRIN_ENDPOINT    0
#define USB_CFG_HAVE_INTRIN_ENDPOINT3   0
#define USB_CFG_EP3_NUMBER              3
/* #define USB_INITIAL_DATATOKEN           USBPID_DATA1 */
#define USB_CFG_IMPLEMENT_HALT          0
#define USB_CFG_SUPPRESS_INTR_CODE      0
#define USB_CFG_INTR_POLL_INTERVAL      10
#define USB_CFG_IS_SELF_POWERED         0
#define USB_CFG_MAX_BUS_POWER           50
#define USB_CFG_IMPLEMENT_FN_WRITE      0
#define USB_CFG_IMPLEMENT_FN_READ       0
#define USB_CFG_IMPLEMENT_FN_WRITEOUT   0
#define USB_CFG_HAVE_FLOWCONTROL        0
#define USB_CFG_DRIVER_FLASH_PAGE       0
#define USB_CFG_LONG_TRANSFERS          0
#define USB_COUNT_SOF                   0
#define USB_CFG_CHECK_DATA_TOGGLING     0
#define USB_CFG_HAVE_MEASURE_FRAME_LENGTH   0
#define USB_USE_FAST_CRC                0

#define  USB_CFG_VENDOR_ID       0x81, 0x17 /* Mecanique */
#define  USB_CFG_DEVICE_ID       0x98, 0x0a
#define USB_CFG_DEVICE_VERSION  1, 2
#define USB_CFG_VENDOR_NAME     'D','r','a','c','a','l',' ','T','e','c','h','n','o','l','o','g','i','e','s',' ','i','n','c','.'
#define USB_CFG_VENDOR_NAME_LEN 24
#define USB_CFG_DEVICE_NAME     'U','S','B','T','e','n','k','i'
#define USB_CFG_DEVICE_NAME_LEN 8
#define USB_CFG_DEVICE_CLASS        0xff    /* set to 0 if deferred to interface */
#define USB_CFG_DEVICE_SUBCLASS     0
#define USB_CFG_INTERFACE_CLASS     0   /* define class here if not at device level */
#define USB_CFG_INTERFACE_SUBCLASS  0
#define USB_CFG_INTERFACE_PROTOCOL  0
#define USB_CFG_DESCR_PROPS_DEVICE                  0
#define USB_CFG_DESCR_PROPS_CONFIGURATION           0
#define USB_CFG_DESCR_PROPS_STRINGS                 0
#define USB_CFG_DESCR_PROPS_STRING_0                0
#define USB_CFG_DESCR_PROPS_STRING_VENDOR           0
#define USB_CFG_DESCR_PROPS_STRING_PRODUCT          0
#define USB_CFG_DESCR_PROPS_STRING_SERIAL_NUMBER    (USB_PROP_IS_RAM|USB_PROP_LENGTH(7*2))
#define USB_CFG_DESCR_PROPS_HID                     0
#define USB_CFG_DESCR_PROPS_HID_REPORT              0
#define USB_CFG_DESCR_PROPS_UNKNOWN                 0


#define usbMsgPtr_t unsigned short

#endif /* __usbconfig_h_included__ */
