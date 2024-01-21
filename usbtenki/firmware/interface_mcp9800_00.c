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
#include "i2c.h"
#include "mcp9800.h"

#define MCP9800_ADDR    (MCP9800_ADDR_BASE + 0)

int sensors_init(void)
{
	i2c_init(I2C_FLAG_INTERNAL_PULLUP, 255);

	return mcp9800_configure(MCP9800_ADDR, MCP9800_CFG_12BITS);
}

int sensors_getNumChannels(void)
{
	return 1;
}

int sensors_getChipID(unsigned char id)
{
	return USBTENKI_CHIP_MCP9800;
}

int sensors_getRaw(unsigned char id, unsigned char *dst)
{
	int res;
	res = mcp9800_readRegister(MCP9800_ADDR, MCP9800_REG_TEMP, dst, 2);
	if (res)
		return res;
	return 2;
}


