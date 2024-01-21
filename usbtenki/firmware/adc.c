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
#include <avr/io.h>
#include <util/delay.h>
#include "adc.h"
#include "eeprom.h"

static void setChannel(int id)
{
	// REFS1  REFS0   
	//   0      0    AREF (internal ref. disabled)
	//   0      1    AVCC with cap on AREF
	//   1      0    Reserved
	//   1      1    Internal 2.56v

	if (!g_eeprom_data.use_aref) {
		ADMUX = (1<<REFS0) | id;
	} else {
		ADMUX = 0 | id;
	}
}

void adc_init(void)
{
	setChannel(0);

	/* Enable ADC and setup prescaler to /128 (gives 93khz) */
	ADCSRA = (1<<ADEN) | 
		(1<<ADPS2) | (1<<ADPS1) | (1<<ADPS0);
}

unsigned short adc_sample(char id, int n_samples, int interval_ms)
{
	unsigned short cur_val = 0;
	unsigned long total = 0;
	int i,j;

	if (id<0 || id > 15)
		return 0xffff;

	/* set MUX3:0  (bits 3:0). No mask needed because of range
	 * check above. */
	setChannel(id);

	for (i=0; i<n_samples; i++) {
		ADCSRA |= (1<<ADSC);	/* start conversion */ 
		while (!(ADCSRA & (1<<ADIF)))
			{ /* do nothing... */ };

		cur_val = ADCL;
		cur_val |= (ADCH << 8);
		total += cur_val << 6; // convert to 16 bit

		for (j=0; j<interval_ms; j++)  {
			_delay_ms(1);
		}
	}

	if (n_samples == 1) {
		return cur_val;
	}
	return (total / n_samples) & 0xffff;
}


