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
#include <stdlib.h>
#include <stdint.h>

extern div_t udiv(int __num, int __denom) __asm__("__udivmodhi4")
                                                               __ATTR_CONST__;

static inline void send_char(char c)
{
  loop_until_bit_is_set(UCSR0A, UDRE0);
  UDR0 = c;
}

uint16_t powten[5] = { 10000, 1000, 100, 10, 1 };

static inline void send_ui16(uint16_t n)
{
  div_t divbuf;
  uint8_t i;

  for (i = 0; i < 4; i++)
  {
    if (n > powten[i])
    {
      break;
    }
  }

  for (; i < 5; i++)
  {
    divbuf = udiv(n, powten[i]);
    send_char('0' + divbuf.quot);
    n = divbuf.rem;
  }
}

int main(void)
{
  uint8_t low_first, hi_second;
  uint16_t value;

  /* Setup the USART: 9600 baud */
  UBRR0H = 0;
  UBRR0L = 103;
  UCSR0B = (1 << TXEN0);

  /* Use AVCC as the reference. ADC set to read ADC0
   * Disable digital input circuitry for input 0 */
  ADMUX  = (1 << REFS0);
  DIDR0  = (1 << ADC0D);

  /* Enable ADC; set clock prescaler to 128 so the adc's clock frequency
   * is 125KHz, (which is in the desired zone, 50 - 200KHz) */
  ADCSRA  = (1 << ADEN) | (1 << ADPS0) | (1 << ADPS1) | (1 << ADPS2);

  /* Start ze first conversion! */
  ADCSRA |= (1 << ADSC);

  for (;;)
  {
    /* Wait until the conversion is finished */
    loop_until_bit_is_clear(ADCSRA, ADSC);

    low_first = ADCL;
    hi_second = ADCH;

    /* Start another conversion to run while we USART */
    ADCSRA |= (1 << ADSC);

    value = low_first | (hi_second << 8);

    send_ui16(value);
    send_char('\n');
  }
}

