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

#include <string.h>
#include <stdint.h>
#include <avr/io.h>
#include <util/delay.h>

#define LOW  2741
#define HIGH 2439
#define BAUD 50

char *str = "Hello world\n";
uint8_t i;

int main(void)
{
  uint8_t c, m;

  DACA.CTRLA = DAC_CH0EN_bm | DAC_ENABLE_bm;
  DACA.CTRLC = DAC_REFSEL_INT1V_gc;
  DACA.CH0DATA = 0x00;

  for (;;)
  {
    for (i = 0; i < strlen(str); i++)
    {
      c = str[i];

      DACA.CH0DATA = HIGH;
      _delay_ms(1000/BAUD);

      for (m = 0x01; m != 0x00; m <<= 1)
      {
        if (c & m)  DACA.CH0DATA = LOW;
        else        DACA.CH0DATA = HIGH;

        _delay_ms(1000/BAUD);
      }

      DACA.CH0DATA = LOW;
      _delay_ms(1000/BAUD);
      _delay_ms(1000/BAUD);
    }
  }
}
