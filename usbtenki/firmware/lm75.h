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
#ifndef _lm75_h__
#define _lm75_h__

#define LM75_REG_TEMP		0x00
#define LM75_REG_CFG		0x01
#define LM75_REG_TEMP_HYST	0x02
#define LM75_REG_TEMP_LIMIT	0x03

#define LM75_ADDR_BASE		0x48	/* +3 user bits */

#define LM75_CFG_SHUTDOWN	0x01
#define LM75_CFG_CMP_INT	0x02
#define LM75_CFG_OS_POL		0x04
#define LM75_CFG_FAULTQ_1	0x00
#define LM75_CFG_FAULTQ_2	0x08
#define LM75_CFG_FAULTQ_4	0x10
#define LM75_CFG_FAULTQ_6	0x18


int lm75_writeRegister(char i2c_addr, char reg_addr, unsigned char *dat, char len);
int lm75_readRegister(char i2c_addr, char reg_addr, unsigned char *dst, char len);
int lm75_configure(char i2c_addr, unsigned char cfg);

#endif // _lm75_h__

