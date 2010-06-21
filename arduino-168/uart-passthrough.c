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

/* Disable the UART on the Arduino's mega168 so it won't interfere while the 
 * FTDI chip is hijacked for breadboarding */

#include <avr/io.h>
#include <avr/sleep.h>

int main(void)
{
  /* Setup IO */
  DDRB  |= _BV(DDB5);     /* Put PB5 as an output (pin13) */
  PORTB |= _BV(PB5);      /* Turn it on */

  /* Disable UART so it won't interfere while the 
   * FTDI chip is hijacked for breadboarding */
  UCSR0B = 0;

  /* Sleep */
  for (;;) sleep_mode();
}

