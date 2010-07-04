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

    The 1 Wire temperature code originates from ALIEN-1, which is
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
#include <stdio.h>
#include <avr/io.h>
#include <util/delay.h>

#define skiprom_cmd        0xCC
#define convtemp_cmd       0x44
#define readscratch_cmd    0xBE

uint8_t temp_crc, temp_msb, temp_lsb;

#define temp_release()  PORTA.DIRCLR = 0x40
#define temp_pullow()   PORTA.DIRSET = 0x40
#define temp_read()     (PORTA.IN & 0x40)

uint8_t temp_retrieve();
void temp_writebyte(uint8_t db);
uint8_t temp_reset();
void temp_readbyte(uint8_t *target);
uint8_t temp_readbit();
void temp_crcpush(uint8_t bit);

uint8_t temp_retrieve()
{
  uint8_t i, d;

  /* Reset some stuff */
  temp_crc = 0;
  temp_msb = 0;
  temp_lsb = 0;

  /* RESET */
  if (!temp_reset())
  {
    return 0;
  }

  /* SKIPROM cmd */
  temp_writebyte(skiprom_cmd);

  /* CONV_T cmd */
  temp_writebyte(convtemp_cmd);

  _delay_ms(1000);

  /* CONV_T test */
  d = temp_readbit();

  if (!d)
  {
    /* If it hasn't completed it by now... gah! */
    return 0;
  }

  /* RESET */
  if (!temp_reset())
  {
    return 0;
  }

  /* SKIPROM cmd */
  temp_writebyte(skiprom_cmd);

  /* READSCRATCH cmd */
  temp_writebyte(readscratch_cmd);

  /* READSCRATCH readbytes */
  /* Bytes 0 and 1 are temp data in little endian (that's good),
   * then the rest can be ignored (but automatically shifted into the CRC
   * by readbyte). */
  temp_readbyte(&temp_lsb);
  temp_readbyte(&temp_msb);

  /* Read the remaining 7 bytes, CRC and discard */
  for (i = 0; i < 7; i++)
  {
    temp_readbyte(NULL);
  }

  /* SIGN_CHECK */
  /* For some reason, all the bits in the most significan byte of the temp.
   * should be the same. We use them to signal other things, like how good the 
   * temp is, so check that they actually are the all 0 or 1 */
  if (temp_msb != 0x00 && temp_msb != 0xFF)
  {
    return 0;
  }

  /* CRC_CHECK */
  /* The CRC is such that if you shift the last byte, the CRC byte, into the
   * CRC register, then it should equal zero. */
  if (temp_crc != 0)
  {
    return 0;
  }

  return 1;
}

uint8_t temp_reset()
{
  uint8_t ok;

  ok = 1;

  /* RESET_PULSE part1 */
  temp_pullow();
  _delay_us(560);
  temp_release();

  _delay_us(100);

  if (temp_read())
  {
    /* If it's high, then the sensor isn't there, or isn't working. */
    ok = 0;
  }

  _delay_us(240);

  return ok;
}

void temp_writebyte(uint8_t db)
{
  uint8_t i, b;

  /* Shift left until we rollover to 0 */
  for (i = 0x01; i != 0x00; i = i << 1)
  {
    /* Select the required bit */
    b = db & i;

    /* If we are writing low then pull it down for the full 64us, 
     * release and wait a tiny bit for it to be surely high.
     * If we are writing high then pull it down for 4us then release,
     * and wait out the rest of the 72us (72 - 4 = 68) */

    temp_pullow();

    if (b)  _delay_us(4);            /* Wait 4us    */
    else    _delay_us(64);           /* Wait 64us   */

    temp_release();

    if (b)  _delay_us(68);    /* Wait out the rest of the slot */
    else    _delay_us(8);     /* Wait 8us for it to come high  */
  }
}

void temp_readbyte(uint8_t *target)
{
  uint8_t i, d;

  for (i = 0x01; i != 0x00; i = i << 1)
  {
    d = temp_readbit();
    temp_crcpush(d);
 
    if (d && target != NULL)
    {
      *target |= i;
    }
  }
}

