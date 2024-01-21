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
#ifndef _mcp_9800_h__
#define _mcp_9800_h__

#define MCP9800_REG_TEMP		0x00
#define MCP9800_REG_CFG			0x01
#define MCP9800_REG_TEMP_HYST	0x02
#define MCP9800_REG_TEMP_LIMIT	0x03

#define MCP9800_ADDR_BASE		0x48	/* + 3 user bits */

#define MCP9800_CFG_ONESHOT		0x80
#define MCP9800_CFG_9BITS		0x00
#define MCP9800_CFG_10BITS		0x20
#define MCP9800_CFG_11BITS		0x40
#define MCP9800_CFG_12BITS		0x60
#define MCP9800_CFG_FAULTQ_1	0x00
#define MCP9800_CFG_FAULTQ_2	0x08
#define MCP9800_CFG_FAULTQ_4	0x10
#define MCP9800_CFG_FAULTQ_6	0x18
#define MCP9800_ALERT_POL_HIGH	0x04
#define MCP9800_INT_MODE		0x02
#define MCP9800_SHUTDOWN		0x01

int mcp9800_writeRegister(char i2c_addr, char reg_addr, unsigned char *dat, char len);
int mcp9800_readRegister(char i2c_addr, char reg_addr, unsigned char *dst, char len);
int mcp9800_configure(char i2c_addr, unsigned char cfg);

#endif // _mcp_9800_h__

