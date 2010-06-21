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

/* Produce spam on the USART for oscilloscoping */
/* 9600 baud */

#include <avr/io.h>
#include <avr/interrupt.h>

#define PM_SLEEP_CTRL SLEEP.CTRL /* Hack for avr-libc 1.6.2 */
#include <avr/sleep.h>

ISR(USARTC0_DRE_vect)
{
  PORTE.OUT++;
  USARTC0.DATA = 0x55;
}

int main(void)
{
  PORTE.DIR = 0xFF;
  PORTE.OUT = 0xFF;

  PORTC.DIR = 0x08;

  USARTC0.CTRLA = USART_DREINTLVL_HI_gc;
  USARTC0.CTRLC = USART_CHSIZE_8BIT_gc;
  USARTC0.BAUDCTRLA = 12;
  USARTC0.CTRLB = USART_TXEN_bm;

  PMIC.CTRL = PMIC_HILVLEN_bm;
  sei();

  for (;;) sleep_mode();
}

