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

/* Playing with the MAX541 and MAX1416 (the latter didn't work properly) */

#include <avr/io.h>
#include <util/delay.h>
#include <stdlib.h>
#include <stdint.h>

#define SS       PB2
#define MOSI     PB3
#define MISO     PB4
#define SCK      PB5

#define  SEL_DAC  DDRD |=  (1 << PD4)
#define USEL_DAC  DDRD &= ~(1 << PD4)

#define  SEL_ADC  DDRB |=  (1 << PB1)
#define USEL_ADC  DDRB &= ~(1 << PB1)

extern div_t udiv(int __num, int __denom) __asm__("__udivmodhi4")
                                                               __ATTR_CONST__;

static inline void send_char(char c)
{
  loop_until_bit_is_set(UCSR0A, UDRE0);
  UDR0 = c;
}

#define send_debug_string(s)  /* send_string(s) */
#define send_debug_ui16(u)    /* send_ui16(u)   */

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
    if (n >= powten[i])
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

static inline uint8_t spi_char(uint8_t c)
{
  SPDR = c;
  loop_until_bit_is_set(SPSR, SPIF);
  return SPDR;
}

static inline void set_value(uint16_t value)
{
  SEL_DAC;

  spi_char((value & 0xFF00) >> 8);
  spi_char(value & 0xFF);

  USEL_DAC;
}

static inline uint16_t get_value(uint8_t channel)
{
  uint16_t result;
  uint8_t cmask, i;

  if (channel)
  {
    send_debug_string("Selecting Channel 2...\n");
    cmask = 1;
  }
  else
  {
    send_debug_string("Selecting Channel 1...\n");
    cmask = 0;
  }

  send_debug_string("Reading...\n");
  send_debug_string("Data Register: ");

  SEL_ADC;

  /* FIXME */
  for (i = 0; i < 2; i++)
  {
    spi_char(0x38 | cmask);
    spi_char(0xFF);
    spi_char(0xFF);

    loop_until_bit_is_set(PINC, PC1);
    loop_until_bit_is_clear(PINC, PC1);
  }

  spi_char(0x38 | cmask);
  result  = spi_char(0xFF) << 8;
  result |= spi_char(0xFF);

  USEL_ADC;

  send_debug_ui16(result);
  send_debug_string(".\n");

  return result;
}

int main(void)
{
  uint16_t value, result1, result2;

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
  send_debug_string("\n\n");
  send_debug_string("Power up\n");

  if (!(PINC & (1 << PC1)))
  {
    send_debug_string("DRDY low\n");
  }

  send_debug_string("Configuring ADC\n");

  SEL_ADC;

  spi_char(0x20);
  spi_char(0xA7);
  spi_char(0x10);
  spi_char(0x46);

  USEL_ADC;

  send_debug_string("\n");

  value = 0;

  while (1)
  {
    send_debug_string("Setting new value, ");
    send_debug_ui16(value);
    send_debug_string(".\n");

    set_value(value);

    send_debug_string("Done\n\n");

    send_debug_string("Getting result... \n");
    result1 = get_value(0);
    result2 = get_value(1);
    send_debug_string("Done\n\n");

    send_ui16(value);
    send_char(' ');
    send_ui16(result1);
    send_char(' ');
    send_ui16(result2);
    send_char('\n');

    _delay_ms(100);

    value += 1024;
  }
}

