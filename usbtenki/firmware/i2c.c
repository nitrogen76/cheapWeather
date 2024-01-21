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
#include <avr/io.h>
#include <util/twi.h>

#include "i2c.h"

void i2c_init(int use_int_pullup, unsigned char twbr)
{
	if (use_int_pullup) {
		/* Use internal pullups */
		PORTC |= (1<<5)|(1<<4);
	} else {
		/* Extenal pullups required */
		PORTC &= ~((1<<5)|(1<<4));
	}

	/* This gives roughly 30 khz with a 16mhz xtal */
	TWBR = twbr;
	TWSR &= ~((1<<TWPS1)|(1<<TWPS0));
}

// \brief Perform an I2C transaction
//
// \param addr I2C address (right-aligned)
// \param wr_len Write length
// \param wr_data Data to write
// \param rd_len Length to read
// \param rd_data Destination for read data
//
// Special case: If wr_data is NULL but wr_len is non-zero, no data is written. The
// slave is only addressed.
//
// \return 0 On success
int i2c_transaction(unsigned char addr, int wr_len, unsigned char *wr_data,
								int rd_len, unsigned char *rd_data)
{
	int ret =0;
	unsigned char twsr;

	if (wr_len==0 && rd_len==0)
		return -1;

	if (wr_len != 0)
	{
		// Send a start condition
		TWCR = (1<<TWINT)|(1<<TWSTA)|(1<<TWEN);

		while (!(TWCR & (1<<TWINT)))
			{ /* do nothing */ }
		if ((TWSR & 0xF8) != TW_START)
			return 1;	/* Failed */

		TWDR = (addr<<1) | 0;	/* Address + write(0) */
		TWCR = (1<<TWINT)|(1<<TWEN);

		while (!(TWCR & (1<<TWINT)))
			{ /* do nothing */ }

		/* TWSR can be:
		 * TW_MT_SLA_ACK, TW_MT_SLA_NACK or TW_MR_ARB_LOST */
		if ((TWSR & 0xF8) != TW_MT_SLA_ACK) {
			ret = 2;
			goto err;
		}

		if (wr_data) {
			while (wr_len--)
			{
				TWDR = *wr_data;
				TWCR = (1<<TWINT)|(1<<TWEN);

				while (!(TWCR & (1<<TWINT)))
					{ /* do nothing */ }

				wr_data++;
			}
		}
	} // if (wr_len != 0)

	if (rd_len != 0)
	{
		/* Do a (repeated) start condition */
		TWCR = (1<<TWINT)|(1<<TWSTA)|(1<<TWEN);
		while (!(TWCR & (1<<TWINT)))
			{ /* do nothing */ }
		twsr = TWSR;

		if ((twsr != TW_REP_START) && (twsr != TW_START) ) {
			ret = 3;
			goto err;
		}

		TWDR = (addr<<1) | 1;	/* Address + read(1) */
		TWCR = (1<<TWINT)|(1<<TWEN);

		while (!(TWCR & (1<<TWINT)))
			{ /* do nothing */ }

		/* TWSR can be:
		 * TW_MR_SLA_ACK, TW_MR_SLA_NACK or TW_MR_ARB_LOST */
		if (TWSR != TW_MR_SLA_ACK) {
			ret = 4;
			goto err;
		}

		while (rd_len--)
		{

			if (rd_len)
				TWCR = (1<<TWINT)|(1<<TWEN)|(1<<TWEA);
			else
				TWCR = (1<<TWINT)|(1<<TWEN);

			while (!(TWCR & (1<<TWINT)))
					{ /* do nothing */ }

			*rd_data = TWDR;
			rd_data++;
		}
	} // if (rd_len != 0)


	TWCR = (1<<TWINT)|(1<<TWSTO)|(1<<TWEN);

	return 0;

err:

	TWCR = (1<<TWINT)|(1<<TWSTO)|(1<<TWEN);

	return ret;
}
