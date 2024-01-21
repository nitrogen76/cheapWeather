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
#include <stdio.h>
#include <string.h>
#include <avr/io.h>
#include <util/delay.h>
#include "sser.h"

#define dly() _delay_us(8);

void sser_init(void)
{
	/* Clock output, normally low */
	SSER_SCK_DDR |= SSER_SCK_BIT;
	SSER_SCK_PORT &= ~SSER_SCK_BIT;

	/* Clock as input, with pullup */
	SSER_DATA_DDR &= ~SSER_DATA_BIT;
	SSER_DATA_PORT |= SSER_DATA_BIT;
}

static void pullData()
{
	/* Disable pullup first in order to pull
	 * when we change the direction to output */
	SSER_DATA_PORT &= ~SSER_DATA_BIT;
	SSER_DATA_DDR |= SSER_DATA_BIT;
}

static void releaseData()
{
	SSER_DATA_DDR &= ~SSER_DATA_BIT;
	SSER_DATA_PORT |= SSER_DATA_BIT;
}

static void clockHigh()
{
	SSER_SCK_PORT |= SSER_SCK_BIT;
}

static void clockLow()
{
	SSER_SCK_PORT &= ~SSER_SCK_BIT;
}

char sser_cmd(unsigned char cmd)
{
	char i;
	char ack;
	unsigned char tout;

	/* Transmission Start */
	clockHigh(); 
	dly();
	pullData(); 
	dly();
	clockLow();
	dly();
	clockHigh();	
	dly();
	releaseData();
	dly();
	clockLow();
	dly();

	/* 3 address bits + 5 command bits */
	for (i=0; i<8; i++)
	{
		if (cmd & 0x80)
			releaseData();
		else
			pullData();
		
		dly();
		clockHigh();
		dly();
		clockLow();

		cmd <<= 1;
	}
	releaseData();

	/* ack */	
	dly();
	clockHigh();
	dly();
	ack = SSER_DATA_PIN & SSER_DATA_BIT;
	clockLow();
	
	if (ack) {
		return -1; // no ack!
	}

	// Continue once the slave releases DATA
	tout = 255;
	while (!(SSER_DATA_PIN & SSER_DATA_BIT)) 
	{ /* empty */	
		dly();
		if (!--tout)
			return -1;
	}

	// clk low
	// data floating

	return 0;
}

/**
 * NOT FINISHED NOR TESTED
 */
char sser_writeByte(unsigned char dat)
{
	char ack, i;
	unsigned char tout;

	dly();
	/* 3 address bits + 5 command bits */
	for (i=0; i<8; i++)
	{
		if (dat & 0x80)
			releaseData();
		else
			pullData();
		
		dly();
		clockHigh();
		dly();
		clockLow();
		dly();

		dat <<= 1;
	}
	releaseData();

	/* ack */	
	dly();
	clockHigh();
	dly();
	ack = SSER_DATA_PIN & SSER_DATA_BIT;
	clockLow();
	
	if (ack) {
		return -1; // no ack!
	}

	// Continue once the slave releases DATA
	tout = 255;
	while (!(SSER_DATA_PIN & SSER_DATA_BIT)) 
	{ /* empty */	
		dly();
		if (!--tout)
			return -1;
	}

	return 0;
}

/** \brief Read and ack a byte from the sensor
 *
 * Note: This should be called after the transmission
 * start, address and commands are sent and after the slave
 * has pulled data low again indicating that the conversion
 * is completed. */
char sser_readByte(unsigned char *dst, unsigned char flags)
{
	unsigned char tmp;
	char i;

	for (tmp=0,i=0; i<8; i++) {
		dly();
		clockHigh();
		dly();
		tmp <<= 1;
		if (SSER_DATA_PIN & SSER_DATA_BIT) {
			tmp |= 1;
		} else {
			// tmp &= ~1;
		}
		clockLow();
	}
	*dst = tmp;
	
	/* Ack the byte by pulling data low during a 9th clock cycle */
	if (!(flags & SSER_READ_FLAGS_SKIP_ACK))
		pullData();

	dly();
	clockHigh();
	dly();
	clockLow();
	releaseData();
	dly();

	return 0;
}

static unsigned char invert_bits(unsigned char input)
{
	char i;
	unsigned char output;

	for (i=0; i<8; i++) {
		output >>= 1;
		output |= input & 0x80;
		input <<= 1;
	}
	
	return output;
}


unsigned char sht_crc(unsigned char crc, unsigned char val)
{
	char i;

	for (i=0; i<8; i++)
	{
		if (!((val^crc)&0x80)) {
			crc = crc << 1;
		} else {
			crc = (crc << 1) ^ 0x31;
		}
		val <<= 1;
	}

	return crc;
}

static char calcCrc(unsigned char cmd, unsigned char data[2])
{
	unsigned char crc = 0; // This depends on the status register

	crc = sht_crc(crc, cmd);
	crc = sht_crc(crc, data[0]);
	crc = sht_crc(crc, data[1]);

	return invert_bits(crc);
}


char sser_getWord(unsigned char cmd, unsigned char dst[2])
{
	unsigned char tmp_data[2];
	unsigned char crc, ccrc;
	unsigned char t_out;
	char res;

	if (sser_cmd(cmd))
		return -1;
	
	/* The slave pulls data low when conversion is done */
	t_out = 100;
	while ((SSER_DATA_PIN & SSER_DATA_BIT)) {
		// 100 * 5 = 600ms. Well above the 320ms (+/- 30%) required for a 14 bit measurement. 
		_delay_ms(6);
		if (!--t_out) {
			return -1;
		}
	}

	res = sser_readByte(&tmp_data[0], SSER_READ_FLAGS_NONE);
	if (res)
		return -1;
	res = sser_readByte(&tmp_data[1], SSER_READ_FLAGS_NONE);
	if (res)
		return -1;
	res = sser_readByte(&crc, SSER_READ_FLAGS_SKIP_ACK);
	if (res)
		return -1;

	ccrc = calcCrc(cmd, tmp_data);

	if (crc != ccrc) {
		//printf("%02x %02x %02x %02x (%02x!!)\r\n", cmd, tmp_data[0], tmp_data[1], crc, ccrc);
		return -1;
	}


	memcpy(dst, tmp_data, 2);

	return 0;
}




