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
#ifndef _tsl2561_h__
#define _tsl2561_h__

/* I2C address when address select pin is connected to GND */
#define TSL2561_ADDRESS_GND		0x29

/* I2C address when address select pin is left floating */
#define TSL2561_ADDRESS_FLOAT	0x39

/* I2C address when address select pin is connected to VDD */
#define TSL2561_ADDRESS_VDD		0x49


#define TSL2561_CMD_CMD			0x80
#define TSL2561_CMD_CLEAR		0x40
#define TSL2561_CMD_WORD		0x20
#define TSL2561_CMD_BLOCK		0x10

// other CMD bits are the register address:
#define TSL2561_REG_CONTROL		0x00
#define TSL2561_REG_TIMING		0x01
#define TSL2561_REG_THRESLOW_L	0x02
#define TSL2561_REG_THRESLOW_H	0x03
#define TSL2561_REG_THRESHIGH_L	0x04
#define TSL2561_REG_THRESHIGH_H	0x05
#define TSL2561_REG_INTERRUPT	0x06
#define TSL2561_REG_CRC			0x08
#define TSL2561_REG_ID			0x0a
#define TSL2561_REG_DATA0_L		0x0c
#define TSL2561_REG_DATA0_H		0x0d
#define TSL2561_REG_DATA1_L		0x0e
#define TSL2561_REG_DATA1_H		0x0f

int tsl2561_readRegister(char i2c_addr, char reg_addr, unsigned char *dst, char len);
int tsl2561_writeRegister(char i2c_addr, char reg_addr, unsigned char *dat, char len);

#endif // _tsl2561_h__

