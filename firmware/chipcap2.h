/*   USBTenki - Interfacing sensors to USB
 *   Copyright (C) 2007-2014  Raphaël Assénat <raph@raphnet.net>
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
#ifndef _chipcap2_h__
#define _chipcap2_h__

#define CC2_DEFAULT_I2C_ADDRESS	0x28

/* R/W EEPROM. Where 0x16 <= a <= 0x1F */
#define CC2_CMD_READ_EEPROM(a)		(0x16 + ((a)-0x16))
#define CC2_CMD_WRITE_EEPROM(a)		(0x56 + ((a)-0x16))

#define CC2_CMD_START_NOM	0x80	/* Start normal mode */
#define CC2_CMD_START_CM	0xA0	/* Start command mode */

char cc2_send_measurement_rq(char i2c_addr);

/* RH[2], T[2]
 */
char cc2_fetch_data(char i2c_addr, char len, unsigned char *dst);

#endif // _chipcap2_h__
