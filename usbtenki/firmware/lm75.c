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

#include <string.h>
#include "i2c.h"
#include "lm75.h"

int lm75_writeRegister(char i2c_addr, char reg_addr, unsigned char *dat, char len)
{
	unsigned char tmp[len+1];
	tmp[0] = reg_addr & 0x03;
	memcpy(&tmp[1], dat, len);
	return i2c_transaction(i2c_addr, len + 1, tmp, 0, NULL);
}

int lm75_readRegister(char i2c_addr, char reg_addr, unsigned char *dst, char len)
{
	unsigned char tmp;
	tmp = reg_addr & 0x03;
	return i2c_transaction(i2c_addr, 1, &tmp, len, dst);
}

int lm75_configure(char i2c_addr, unsigned char cfg)
{
	return lm75_writeRegister(i2c_addr, LM75_REG_CFG, &cfg, 1);
}

