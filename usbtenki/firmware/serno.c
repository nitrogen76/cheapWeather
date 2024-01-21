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
#include <string.h>
#include <avr/pgmspace.h>
#include "usbconfig.h"
#include "usbdrv.h"
#include "eeprom.h"
#include "serno.h"

//int usbDescriptorStringSerialNumber[6] = { SERIAL_NUMBER };
int usbDescriptorStringSerialNumber[] = { 
		USB_STRING_DESCRIPTOR_HEADER(6),
		'A','B','C','D','E','F' 
};

void serno_setChar(int idx, unsigned char v)
{
	if (idx<0)
		return;

	if (idx>=sizeof(g_eeprom_data.serial))
		return;

	g_eeprom_data.serial[idx] = v;
}

void serno_store(void)
{
	eeprom_commit();
	int i;
	for (i=0; i<6; i++) {
		usbDescriptorStringSerialNumber[i+1] =
							g_eeprom_data.serial[i];
	}
}

void serno_init()
{
	int i;
	for (i=0; i<6; i++) {
		usbDescriptorStringSerialNumber[i+1] =
							g_eeprom_data.serial[i];
	}
}


