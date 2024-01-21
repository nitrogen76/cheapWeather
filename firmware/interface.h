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
#ifndef _interface_h__
#define _interface_h__

void usbtenki_delay_ms(int ms);
void usbtenki_usbpoll_delay_ms(int ms);

int sensors_init(void);
int sensors_getNumChannels(void);
int sensors_getChipID(unsigned char id);

/**
 * \brief Called to get a raw value from a sensor.
 * \return Number of bytes returned (max 6)
 **/
int sensors_getRaw(unsigned char id, unsigned char *dst);

/* Optional (weak symbol in main) */
void sensors_doTasks(void);
int sensors_getCalibration(unsigned char id, unsigned char *dst);

#endif // _interface_h__
