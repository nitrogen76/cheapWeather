/*   USBTenki - Interfacing sensors to USB 
 *   Copyright (C) 2007-2011  Raphaël Assénat <raph@raphnet.net>
 *
 *   This program is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, either version 3 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "interface.h"
#include "usbtenki_cmds.h"
#include "sser.h"
#include <util/delay.h>

int sensors_init(void)
{
	// SHT7x datasheet specifies 11ms. With 50ms, we are on the safe side.
	_delay_ms(50);

	sser_init();

	// Explicitely set the status register with default values.
	sser_cmd(SHT_CMD_WRITE_STATUS);
	sser_writeByte(0x00);

	return 0;	
}

int sensors_getNumChannels(void)
{
	return 2;
}

int sensors_getChipID(unsigned char id)
{
	switch (id)
	{
		case 0: return USBTENKI_CHIP_SHT_TEMP;
		case 1: return USBTENKI_CHIP_SHT_RH;
	}
	return -1;
}

int sensors_getRaw(unsigned char id, unsigned char *dst)
{
	int res;

	switch (id)
	{
		case 0:
			res = sser_getWord(SHT_CMD_MEASURE_TEMPERATURE, dst);
			if (res)
				return res;
			break;

		case 1:
			res = sser_getWord(SHT_CMD_MEASURE_HUMIDITY, dst);
			if (res)
				return res;			
			break;

		default:
			return -1;
	}
	return 2;
}


