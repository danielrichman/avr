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
/*

Dual 7 Segment Display connections:

PA0 - B1
PA1 - A1
PA2 - G1
PA3 - F1
PA4 - 
PA5 - E1
PA6 - D1
PA7 - C1

PC1 - F2
PC2 - A2
PC3 - B2
PC4 - 
PC5 - DP2
PC6 - C2
PC7 - G2

PE2 - D2
PE1 - E2
PE0 - DP1

       PA       PC     PE
1   BAGF-EDC -------- P--
2   -------- -FAB-PCG -ED

    ABCDEFG   1-PA     2-PC     2-PE
0 - 1111110 11010111 01110010 01100000
1 - 0110000 10000001 00010010 00000000
2 - 1101101 11100110 00110001 01100000
3 - 1111001 11100011 00110011 00100000
4 - 0110011 10110001 01010011 00000000
5 - 1011011 01110011 01100011 00100000
6 - 1011111 01110111 01100011 01100000
7 - 1110000 11000001 00110010 00000000
8 - 1111111 11110111 01110011 01100000
9 - 1111011 11110011 01110011 00100000
A - 1110111 11110101 01110011 01000000
B - 0011111 00110111 01000011 01100000
C - 0001101 00100110 00000001 01100000
D - 0111101 10100111 00010011 01100000
E - 1001111 01110110 01100001 01100000
F - 1000111 01110100 01100001 01000000
*/

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <stdint.h>
#include <stdlib.h>

uint8_t digitone_porta[16] = { 0xeb, 0x81, 0x67, 0xc7, 0x8d, 0xce, 0xee, 0x83, 
                               0xef, 0xcf, 0xaf, 0xec, 0x64, 0xe5, 0x6e, 0x2e };
uint8_t digittwo_portc[16] = { 0x4e, 0x48, 0x8c, 0xcc, 0xca, 0xc6, 0xc6, 0x4c, 
                               0xce, 0xce, 0xce, 0xc2, 0x80, 0xc8, 0x86, 0x86 };
uint8_t digittwo_porte[16] = { 0x06, 0x00, 0x06, 0x04, 0x00, 0x04, 0x06, 0x00, 
                               0x06, 0x04, 0x02, 0x06, 0x06, 0x06, 0x06, 0x02 };

#define digitdisplay_one(value)   DDRA = digitone_porta[value]
#define digitdisplay_twoa(value)  DDRC = digittwo_portc[value]
#define digitdisplay_twob(value)  DDRE = digittwo_porte[value]

extern div_t udiv(int __num, int __denom) __asm__("__udivmodhi4")
                                                               __ATTR_CONST__;

void digitdisplay_dec(uint8_t value)
{
  div_t d;
  d = udiv(value, 10);

  digitdisplay_one(d.quot);
  digitdisplay_twoa(d.rem);
  digitdisplay_twob(d.rem);
}

#define first_four(byte)       (0x0F & (byte))
#define  last_four(byte)      ((0xF0 & (byte)) >> 4)

void digitdisplay_hex(uint8_t value)
{
  digitdisplay_one(last_four(value));
  digitdisplay_twoa(first_four(value));
  digitdisplay_twob(first_four(value));
}

uint8_t valuea;
uint8_t type;

ISR (TIMER1_COMPA_vect)
{
  if (type)
  {
    digitdisplay_hex(valuea);
  }
  else
  {
    digitdisplay_dec(valuea);
  }

  valuea++;
  if (!type && valuea == 100)  valuea = 0;
  if (valuea == 0)             type = !type;
}

int main()
{
  PORTA  = 0x00;
  PORTC  = 0x00;
  PORTE  = 0x00;
  DDRA   = 0x00;
  DDRC   = 0x00;
  DDRE   = 0x00;

  OCR1A  = 15625;
  TCCR1B = _BV(WGM12) | _BV(CS12);
  TIMSK  = _BV(OCIE1A);
  
  sei();

  for (;;) sleep_mode();
}