uint8_t temp_readbit()
{
  uint8_t d;

  d = 0;

  temp_pullow();
  _delay_us(1.5);
  temp_release();

  _delay_us(12);

  d = temp_read();

  /* Wait for the slot to end: Wait another 52us */

  _delay_us(52);

  return d;
}

void temp_crcpush(uint8_t bit)
{
  /* XOR gates push into bits 7, 3 and 2, so move the bit in 'bit' to
   * be in those. Take the bit and XOR it with the least significant
   * CRC bit. */

  if ((bit ? 0x1 : 0x0) ^ (temp_crc & 0x01))
  {
    bit = 0x8C;
  }
  else
  {
    bit = 0x00;
  }

  temp_crc >>=  1;
  temp_crc ^= bit;
}

#define NUM_TONES 18
#define BASE 2103
#define TONE_SHIFT 36
#define DELAY 1484
#define IDLE 1000

/* WARNING! The below alphabet and the code to retrieve the data from
 * it has been mutilated as much as possible to squeese it into as little
 * ROM as possible. You can see how it looked before in 
 * dac_domex_usart_tune.c. The new table was generated by 
 * /misc/dac_domex_table.c
 *
 *  Eg: The first two sets are { 1,15, 9}, { 1,15,10},
 *  that is, 0x1, 0xf, 0x9, 0x1, 0xf, 0xa
 *
 *  We represent each set as a 16 bit integer, eg. (0x09f1 for the first set)
 *  so that when reading it we can use the bitmask 0xf to grab the first
 *  number, shift by 4 to the right, and repeat. Because the AVR is little
 *  endian the first set is therefore \xf1 \x09
 *
 *  Since there are only three numbers in each symbol 2 symbols can actually
 *  fit into 2 bytes. We take the first number of the second symbol and fill
 *  the empty space in the first byte, to get, for example
 *    0   1   2
 *    \xf1\x19\xaf
 *
 *  If we read a 16bit int at [0], then we get 0x19f1 (little endian), 
 *  meaning that the following tones are transmitted: 1, 15, 9. The final
 *  1 (that belongs to the second set) is ignored since it doesn't have
 *  the MSB set (ie & 0x8).
 *
 *  If we read a 16bit int at [1], we get 0xaf19. We discard the 9 since
 *  it belongs to [0], and then have our data 0x0af1 which we can use.
 *  (see transmit_dominoex_character). */
/* This is the primary alphabet only;
 * if you want the secondary alphabet, go fish */
