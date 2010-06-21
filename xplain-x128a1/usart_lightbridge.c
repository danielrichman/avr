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

/* Read USART bytes in and light up the 8 LEDs from the raw binary data */
/* Uses the XPLAINBridge in USART bridge mode: 9600 baud */

/* Groovy (?) Examples: */
/* for I in {0..255}; do  echo -en '\x'`printf %02x $I` > /dev/ttyACM0; 
 * sleep 0.1; done */
/* while true; do dd if=/dev/urandom bs=1 count=1 of=/dev/ttyACM0 2> /dev/null; 
 * sleep 1; done */
/* while true; do for i in 11 22 44 88; do echo -en '\x'$i > /dev/ttyACM0; 
 * sleep 0.1; done; done */
/* while true; do for i in 01 02 04 08 00 80 40 20 10 00; do 
 * echo -en '\x'$i > /dev/ttyACM0; sleep 0.1; done; done */

#include <avr/io.h>
#include <avr/interrupt.h>

#define PM_SLEEP_CTRL SLEEP.CTRL /* Hack for avr-libc 1.6.2 */
#include <avr/sleep.h>

ISR(USARTC0_RXC_vect)
{
  PORTE.OUT = ~USARTC0.DATA;
}

int main(void)
{
  PORTE.DIR = 0xFF;
  PORTE.OUT = 0xFF;

  USARTC0.CTRLA = USART_RXCINTLVL_HI_gc;
  USARTC0.CTRLC = USART_CHSIZE_9BIT_gc;
  USARTC0.BAUDCTRLA = 12;
  USARTC0.CTRLB = USART_RXEN_bm;

  PMIC.CTRL = PMIC_HILVLEN_bm;
  sei();

  for (;;) sleep_mode();
}

