#
# Copyright (c) 2017-2018 Volker Wiegand <volker@railduino.de>
#
# This file is part of Zeroconf-Lookup.
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

ifeq ($(OS),Windows_NT)
  CCFLAGS += -D__windows__
else
  CFLAGS += -g -W -Wall -Wextra -Wshadow -Wstrict-prototypes -Wunused-result -Wpointer-arith -Wcast-qual -Winline -Werror
  UNAME_S := $(shell uname -s)
  ifeq ($(UNAME_S),Linux)
    CFLAGS  += $(shell pkg-config --cflags avahi-client)
    LDFLAGS += $(shell pkg-config --libs   avahi-client)
  endif
endif

HDRS := $(wildcard *.h)
OBJS := $(patsubst %.c,%.o,$(wildcard *.c))

zeroconf_lookup: $(OBJS)
	ctags *.[ch]
	gcc -o $@ $^ $(LDFLAGS)

%.o: %.c $(HDRS)
	$(CC) $(CFLAGS) -c -o $@ $<

.PHONY: clean tags uninstall

clean:
	rm -f zeroconf_lookup *.o tags

tags:
	ctags *.[ch]

run: zeroconf_lookup
	./zeroconf_lookup -j

debug: zeroconf_lookup
	./zeroconf_lookup -jv

chatty: zeroconf_lookup
	./zeroconf_lookup -jvvvvvv

local: zeroconf_lookup
	./install-manifest.sh local

system: zeroconf_lookup
	sudo ./install-manifest.sh system

uninstall:
	./install-manifest.sh purge

log: zeroconf_lookup
	tail -f /tmp/zeroconf_lookup.log

