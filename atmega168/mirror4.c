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

/* Mirror inputs to outputs */

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>

#define PORTD_START  0
#define PORTD_END    3

ISR (TIMER1_COMPA_vect)
{
  PORTD = ~PINC;
}

int main(void)
{
  DDRD   = 0xFF;
  PORTC  = 0xFF;
  PORTD  = (1 << PORTD_START);

  TCCR1B  = ((1 << CS11) | (1 << CS10) | (1 << WGM12));
  OCR1A   = 31250;

  TCNT1   = 0;
  TIMSK1  = (1 << OCIE1A);
  sei();

  for (;;) sleep_mode();
}

