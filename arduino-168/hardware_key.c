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

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>

static inline void send_char(char c)
{
  loop_until_bit_is_set(UCSR0A, UDRE0);
  UDR0 = c;
}

int main(void)
{
  /* Setup the USART: 9600 baud */
  UBRR0H = 0;
  UBRR0L = 103;
  UCSR0B = (1 << TXEN0);

  /* PD7 Pullup ON */
  PORTD |= (1 << PD7);

  /* Do it */
  for (;;)
  {
    send_char(PIND & (1 << PD7) ? ' ' : '-');
  }
}

