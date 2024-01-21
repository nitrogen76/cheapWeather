#ifndef _usbtenki_units_h__
#define _usbtenki_units_h__


#define TENKI_UNIT_RAW			0x00

#define TENKI_UNIT_CELCIUS		0x01
#define TENKI_UNIT_FAHRENHEIT	0x02
#define TENKI_UNIT_KELVIN		0x03
#define TENKI_UNIT_RH			0x04

#define TENKI_UNIT_IS_PRESSURE(p)	(((p)>=0x10) && ((p)<=0x16))
#define TENKI_UNIT_KPA			0x10
#define TENKI_UNIT_HPA			0x11
#define TENKI_UNIT_BAR			0x12
#define TENKI_UNIT_AT			0x13
#define TENKI_UNIT_ATM			0x14
#define TENKI_UNIT_TORR			0x15
#define TENKI_UNIT_PSI			0x16
#define TENKI_UNIT_INHG			0x17 // 3386.389 pascals

#define TENKI_UNIT_PERCENT		0x19
#define TENKI_UNIT_PPM			0x1A
#define TENKI_UNIT_PPB			0x1B

#define TENKI_UNIT_uG_PER_M3	0x1C		// micro grams per cubic meter
#define TENKI_UNIT_COUNT_PER_CM3	0x1D	// count per cubic centimeter

#define TENKI_UNIT_VOLTS		0x20
#define TENKI_UNIT_MILLIVOLT	0x23
#define TENKI_UNIT_AMPS			0x25
#define TENKI_UNIT_MILLIAMPS	0x28

#define TENKI_UNIT_KILOWATTS	0x29
#define TENKI_UNIT_WATTS		0x2A
#define TENKI_UNIT_MILLIWATTS	0x2D

#define TENKI_UNIT_LUX			0x30
#define TENKI_UNIT_uW_PER_CM2	0x31	// micro watt per square centimeter

#define TENKI_UNIT_METER_SEC	0x40

#define TENKI_UNIT_MILLIHZ		0x50
#define TENKI_UNIT_HZ			0x51
#define TENKI_UNIT_KHZ			0x52
#define TENKI_UNIT_MHZ			0x53

#define TENKI_UNIT_RPM			0x55

#define TENKI_UNIT_METERS		0x60
#define TENKI_UNIT_DECIMETERS	0x61
#define TENKI_UNIT_CENTIMETERS	0x62
#define TENKI_UNIT_MILLIMETERS	0x63
#define TENKI_UNIT_MICROMETERS	0x64

#define TENKI_UNIT_MILS			0x69
#define TENKI_UNIT_INCHES		0x6A
#define TENKI_UNIT_FEET			0x6B
#define TENKI_UNIT_YARDS		0x6C

#define TENKI_UNIT_ARBITRARY	0x70

#define TENKI_UNIT_HEXCOLOR		0x90


// used with USBTenki_unitPreferences to indicate
// that the units used natively by the sensor
// shall be used. (eg: VOC in ppb, CO2 in ppm)
#define TENKI_UNIT_SENSOR_DEFAULT	0xFF

#endif

