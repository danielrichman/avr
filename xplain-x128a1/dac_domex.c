/*
    The DomEX22 code below originates from the fl-digi program;
    it has since been modified by CUSF and then myself. Therefore,

    fldigi/src/dominoex/dominoex.cxx & fldigi/include/dominoex.h
       Copyright (C) 2001, 2002, 2003
                  Tomi Manninen (oh2bns@sral.fi)
       Copyright (C) 2006
                  Hamish Moffatt (hamish@debian.org)
       Copyright (C) 2006, 2008-2009
                  David Freese (w1hkj@w1hkj.com)

       based on code in gmfsk

    fldigi/src/dominoex/dominovar.cxx
       Copyright (C) 2001, 2002, 2003
                  Tomi Manninen (oh2bns@sral.fi)

    Copyright (C) 2010 CU Spaceflight
    Copyright (C) 2010 Daniel Richman

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

#include <string.h>
#include <stdint.h>
#include <avr/io.h>
#include <util/delay.h>

#define NUM_TONES 18
#define BASE 2741
#define TONE_SHIFT 36
#define DELAY 1484

/* This is the primary alphabet only;
 * if you want the secondary alphabet, go fish */
static uint8_t *varicode = 
 "\x1f\x91\xfa\x1f\xb1\xfc\x1f\xd1\xfe\x1f\xf2\x88\x2c\x02\x89\x28\xa2\x8b\x28"
 "\xc2\xd0\x28\xd2\x8e\x28\xf2\x98\x29\x92\x9a\x29\xb2\x9c\x29\xd2\x9e\x29\xf2"
 "\xa8\x2a\x92\xaa\x2a\xb2\xac\x2a\xd2\xae\x00\x07\xb0\x08\xe0\xab\x09\xa0\x99"
 "\x08\xf7\xa0\x08\xc0\x8b\x09\xd0\x88\x2b\x07\xe0\x7d\x00\x89\x3f\x04\xa0\x4f"
 "\x05\x90\x68\x05\xc0\x5e\x06\xc0\x6b\x06\xe0\x08\xa0\x8d\x0a\x87\xf0\x09\xf7"
 "\xc0\x09\x83\x90\x4e\x03\xc0\x3e\x03\x80\x4c\x05\x80\x5a\x03\xa0\x78\x06\xa0"
 "\x4b\x04\x80\x4d\x03\xb0\x49\x06\xf0\x3d\x02\xf0\x2e\x05\xb0\x6d\x05\xd0\x5f"
 "\x06\x90\x79\x00\xae\x0a\x90\xaf\x0a\xa0\x9c\x09\xb4\x00\x1b\x00\xc0\x0b\x01"
 "\x00\x0f\x01\x90\x0a\x05\x00\x2a\x01\xe0\x09\x00\xe0\x60\x03\x00\x18\x02\x80"
 "\x70\x00\x80\x20\x00\xd0\x1d\x01\xc0\x1f\x01\xa0\x29\x00\xac\x09\xe0\xac\x0b"
 "\x82\xaf\x2b\x82\xb9\x2b\xa2\xbb\x2b\xc2\xbd\x2b\xe2\xbf\x2c\x82\xc9\x2c\xa2"
 "\xcb\x2c\xc2\xcd\x2c\xe2\xcf\x2d\x82\xd9\x2d\xa2\xdb\x2d\xc2\xdd\x2d\xe2\xdf"
 "\x2e\x82\xe9\x2e\xa2\xeb\x2e\xc2\xed\x2e\xe2\xef\x0b\x90\xba\x0b\xb0\xbc\x0b"
 "\xd0\xbe\x0b\xf0\xc8\x0c\x90\xca\x0c\xb0\xcc\x0c\xd0\xce\x0c\xf0\xd8\x0d\x90"
 "\xda\x0d\xb0\xdc\x0d\xd0\xde\x0d\xf0\xe8\x0e\x90\xea\x0e\xb0\xec\x0e\xd0\xee"
 "\x0e\xf0\xf8\x0f\x90\xfa\x0f\xb0\xfc\x0f\xd0\xfe\x0f\xf1\x88\x18\x91\x8a\x18"
 "\xb1\x8c\x18\xd1\x8e\x18\xf1\x98\x19\x91\x9a\x19\xb1\x9c\x19\xd1\x9e\x19\xf1"
 "\xa8\x1a\x91\xaa\x1a\xb1\xac\x1a\xd1\xae\x1a\xf1\xb8\x1b\x91\xba\x1b\xb1\xbc"
 "\x1b\xd1\xbe\x1b\xf1\xc8\x1c\x91\xca\x1c\xb1\xcc\x1c\xd1\xce\x1c\xf1\xd8\x1d"
 "\x91\xda\x1d\xb1\xdc\x1d\xd1\xde\x1d\xf1\xe8\x1e\x91\xea\x1e\xb1\xec\x1e\xd1"
 "\xee\x1e\xf1\xf8\x6f\x90";

static void dominoex_sendtone(uint8_t tone)
{
  while (!(TCD0.INTFLAGS & TC0_OVFIF_bm));
  TCD0.INTFLAGS = TC0_OVFIF_bm;

  DACA.CH0DATA = BASE - (tone * TONE_SHIFT);
}

static void dominoex_sendsymbol(uint8_t sym)
{
  uint8_t tone;  
  static uint8_t txprevtone = 0;

  tone = (txprevtone + 2 + sym) % NUM_TONES;
  txprevtone = tone;
  dominoex_sendtone(tone);
}

void transmit_dominoex_character(uint8_t c)
{
  uint8_t sym;
  uint16_t num, loc, data;

  num = c * 3;
  loc = (num & ~0x01) / 2; /* num & ~0x01 ensures it's even */
  data = (varicode[loc] << 8) | varicode[loc + 1];

  if (num & 0x01)
  {
    /* it was odd */
    data <<= 4;
  }

  do
  {
    dominoex_sendsymbol((data & 0xF000) >> 12);
    data <<= 4;
  }
  while (data & 0x8000);
}

char *str = "Greetings, human.\n";

int main(void)
{
  uint8_t i;

  DACA.CTRLA = DAC_CH0EN_bm | DAC_ENABLE_bm;
  DACA.CTRLC = DAC_REFSEL_INT1V_gc;
  DACA.CH0DATA = 0x00;

  TCD0.PER = DELAY;
  TCD0.CTRLA = TC_CLKSEL_DIV64_gc;

  for (;;)
  {
    for (i = 0; i < strlen(str); i++)
    {
      transmit_dominoex_character(str[i]);
    }
  }
}
