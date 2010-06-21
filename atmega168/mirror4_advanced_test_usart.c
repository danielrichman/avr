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

/* Similar to mirror4_advanced.c yet with USART tests */

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>

#define clone(f, fp, t, tp) \
  do \
  { \
    if (PIN ## f & (1 << P ## f ## fp)) \
    { \
      PORT ## t &= ~(1 << P ## t ## tp); \
    } \
    else \
    { \
      PORT ## t |= (1 << P ## t ## tp); \
    } \
  } \
  while (0)

#define intsleep(t) \
  do \
  { \
    set_sleep_mode(SLEEP_MODE_ ## t); \
    sleep_enable(); \
    sei(); \
    sleep_cpu(); \
    cli(); \
    sleep_disable(); \
  } \
  while (0)

#define intwait(c, t) \
  do \
  { \
    intsleep(t); \
  } \
  while (c)

EMPTY_INTERRUPT(PCINT1_vect);

static inline void send_string(const char *s)
{
  while (*s != '\0')
  {
    while (!(UCSR0A & (1 << UDRE0)));
    UDR0 = *s;
    s++;
  }

  while (!(UCSR0A & (1 << TXC0)));
  UCSR0A |= (1 << TXC0);
}

int main(void)
{
  /* Setup IO */
  DDRD   = (1 << DDD5) | (1 << DDD6) | (1 << DDD7);
  DDRB   = (1 << DDB0);
  PORTC  = (1 << PC2) | (1 << PC3) | (1 << PC4) | (1 << PC5);
  PCMSK1 = (1 << PCINT10) | (1 << PCINT11) | (1 << PCINT12) | (1 << PCINT13);
  UBRR0H = 0;
  UBRR0L = 103;
  UCSR0B = (1 << TXEN0);

  send_string("Hello\n");

  /* Duplicate */
  for (;;)
  {
    send_string("Cloning\n");
    clone(C, 2, B, 0);
    clone(C, 3, D, 7);
    clone(C, 4, D, 6);
    clone(C, 5, D, 5);

    send_string("Sleeping\n");
    PCICR |= (1 << PCIE1);
    intsleep(PWR_DOWN);
    PCICR &= ~(1 << PCIE1);
    send_string("Woke up\n");
  }
}
