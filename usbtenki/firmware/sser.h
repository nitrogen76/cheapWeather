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
#ifndef _sensirion_serial_h__
#define _sensirion_serial_h__

#ifndef SSER_SCK_BIT

#define SSER_SCK_BIT	(1<<5)
#define SSER_SCK_PORT	PORTC
#define SSER_SCK_DDR	DDRC

#endif

#ifndef SSER_DATA_BIT

#define SSER_DATA_BIT	(1<<4)
#define SSER_DATA_PORT	PORTC
#define SSER_DATA_DDR	DDRC
#define SSER_DATA_PIN	PINC

#endif

#define SHT_CMD_MEASURE_TEMPERATURE	0x03
#define SHT_CMD_MEASURE_HUMIDITY	0x05
#define SHT_CMD_READ_STATUS			0x07
#define SHT_CMD_WRITE_STATUS		0x06
#define SHT_CMD_SOFT_RESET			0x36


void sser_init(void);
char sser_cmd(unsigned char cmd);

#define SSER_READ_FLAGS_NONE		0
#define SSER_READ_FLAGS_SKIP_ACK	1 /* Used to end transfer */
char sser_readByte(unsigned char *dst, unsigned char flags);
char sser_getWord(unsigned char cmd, unsigned char dst[2]);
char sser_writeByte(unsigned char dat);


#endif // _sensirion_serial_h__