static uint8_t varicode[] = 
  "\xf1\x19\xaf\xf1\x1b\xcf\xf1\x1d\xef\xf1\x2f\x88\xc2\x20\x98\x82\x2a"
  "\xb8\x82\x2c\x0d\x82\x2d\xe8\x82\x2f\x89\x92\x29\xa9\x92\x2b\xc9\x92"
  "\x2d\xe9\x92\x2f\x8a\xa2\x29\xaa\xa2\x2b\xca\xa2\x2d\xea\x00\x70\x0b"
  "\x80\x0e\xba\x90\x0a\x99\x80\x7f\x0a\x80\x0c\xb8\x90\x0d\x88\xb2\x70"
  "\x0e\xd7\x00\x98\xf3\x40\x0a\xf4\x50\x09\x86\x50\x0c\xe5\x60\x0c\xb6"
  "\x60\x0e\x80\x0a\xd8\xa0\x78\x0f\x90\x7f\x0c\x90\x38\x09\xe4\x30\x0c"
  "\xe3\x30\x08\xc4\x50\x08\xa5\x30\x0a\x87\x60\x0a\xb4\x40\x08\xd4\x30"
  "\x0b\x94\x60\x0f\xd3\x20\x0f\xe2\x50\x0b\xd6\x50\x0d\xf5\x60\x09\x97"
  "\x00\xea\xa0\x09\xfa\xa0\x0a\xc9\x90\x4b\x00\xb1\x00\x0c\xb0\x10\x00"
  "\xf0\x10\x09\xa0\x50\x00\xa2\x10\x0e\x90\x00\x0e\x06\x30\x00\x81\x20"
  "\x08\x07\x00\x08\x02\x00\x0d\xd1\x10\x0c\xf1\x10\x0a\x92\x00\xca\x90"
  "\x0e\xda\xb0\x28\xfa\xb2\x28\x9b\xb2\x2a\xbb\xb2\x2c\xdb\xb2\x2e\xfb"
  "\xc2\x28\x9c\xc2\x2a\xbc\xc2\x2c\xdc\xc2\x2e\xfc\xd2\x28\x9d\xd2\x2a"
  "\xbd\xd2\x2c\xdd\xd2\x2e\xfd\xe2\x28\x9e\xe2\x2a\xbe\xe2\x2c\xde\xe2"
  "\x2e\xfe\xb0\x09\xab\xb0\x0b\xcb\xb0\x0d\xeb\xb0\x0f\x8c\xc0\x09\xac"
  "\xc0\x0b\xcc\xc0\x0d\xec\xc0\x0f\x8d\xd0\x09\xad\xd0\x0b\xcd\xd0\x0d"
  "\xed\xd0\x0f\x8e\xe0\x09\xae\xe0\x0b\xce\xe0\x0d\xee\xe0\x0f\x8f\xf0"
  "\x09\xaf\xf0\x0b\xcf\xf0\x0d\xef\xf0\x1f\x88\x81\x19\xa8\x81\x1b\xc8"
  "\x81\x1d\xe8\x81\x1f\x89\x91\x19\xa9\x91\x1b\xc9\x91\x1d\xe9\x91\x1f"
  "\x8a\xa1\x19\xaa\xa1\x1b\xca\xa1\x1d\xea\xa1\x1f\x8b\xb1\x19\xab\xb1"
  "\x1b\xcb\xb1\x1d\xeb\xb1\x1f\x8c\xc1\x19\xac\xc1\x1b\xcc\xc1\x1d\xec"
  "\xc1\x1f\x8d\xd1\x19\xad\xd1\x1b\xcd\xd1\x1d\xed\xd1\x1f\x8e\xe1\x19"
  "\xae\xe1\x1b\xce\xe1\x1d\xee\xe1\x1f\x8f\xf6";

static void dominoex_delay()
{
  while (!(TCD0.INTFLAGS & TC0_OVFIF_bm));
  TCD0.INTFLAGS = TC0_OVFIF_bm;
}

static void dominoex_sendtone(uint8_t tone)
{
  dominoex_delay();
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
  uint16_t data;

  /* 2 nibbles are packed in 3 bytes; to retrieve the
   * data multiply c by (3/2). See head of file*/
  data = *((uint16_t *) (varicode + ((c * 3) / 2)));

  if (c & 0x01)
  {
    /* If it was odd we need to discard the first nibble */
    data >>= 4;
  }

  /* First nibble will not have the MSB set, but any multi-nibble
   * chars will have 0x08 set in their "continuation nibbles"  */
  do
  {
    dominoex_sendsymbol(data & 0xF);
    data >>= 4;
  }
  while (data & 0x8);
}

void transmit_dominoex_string(char *str)
{
  uint8_t i, wakeup;

  TCD0.CNT = 0;
  TCD0.INTFLAGS = TC0_OVFIF_bm;

  for (wakeup = 0; wakeup < 5; wakeup++)
  {
    transmit_dominoex_character(0);
  }

  for (i = 0; i < strlen(str); i++)
  {
    transmit_dominoex_character(str[i]);
  }
}

char buf[100];

int main(void)
{
  uint8_t value;

  DACA.CTRLA = DAC_CH0EN_bm | DAC_ENABLE_bm;
  DACA.CTRLC = DAC_REFSEL_INT1V_gc;
  DACA.CH0DATA = IDLE;

  TCD0.PER = DELAY;
  TCD0.CTRLA = TC_CLKSEL_DIV64_gc;

  transmit_dominoex_string("\n\nHello World    ");

  for (;;)
  {
    if (!temp_retrieve())
    {
      transmit_dominoex_string("\nTemperature read failed.    ");
    }

    value = (temp_lsb >> 1) | (temp_msb & 0x80);

    snprintf(buf, sizeof(buf), "\nTemperature: 0x%02x%02x %i.%c    ", 
             temp_msb, temp_lsb, 
             value, temp_lsb & 0x01 ? '5' : '0');

    transmit_dominoex_string(buf);

    dominoex_delay();

    DACA.CH0DATA = IDLE;
  }
}
