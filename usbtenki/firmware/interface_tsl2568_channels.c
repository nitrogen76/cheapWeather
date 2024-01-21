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
#include "interface.h"
#include "usbtenki_cmds.h"


int sensors_getNumChannels(void)
{
	return 4;
}

int sensors_getChipID(unsigned char id)
{
	switch (id)
	{
		case 0:
			return USBTENKI_CHIP_TSL2568_IR_VISIBLE;
		case 1:
			return USBTENKI_CHIP_TSL2568_IR;
		case 2:
			return USBTENKI_CHIP_TSL2568_IR_VISIBLE_16X;
		case 3:
			return USBTENKI_CHIP_TSL2568_IR_16X;
	}
	return USBTENKI_CHIP_NONE;
}

