/*   USBTenki - Interfacing sensors to USB 
 *   Copyright (C) 2007-2012  Raphaël Assénat <raph@raphnet.net>
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
#include <string.h>
#include <util/delay.h>
#include "interface.h"
#include "usbtenki_cmds.h"
#include "i2c.h"
#include "mcp3423.h"
#include "eeprom.h"

static unsigned char mcp_addr = MCP3423_BASE_ADDR;

int sensors_init(void)
{
	unsigned char cfg_reg = MCP3423_CFGREG_18BITS | MCP3423_CHANNEL(0) | MCP3423_GAIN1X;

	i2c_init(I2C_FLAG_INTERNAL_PULLUP, 50);
	return i2c_transaction(mcp_addr, 1, &cfg_reg, 0, NULL);
}

int sensors_getNumChannels(void)
{
	return 1;
}

int sensors_getChipID(unsigned char id)
{
	return USBTENKI_CHIP_PT100_RTD;
}

int sensors_getRaw(unsigned char id, unsigned char *dst)
{
	// Assuming a maximum RTD resistance of 400ohm ( > 850 C ) and
	// worst case wire resistance of 128 ohm. At 428ohm, we have 0.428 volt.
	// The maximum common mode value is 1.024. We can therefore afford a 2x gain. (.856mV)
	mcp3423_readChannel(mcp_addr, 0, MCP3423_GAIN2X , dst);
	
	_delay_ms(1);

	// At I = 1mA, this channels saturates with a 128 ohm wire.
	mcp3423_readChannel(mcp_addr, 1, MCP3423_GAIN8X , dst + 3);

	return 6;
}

// overwire weak funtion in main.c
int sensors_getCalibration(unsigned char id, unsigned char *dst)
{
	dst[0] = g_eeprom_data.rtd_corr;
	dst[1] = g_eeprom_data.rtd_corr >> 8;

	return 2;
}

