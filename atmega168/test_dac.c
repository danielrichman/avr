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

/* Playing with the MAX541 */

#include <avr/io.h>
#include <stdlib.h>
#include <stdint.h>

#define SS       PB2
#define MOSI     PB3
#define MISO     PB4
#define SCK      PB5

#define  SEL_DAC  DDRD |=  (1 << PD4)
#define USEL_DAC  DDRD &= ~(1 << PD4)

#define  SEL_ADC  DDRD |=  (1 << PD3)
#define USEL_ADC  DDRD &= ~(1 << PD3)

extern div_t udiv(int __num, int __denom) __asm__("__udivmodhi4")
                                                               __ATTR_CONST__;

static inline void send_char(char c)
{
  loop_until_bit_is_set(UCSR0A, UDRE0);
  UDR0 = c;
}

static inline void send_string(const char *s)
{
  while (*s != '\0')
  {
    send_char(*s);
    s++;
  }
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

static inline uint8_t read_char()
{
  loop_until_bit_is_set(UCSR0A, RXC0);
  return UDR0;
}

static inline void read_until_newline()
{
  while (read_char() != '\n');
}

static inline uint16_t read_ui16()
{
  char buf[6];
  uint8_t i;

  for (i = 0; i < 6; i++)
  {
    buf[i] = read_char();

    if (buf[i] == '\n')
    {
      buf[i] = '\0';
      return atoi(buf);
    }
    else if (buf[i] < '0' || buf[i] > '9')
    {
      read_until_newline();
      return 0;
    }
  }

  read_until_newline();
  return 0;
}

static inline void set_value(uint16_t value)
{
  SEL_DAC;

  SPDR = (value & 0xFF00) >> 8;
  loop_until_bit_is_set(SPSR, SPIF);

  SPDR = value & 0xFF;
  loop_until_bit_is_set(SPSR, SPIF);

  USEL_DAC;
}

int main(void)
{
  uint16_t value;

  /* Setup IO */
  UBRR0H = 0;
  UBRR0L = 103;
  UCSR0B = (1 << TXEN0) | (1 << RXEN0);

  DDRB   = (1 << MOSI) | (1 << SCK) | (1 << SS);
  PORTB  = (1 << SS) | (1 << MISO);
  SPCR   = (1 << SPE)  | (1 << MSTR) | (1 << SPR0) | (1 << SPR1);

  DDRD   = (1 << PD3);
  PORTD  = (1 << PD3);

  /* Main */
  send_string("Power up\n");

  while (1)
  {
    send_string("New Value?\n");

    value = read_ui16();

    send_string("Setting new value, ");
    send_ui16(value);
    send_string(".\n");

    set_value(value);

    send_string("Done\n");
  }
}

