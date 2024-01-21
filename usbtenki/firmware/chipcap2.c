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
#include <string.h>
#include "i2c.h"
#include "chipcap2.h"

char cc2_send_measurement_rq(char i2c_addr)
{
	return i2c_transaction(i2c_addr, 1, NULL, 0, NULL);
}

char cc2_fetch_data(char i2c_addr, char len, unsigned char *dst)
{
	return i2c_transaction(i2c_addr, 0, NULL, len, dst);
}

