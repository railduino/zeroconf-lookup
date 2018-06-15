#
# Copyright (c) 2017-2018 Volker Wiegand <volker@railduino.de>
#
# This file is part of Zeroconf-Lookup.
# Project home: https://www.railduino.de/zeroconf-lookup
# Source code:  https://github.com/railduino/zeroconf-lookup.git
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in all
# copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.
#

prefix ?= /usr/local

HDRS := $(wildcard *.h)
OBJS := $(patsubst %.c,%.o,$(wildcard *.c))

CFLAGS += -I. -W -Wall -Wextra -Wshadow -Wstrict-prototypes -Wpointer-arith -Wcast-qual -Winline -Werror

all: zeroconf_lookup

zeroconf_lookup: $(OBJS)
	$(CC) -o $@ $^ $(LDFLAGS)

%.o: %.c $(HDRS)
	$(CC) $(CFLAGS) -c -o $@ $<

.PHONY: clean

clean:
	rm -f zeroconf_lookup *.o

install: zeroconf_lookup
	mkdir -p "$(DESTDIR)$(prefix)/bin"
	install -v zeroconf_lookup "$(DESTDIR)$(prefix)/bin/zeroconf_lookup"
	"$(DESTDIR)$(prefix)/bin/zeroconf_lookup" -i

uninstall: zeroconf_lookup
	./zeroconf_lookup -u

