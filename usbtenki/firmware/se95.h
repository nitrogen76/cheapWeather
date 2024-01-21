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
#ifndef _se95_h__
#define _se95_h__

// MSB LSB
// 1  0  0  1  A2  A1  A0
#define SE95_ADDR_BASE	0x48

#define SE95_REG_TEMP	0x00
#define SE95_REG_CONF	0x01
#define SE95_REG_THYST	0x02
#define SE95_REG_TOS	0x03
#define SE95_REG_ID		0x05

#define SE95_CFG_SHDN			0x00
#define SE95_CFG_OS_COMP_INIT	0x01
#define SE95_CFG_OS_POL			0x02
#define SE95_CFG_OS_F_QUE	 	0x03 // 2bit
#define SE95_CFG_RATEVAL		0x05 // 2 bits

#endif // _se95_h__

