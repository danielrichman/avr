#!/usr/bin/env python

# Copyright 2010 Daniel Richman

# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.

# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.

import os
import sys
import struct

import pygtk
pygtk.require('2.0')
import gtk

print "Port: ", sys.argv[1]

os.system("stty -F %s cs8 9600 ignbrk -brkint -icrnl -imaxbel -opost -onlcr "
          "-isig -icanon -iexten -echo -echoe -echok -echoctl -echoke noflsh "
          "-ixon -crtscts" % sys.argv[1])

def change(adj, l, f):
  l.set_text(str(int(adj.value)))
  f.write(struct.pack(">h", int(adj.value)))

f = open(sys.argv[1], 'wb', 0)
w = gtk.Window()
l = gtk.Label()
a = gtk.Adjustment(upper=4096, step_incr=1, page_incr=128, page_size=1)
a.connect("value_changed", change, l, f)
a.value_changed()
h = gtk.HScrollbar(adjustment=a)
b = gtk.HBox(False, 0)
b.pack_start(l, True, True, 0)
b.pack_start(h, True, True, 0)
w.add(b)
h.set_size_request(1000, 20)
l.set_size_request(100, 20)
w.show_all()
gtk.main()
