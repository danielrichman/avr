#!/usr/bin/make
# -*- makefile -*-

#    Copyright (C) 2010  Daniel Richman
#
#    This program is free software: you can redistribute it and/or modify
#    it under the terms of the GNU General Public License as published by
#    the Free Software Foundation, either version 3 of the License, or
#    (at your option) any later version.
#
#    This program is distributed in the hope that it will be useful,
#    but WITHOUT ANY WARRANTY; without even the implied warranty of
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#    GNU General Public License for more details.
#
#    For a full copy of the GNU General Public License, 
#    see <http://www.gnu.org/licenses/>.

F_CPU = 2000000
code_mmcu = atxmega128a1
prog_mmcu = x128a1

cfiles   := $(wildcard *.c)
hexfiles := $(patsubst %.c,%.hex,$(cfiles))

avr_gcc = avr-gcc
avr_objcopy = avr-objcopy
avr_size = avr-size
avrdude = avrdude

CFLAGS   = -mmcu=$(code_mmcu) -DF_CPU=$(F_CPU) -pipe -Wall -O2
MAPFLAGS = -Wl,-Map,

all : $(hexfiles)

%.elf : %.c
	$(avr_gcc)  $(CFLAGS) -o $@ $<
	$(avr_size) --common $@

%.hex : %.elf
	$(avr_objcopy) -O ihex $< $@

%.hex.upload : %.hex
	$(avrdude) -p $(prog_mmcu) -P usb -c avrispmkii -U flash:w:$<

reset :
	$(avrdude) -p $(prog_mmcu) -P usb -c avrispmkii -U flash:w:blobs/ATxmega128a1_original.hex

clean :
	rm -f *.hex 

.PHONY : clean reset all
.DEFAULT_GOAL := all

