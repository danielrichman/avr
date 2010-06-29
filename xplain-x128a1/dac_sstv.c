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

#include <string.h>
#include <stdint.h>
#include <avr/io.h>
#include <util/delay.h>

#include "smiley.inc"

/* TODO: I've moved the pixel timings to TCD0 but left the others as _delays;
 * I will change them over eventually :) */
/* Solid SSTV Specs: http://www.barberdsp.com/files/Dayton%20Paper.pdf ,
 * complements understanding of the transmission mode that can be gleaned
 * from http://en.wikipedia.org/wiki/SSTV and */
/* Pixel timings must be very precise! In this example at 2MHz increasing
 * TIMER.PER by 1 changes from the leftwards shift seen in 
 * http://i.imgur.com/SzrNU.png to a shift to the right. The plan is to
 * use the xmega's PLL and a 8MHz crystal to get the peripheral frequency
 * high enough to get accurate timings */
/* TODO: The stuff below perhaps could do with a few more comments */
/* TODO: Clear OVFIF when we reset TCD0.CNT and fix the timing error that will
 * inevitably result from the change! (currently the first pixel is *probably* 
 * skipped over; not sure). */

static void pixel(uint8_t value)
{
  DACA.CH0DATA = 1900 - ((value * 45) / 10);
  while (!(TCD0.INTFLAGS & TC0_OVFIF_bm));
  TCD0.INTFLAGS = TC0_OVFIF_bm;
}

static void separator()
{
  DACA.CH0DATA = 1900;
  _delay_us(572);
}

static void sync()
{
  DACA.CH0DATA = 2392;
  _delay_us(4862);
}

static void header()
{
  DACA.CH0DATA = 1344;
  _delay_ms(300);
}

static void vis_startstop()
{
  DACA.CH0DATA = 2392;
  _delay_ms(30);
}

static void vis_zero()
{
  DACA.CH0DATA = 2235;
  _delay_ms(30);
}

static void vis_one()
{
  DACA.CH0DATA = 2546;
  _delay_ms(30);
}

int main(void)
{
  uint8_t m, c;
  uint16_t x, y;

  DACA.CTRLA = DAC_CH0EN_bm | DAC_ENABLE_bm;
  DACA.CTRLC = DAC_REFSEL_INT1V_gc;
  DACA.CH0DATA = 0x00;

  TCD0.PER = 932;
  TCD0.CTRLA = TC_CLKSEL_DIV1_gc;

  for (;;)
  {
    header();
    sync();
    sync();
    header();

    vis_startstop();
    for (m = 0x01; m != 0x00; m <<= 1)
    {
      if (m & 0xAC)   vis_one();
      else            vis_zero();
    }
    vis_startstop();

    for (y = 0; y < 16; y++)
    {
      sync();
      separator();
      TCD0.CNT = 0;

      for (c = 0; c < 3; c++)
      {
        for (x = 0; x < 320; x++)
        {
          pixel(128);
        }
      }

      separator();
    }

    for (y = 0; y < 240; y++)
    {
      sync();

      /* Green, Blue, Red */
      for (c = 0; c < 3; c++)
      {
        separator();
        TCD0.CNT = 0;

        for (x = 0; x < 320; x++)
        {
          /* Scale the 32x24 image up: divide x and y by 10. */
          pixel(GIMP_IMAGE_pixel_data[((y / 10) * 32 + (x / 10)) * 3 + c]);
        }
      }

      separator();
    }
  }
}
