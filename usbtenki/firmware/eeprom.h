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
#ifndef _eeprom_h__
#define _eeprom_h__

#define EEPROM_MAGIC_SIZE		7
#define EEPROM_SERIAL_SIZE		6
#define EEPROM_ADC_CHIPS_SIZE	8

struct eeprom_data_struct {
	unsigned char magic[EEPROM_MAGIC_SIZE]; /* 'TenkiCfg' */
	unsigned char serial[EEPROM_SERIAL_SIZE];
	unsigned char adc_chips[EEPROM_ADC_CHIPS_SIZE];
	char use_aref;
	short rtd_corr;
};

extern struct eeprom_data_struct g_eeprom_data;
void eeprom_commit(void);
void eeprom_init(void);

#endif // _eeprom_h__

