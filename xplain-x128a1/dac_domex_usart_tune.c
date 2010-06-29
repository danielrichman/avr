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

/* This is a hacky variant of dac_domex.c. It allows tuning of the DELAY and
 * TONE_SHIFT variables by the method used in usart_dac.c and the accompanying
 * pygtk app. The pygtk app txes a 16bit int between 0 and 4096, so we take the
 * first byte, which is between 0 and 15, add 20 to it and use that as 
 * TONE_SHIFT. The other byte, we take, add 1300 to and use as DELAY. 
 * Therefore to change TONE_SHIFT by one you have to go up 256 on the slider.
 * The sweet spot is at 3000; and these values have therefore been hardcoded
 * into dac_domex.c */

/* Uses the XPLAINBridge in USART bridge mode: 9600 baud */
/* The DomEX22 code below originates from the fl-digi program;
 * it has since been modified by CUSF and then myself */

#include <string.h>
#include <stdint.h>
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>

#define NUM_TONES 18
#define BASE 2103
volatile uint8_t TONE_SHIFT;

// This is the primary alphabet only, if you want the secondary alphabet, go fish
static uint8_t varicode[][3] = {
  { 1,15, 9}, { 1,15,10}, { 1,15,11}, { 1,15,12}, { 1,15,13}, { 1,15,14}, 
  { 1,15,15}, { 2, 8, 8}, { 2,12, 0}, { 2, 8, 9}, { 2, 8,10}, { 2, 8,11}, 
  { 2, 8,12}, { 2,13, 0}, { 2, 8,13}, { 2, 8,14}, { 2, 8,15}, { 2, 9, 8}, 
  { 2, 9, 9}, { 2, 9,10}, { 2, 9,11}, { 2, 9,12}, { 2, 9,13}, { 2, 9,14}, 
  { 2, 9,15}, { 2,10, 8}, { 2,10, 9}, { 2,10,10}, { 2,10,11}, { 2,10,12}, 
  { 2,10,13}, { 2,10,14}, { 0, 0, 0}, { 7,11, 0}, { 0, 8,14}, { 0,10,11}, 
  { 0, 9,10}, { 0, 9, 9}, { 0, 8,15}, { 7,10, 0}, { 0, 8,12}, { 0, 8,11}, 
  { 0, 9,13}, { 0, 8, 8}, { 2,11, 0}, { 7,14, 0}, { 7,13, 0}, { 0, 8, 9}, 
  { 3,15, 0}, { 4,10, 0}, { 4,15, 0}, { 5, 9, 0}, { 6, 8, 0}, { 5,12, 0}, 
  { 5,14, 0}, { 6,12, 0}, { 6,11, 0}, { 6,14, 0}, { 0, 8,10}, { 0, 8,13}, 
  { 0,10, 8}, { 7,15, 0}, { 0, 9,15}, { 7,12, 0}, { 0, 9, 8}, { 3, 9, 0}, 
  { 4,14, 0}, { 3,12, 0}, { 3,14, 0}, { 3, 8, 0}, { 4,12, 0}, { 5, 8, 0}, 
  { 5,10, 0}, { 3,10, 0}, { 7, 8, 0}, { 6,10, 0}, { 4,11, 0}, { 4, 8, 0}, 
  { 4,13, 0}, { 3,11, 0}, { 4, 9, 0}, { 6,15, 0}, { 3,13, 0}, { 2,15, 0}, 
  { 2,14, 0}, { 5,11, 0}, { 6,13, 0}, { 5,13, 0}, { 5,15, 0}, { 6, 9, 0}, 
  { 7, 9, 0}, { 0,10,14}, { 0,10, 9}, { 0,10,15}, { 0,10,10}, { 0, 9,12}, 
  { 0, 9,11}, { 4, 0, 0}, { 1,11, 0}, { 0,12, 0}, { 0,11, 0}, { 1, 0, 0}, 
  { 0,15, 0}, { 1, 9, 0}, { 0,10, 0}, { 5, 0, 0}, { 2,10, 0}, { 1,14, 0}, 
  { 0, 9, 0}, { 0,14, 0}, { 6, 0, 0}, { 3, 0, 0}, { 1, 8, 0}, { 2, 8, 0}, 
  { 7, 0, 0}, { 0, 8, 0}, { 2, 0, 0}, { 0,13, 0}, { 1,13, 0}, { 1,12, 0}, 
  { 1,15, 0}, { 1,10, 0}, { 2, 9, 0}, { 0,10,12}, { 0, 9,14}, { 0,10,13}, 
  { 0,11, 8}, { 2,10,15}, { 2,11, 8}, { 2,11, 9}, { 2,11,10}, { 2,11,11}, 
  { 2,11,12}, { 2,11,13}, { 2,11,14}, { 2,11,15}, { 2,12, 8}, { 2,12, 9}, 
  { 2,12,10}, { 2,12,11}, { 2,12,12}, { 2,12,13}, { 2,12,14}, { 2,12,15}, 
  { 2,13, 8}, { 2,13, 9}, { 2,13,10}, { 2,13,11}, { 2,13,12}, { 2,13,13}, 
  { 2,13,14}, { 2,13,15}, { 2,14, 8}, { 2,14, 9}, { 2,14,10}, { 2,14,11}, 
  { 2,14,12}, { 2,14,13}, { 2,14,14}, { 2,14,15}, { 0,11, 9}, { 0,11,10}, 
  { 0,11,11}, { 0,11,12}, { 0,11,13}, { 0,11,14}, { 0,11,15}, { 0,12, 8}, 
  { 0,12, 9}, { 0,12,10}, { 0,12,11}, { 0,12,12}, { 0,12,13}, { 0,12,14}, 
  { 0,12,15}, { 0,13, 8}, { 0,13, 9}, { 0,13,10}, { 0,13,11}, { 0,13,12}, 
  { 0,13,13}, { 0,13,14}, { 0,13,15}, { 0,14, 8}, { 0,14, 9}, { 0,14,10}, 
  { 0,14,11}, { 0,14,12}, { 0,14,13}, { 0,14,14}, { 0,14,15}, { 0,15, 8}, 
  { 0,15, 9}, { 0,15,10}, { 0,15,11}, { 0,15,12}, { 0,15,13}, { 0,15,14}, 
  { 0,15,15}, { 1, 8, 8}, { 1, 8, 9}, { 1, 8,10}, { 1, 8,11}, { 1, 8,12}, 
  { 1, 8,13}, { 1, 8,14}, { 1, 8,15}, { 1, 9, 8}, { 1, 9, 9}, { 1, 9,10}, 
  { 1, 9,11}, { 1, 9,12}, { 1, 9,13}, { 1, 9,14}, { 1, 9,15}, { 1,10, 8}, 
  { 1,10, 9}, { 1,10,10}, { 1,10,11}, { 1,10,12}, { 1,10,13}, { 1,10,14}, 
  { 1,10,15}, { 1,11, 8}, { 1,11, 9}, { 1,11,10}, { 1,11,11}, { 1,11,12}, 
  { 1,11,13}, { 1,11,14}, { 1,11,15}, { 1,12, 8}, { 1,12, 9}, { 1,12,10}, 
  { 1,12,11}, { 1,12,12}, { 1,12,13}, { 1,12,14}, { 1,12,15}, { 1,13, 8}, 
  { 1,13, 9}, { 1,13,10}, { 1,13,11}, { 1,13,12}, { 1,13,13}, { 1,13,14}, 
  { 1,13,15}, { 1,14, 8}, { 1,14, 9}, { 1,14,10}, { 1,14,11}, { 1,14,12}, 
  { 1,14,13}, { 1,14,14}, { 1,14,15}, { 1,15, 8}, { 6,15, 9} // idle 
};

