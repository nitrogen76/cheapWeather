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



#define SSER_SCK_BIT	(1<<3)
#define SSER_SCK_PORT	PORTC
#define SSER_SCK_DDR	DDRC

#define SSER_DATA_BIT	(1<<2)
#define SSER_DATA_PORT	PORTC
#define SSER_DATA_DDR	DDRC
#define SSER_DATA_PIN	PINC

#include "sser.c"
