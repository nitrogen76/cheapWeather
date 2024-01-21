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
#include <avr/eeprom.h>
#include <string.h>
#include "eeprom.h"
#include "../common/usbtenki_cmds.h"

struct eeprom_data_struct g_eeprom_data;

void eeprom_commit(void)
{
	eeprom_write_block(&g_eeprom_data, (void*)0x00, sizeof(struct eeprom_data_struct));
}

void eeprom_init(void)
{
	char *magic = "TenkiCf3";
	eeprom_read_block(&g_eeprom_data, (void*)0x00, sizeof(struct eeprom_data_struct));

	/* Check for magic number */
	if (memcmp(g_eeprom_data.magic, magic, EEPROM_MAGIC_SIZE)) {
		memcpy(g_eeprom_data.magic, magic, EEPROM_MAGIC_SIZE);
		memset(g_eeprom_data.serial, '?', EEPROM_SERIAL_SIZE);
		memset(g_eeprom_data.adc_chips, USBTENKI_CHIP_NONE, 
										EEPROM_ADC_CHIPS_SIZE);
		g_eeprom_data.use_aref = 0;
		g_eeprom_data.rtd_corr = 0;

		eeprom_commit();
	}
}

