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
#ifndef _adt7410_h__
#define _adt7410_h__

// MSB ...  LSB
// 1 0 0 1 0 A1 A0  (4 addresses)
#define ADT7410_ADDR_BASE	0x48

#define ADT7410_REG_TEMP_MSB		0x00
#define ADT7410_REG_TEMP_LSB		0x01
#define ADT7410_REG_STATUS			0x02
#define ADT7410_REG_CONFIGURATION	0x03

#define ADT7410_CFG_RESOLUTION_13BIT	0x80
#define ADT7410_CFG_RESOLUTION_16BIT	0 

#define ADT7410_CFG_OP_SHUTDOWN		(0x3 << 5)
#define ADT7410_CFG_OP_1SPS			(0x2 << 5)
#define ADT7410_CFG_OP_ONESHOT		(0x1 << 5)
#define ADT7410_CFG_OP_CONTINUOUS	(0x0 << 5)


#endif // _adt7410_h__

