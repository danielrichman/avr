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

/* The 8 buttons toggle 8 bits of DAC (DC) */

#include <avr/io.h>
#include <util/delay.h>

#define debounce_wait() _delay_ms(10)

int main(void)
{
  uint8_t data;
  uint8_t input;
 
  PORTE.DIR      = 0xFF;
  PORTE.OUT = 0xFF;

  PORTF.PIN0CTRL = PORT_OPC0_bm | PORT_OPC1_bm;
  PORTF.PIN1CTRL = PORT_OPC0_bm | PORT_OPC1_bm;
  PORTF.PIN2CTRL = PORT_OPC0_bm | PORT_OPC1_bm;
  PORTF.PIN3CTRL = PORT_OPC0_bm | PORT_OPC1_bm;
  PORTF.PIN4CTRL = PORT_OPC0_bm | PORT_OPC1_bm;
  PORTF.PIN5CTRL = PORT_OPC0_bm | PORT_OPC1_bm;
  PORTF.PIN6CTRL = PORT_OPC0_bm | PORT_OPC1_bm;
  PORTF.PIN7CTRL = PORT_OPC0_bm | PORT_OPC1_bm;

  DACA.CTRLA = DAC_CH0EN_bm | DAC_ENABLE_bm;
  DACA.CTRLC = DAC_REFSEL_AVCC_gc;
  DACA.CH0DATA = 0x00;

  data = 0x00;

  /* Pull up wait */
  while (PORTF.IN != 0xFF);
  debounce_wait();

  for (;;)
  {
    /* Wait until a button is pressed */
    while (PORTF.IN == 0xFF);

    input = PORTF.IN;

    /* Wait a bit and check it wasn't noise or bouncing */
    debounce_wait();
    if (PORTF.IN != input) continue;

    /* Wait until they let go */
    while (PORTF.IN != 0xFF);

    /* Toggle the bits for which buttons were pressed */
    data ^= ~input;

    /* Update outputs */
    PORTE.OUT = ~data;
    DACA.CH0DATA = data << 4;

    /* Wait for the dac and then delay a bit */
    while (!(DACA.STATUS & DAC_CH0DRE_bm));
    debounce_wait();
  }
}

