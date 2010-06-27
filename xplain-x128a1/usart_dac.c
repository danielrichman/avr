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

/* Uses the XPLAINBridge in USART bridge mode: 9600 baud */

#include <stdlib.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#define PM_SLEEP_CTRL SLEEP.CTRL /* Hack for avr-libc 1.6.2 */
#include <avr/sleep.h>

uint8_t status;
uint8_t data;

ISR(USARTC0_RXC_vect)
{
  if (status == 0)
  {
    data = USARTC0.DATA;
    status++;
  }
  else
  {
    DACA.CH0DATA = (data << 8) | USARTC0.DATA;
    status = 0;
  }
}

int main(void)
{
  uint16_t data;

  DACA.CTRLA = DAC_CH0EN_bm | DAC_ENABLE_bm;
  DACA.CTRLC = DAC_REFSEL_AVCC_gc;
  DACA.CH0DATA = 0x00;

  USARTC0.CTRLA = USART_RXCINTLVL_HI_gc;
  USARTC0.CTRLC = USART_CHSIZE_8BIT_gc;
  USARTC0.BAUDCTRLA = 12;
  USARTC0.CTRLB = USART_RXEN_bm;

  PMIC.CTRL = PMIC_HILVLEN_bm;
  sei();

  for (;;) sleep_mode();
}