static void dominoex_sendtone(uint8_t tone)
{
  while (!(TCD0.INTFLAGS & TC0_OVFIF_bm));
  TCD0.INTFLAGS = TC0_OVFIF_bm;
  // _delay_ms(44);

  DACA.CH0DATA = BASE + (tone * TONE_SHIFT);
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
  int sym;
  uint8_t *code;

  code = varicode[c];

  dominoex_sendsymbol(code[0]);

  for (sym = 1; sym < 3; sym++)
  {
    // If the MSB is set we need to send another nibble 
    if (code[sym] & 0x8)
    {
      dominoex_sendsymbol(code[sym]);
    }
    else
    {
      break;
    }
  }
}

uint8_t status;

ISR(USARTC0_RXC_vect)
{
  if (status == 0)
  {
    TONE_SHIFT = 25 + USARTC0.DATA;
    status++;
  }
  else
  {
    TCD0.PER = 1300 + USARTC0.DATA;
    status = 0;
  }
}

char *str = "Hello world\n";

int main(void)
{
  uint8_t i;

  DACA.CTRLA = DAC_CH0EN_bm | DAC_ENABLE_bm;
  DACA.CTRLC = DAC_REFSEL_INT1V_gc;
  DACA.CH0DATA = 0x00;

  // TCD0.PER = 1420; // ((F_CPU / 64) / 22);
  TCD0.CTRLA = TC_CLKSEL_DIV64_gc;

  USARTC0.CTRLA = USART_RXCINTLVL_HI_gc;
  USARTC0.CTRLC = USART_CHSIZE_8BIT_gc;
  USARTC0.BAUDCTRLA = 12;
  USARTC0.CTRLB = USART_RXEN_bm;

  PMIC.CTRL = PMIC_HILVLEN_bm;
  sei();

  for (;;)
  {
    for (i = 0; i < strlen(str); i++)
    {
      transmit_dominoex_character(str[i]);
    }
  }
}
