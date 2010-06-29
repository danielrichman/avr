/*
    Copyright (C) 2010  Daniel Richman

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

#define LOW  2000
#define HIGH 2700
#define DELAY 104 /* 300 baud */

char *str = "Hello world\n";
uint8_t i;

void delay()
{
  /* TODO: Use interrupts */
  while (!(TCD0.INTFLAGS & TC0_OVFIF_bm));
  TCD0.INTFLAGS = TC0_OVFIF_bm;
}

int main(void)
{
  uint8_t c, m, i;

  DACA.CTRLA = DAC_CH0EN_bm | DAC_ENABLE_bm;
  DACA.CTRLC = DAC_REFSEL_INT1V_gc;
  DACA.CH0DATA = HIGH;

  TCD0.PER = DELAY;
  TCD0.CTRLA = TC_CLKSEL_DIV64_gc;

  delay();

  for (;;)
  {
    for (i = 0; i < strlen(str); i++)
    {
      c = str[i];

      DACA.CH0DATA = LOW;
      delay();

      for (m = 0x01; m != 0x00; m <<= 1)
      {
        if (c & m)  DACA.CH0DATA = HIGH;
        else        DACA.CH0DATA = LOW;

        delay();
      }

      DACA.CH0DATA = HIGH;
      delay();
      delay();
    }
  }
}
