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
#include <util/delay.h>
#include <string.h>
#include "interface.h"
#include "usbtenki_cmds.h"
#include "i2c.h"
#include "chipcap2.h"

int sensors_init(void)
{
	// The divisor of 255 yiels a clock of approx 22kHz.
	// According to the application guide (Table 8),
	// 20 to 400 kHz is admissible.
	i2c_init(I2C_FLAG_INTERNAL_PULLUP, 255);

	return 0;
}

int sensors_getNumChannels(void)
{
	return 2;
}

int sensors_getChipID(unsigned char id)
{
	switch (id)
	{
		case 0: return USBTENKI_CHIP_CC2_RH;
		case 1: return USBTENKI_CHIP_CC2_T;
	}
	return USBTENKI_CHIP_NONE;
}

#define RETRIES	3
static unsigned char raw_data[4];
static char status_error = 1;

void sensors_doTasks(void)
{
	char res, retry;

	res = cc2_send_measurement_rq(CC2_DEFAULT_I2C_ADDRESS);

	if (res)
		goto err;

	_delay_us(200);

	res = cc2_fetch_data(CC2_DEFAULT_I2C_ADDRESS, 4, raw_data);
	if (res) {
		goto err;
	}

	usbtenki_usbpoll_delay_ms(500);
	status_error = 0;
	return;

err:
	status_error = 1;
}

int sensors_getRaw(unsigned char id, unsigned char *dst)
{
	if (status_error)
		return 0;

	if (id > 1)
		return 0;

	memcpy(dst, &raw_data[id*2], 2);
	return 2;
}
