/*
    Copyright (C) 2008  Daniel Richman

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    For a full copy of the GNU General Public License, 
    see <http://www.gnu.org/licenses/>.
*/

/* 1Hz flashing light test using util/delay */

#include <avr/io.h>
#include <util/delay.h>

int main(void)
{
       /* Setup IO */
  PORTE.DIR   = 0x1;     /* Put PC0 as an output (pin21) */
  PORTE.OUT   = 0x1;     /* Turn it on */

       /* Flash */
  for (;;)
  {
    _delay_ms(1000);
    PORTE.OUT ^= 0x1;    /* Toggle the light */
  }
}

