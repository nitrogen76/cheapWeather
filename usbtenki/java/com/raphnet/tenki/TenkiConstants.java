package com.raphnet.tenki;

/**
 * Various constants used in package.
 */
public class TenkiConstants
{
	/* Units from usbtenki.h */

	/** Raw units, chip specific. */
	public static final int UNIT_RAW 	= 		0x00;

	/** Celcius temperature units. */
	public static final int UNIT_CELCIUS	=	0x01;
	/** Fahrenheit temperature units. */
	public static final int UNIT_FAHRENHEIT	= 	0x02;
	/** Kelvin temperature units. */
	public static final int UNIT_KELVIN		=	0x03;
	/** Relative humidity give in percents. */
	public static final int UNIT_RH			=	0x04; // relative humidity

	/** kilopascal pressure unit. */
	public static final int UNIT_KPA 		= 	0x10;
	/** hectopascal pressure unit. */
	public static final int UNIT_HPA 		= 	0x11;
	/** bar pressure unit. */
	public static final int UNIT_BAR		=	0x12;
	/** Technical Atmosphere (AT) pressure unit. 1 Pa = 1.0197 * 10 ^ -5 */
	public static final int UNIT_AT			=	0x13;
	/** Atmosphere pressure unit. 1 Pa = 9.8692 * 10 ^ -6 */
	public static final int UNIT_ATM		=	0x14;
	/** Torr pressure unit. */
	public static final int UNIT_TORR		=	0x15;
	/** PSI pressure unit. */
	public static final int UNIT_PSI		=	0x16;
	
	/** Volts potential difference unit. */
	public static final int UNIT_VOLTS		=	0x20;

	/** Lux illuminance unit. */
	public static final int UNIT_LUX		=	0x30;

	/**
	 * Get the name of an unit.
	 *
	 * @param unit The unit
	 * @return The name of the unit.
	 */
	public static String unitToString(int unit)
	{
		String libname = JNI.unitToName(unit);

		if (libname == null) {
			return "Unknown unit";
		}

		return libname;
	}

	/** MCP9800 family temperature sensor. */
	public static final int CHIP_MCP9800	=	0x00;
	/** LM75 and compatibles temperature sensor. */
	public static final int CHIP_LM75		=	0x01;
	/** LM92 temperature sensor. */
	public static final int CHIP_LM92		=	0x02;
	/** Sensirion dual sensor, temperature channel. */
	public static final int CHIP_SHT_TEMP	=	0x03;
	/** Sensirion dual sensor, relative humidity channel. */
	public static final int CHIP_SHT_RH		=	0x04;
	/** TSL2561 dual sensor, IR + Visible channel. */
	public static final int CHIP_TSL2561_IR_VISIBLE	=	0x05;
	/** TSL2561 dual sensor, IR only channel. */
	public static final int CHIP_TSL2561_IR	=	0x06;
	/** BS02 dual sensor, temperature channel. */
	public static final int CHIP_BS02_TEMP	=	0x07;
	/** BS02 dual sensor, relative humidity channel. */
	public static final int CHIP_BS02_RH	=	0x08;

	public static final int CHIP_MCU_ADC0	=	0x80;
	public static final int CHIP_MCU_ADC1	=	0x81;
	public static final int CHIP_MCU_ADC2	=	0x82;
	public static final int CHIP_MCU_ADC3	=	0x83;
	public static final int CHIP_MCU_ADC4	=	0x84;
	public static final int CHIP_MCU_ADC5	=	0x85;

	/** MPX4115 analogic pressure sensor. */
	public static final int CHIP_MPX4115	=	0x90;

	public static final int CHIP_VOLTS		=	0x91;
	public static final int CHIP_VOLTS_REVERSE	=	0x92;

	/** Unused chip (usually an unconfigured ADC channel). */
	public static final int CHIP_NONE		=	0xff;

	/** 
	 * Get the name of a chip.
	 *
	 * @param chip Chip id
	 * @return The name of the chip.
	 * */
	public static String chipToString(int chip)
	{
		String libname = JNI.chipToName(chip);

		if (libname == null) {
			return "Unknown unit";
		}

		return libname;
	}


}
