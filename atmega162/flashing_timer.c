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

/* 1Hz flashing light test using a timer */

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>

ISR (TIMER1_COMPA_vect)
{
  PORTC ^= _BV(PC0);      /* Toggle the light */
}

int main(void)
{
       /* Setup IO */
  DDRC   = _BV(DDC0);     /* Put PC0 as an output (pin21) */
  PORTC  = _BV(PC0);      /* Turn it on */

       /* Setup timer1 frequency as 1Hz */
  /* Prescaler to FCPU/1024; Clear timer on compare match *
   * Timer freq is 16000000/1024 = 15625Hz
   * We want 1Hz, so we want an interrupt every 15625 timer1 ticks. */
  TCCR1B  = (_BV(CS12) | _BV(CS10) | _BV(WGM12));
  OCR1A   = 15625;

       /* Enable Timer1 */
  TCNT1   = 0;            /* Reset timer */
  TIMSK   = _BV(OCIE1A);  /* Enable Compare Match Interrupts */
  sei();                  /* Turn on interrupts */

       /* Sleep */
  for (;;) sleep_mode();
}

