/*   USBTenki - Interfacing sensors to USB 
 *   Copyright (C) 2007-2013  Raphaël Assénat <raph@raphnet.net>
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
#include <avr/io.h>
#include <util/delay.h>
#include "interface.h"
#include "usbtenki_cmds.h"
#include "i2c.h"
#include "mlx90614.h"

int sensors_init(void)
{
	// Force SCL low for a moment to disable PWM mode and enable SMBus mode.
	PORTC &= ~(1<<PORTC5);
	DDRC |= 1<<PORTC5;
	_delay_ms(2);
	DDRC &= ~(1<<PORTC5);  // let go
	
	i2c_init(I2C_FLAG_INTERNAL_PULLUP, 255);
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
		case 0: return USBTENKI_CHIP_MLX90614_TA;
		case 1: return USBTENKI_CHIP_MLX90614_TOBJ;
	}
	return -1;
}

int sensors_getRaw(unsigned char id, unsigned char *dst)
{
	int res;

	switch (id)
	{
		case 0:
			res = mlx90614_readRam(0, MLX90614_RAM_TA, dst, 3);
			if (res)
				return res;
			break;

		case 1:
			res = mlx90614_readRam(0, MLX90614_RAM_TOBJ1, dst, 3);
			if (res)
				return res;
			break;

		default:
			return -1;
	}

	return 2;
}


