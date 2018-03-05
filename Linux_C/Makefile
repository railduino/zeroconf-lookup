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

VERSION := 2.4.2

HDRS := $(wildcard *.h)
OBJS := $(patsubst %.c,%.o,$(wildcard *.c))

CFLAGS  += -W -Wall -Wextra -Wshadow -Wstrict-prototypes -Wpointer-arith -Wcast-qual -Winline -Werror
LDFLAGS += -lavahi-client -lavahi-common

all: zeroconf_lookup

zeroconf_lookup: $(OBJS)
	$(CC) -o $@ $^ $(LDFLAGS)

%.o: %.c $(HDRS)
	$(CC) $(CFLAGS) -c -o $@ $<

.PHONY: clean wipe tags variables deb rpm

clean:
	rm -f zeroconf_lookup *.o tags

wipe:
	rm -f zeroconf_lookup *.o tags *.deb *.rpm

tags:
	ctags *.[ch]

install: zeroconf_lookup variables
	./setup.sh install

uninstall: zeroconf_lookup variables
	./setup.sh uninstall

deb:
	sudo rm -rf /tmp/fpm_zeroconf_lookup
	./configure -p /usr
	make zeroconf_lookup
	sudo make install DESTDIR=/tmp/fpm_zeroconf_lookup
	fpm -s dir -t deb -f \
		-C /tmp/fpm_zeroconf_lookup \
		--name zeroconf_lookup \
		--version $(VERSION) \
		--iteration 1 \
		--description "Find HTTP Servers in the .local domain using Zeroconf" .
	sudo rm -rf /tmp/fpm_zeroconf_lookup

rpm:
	sudo rm -rf /tmp/fpm_zeroconf_lookup
	./configure -p /usr
	make zeroconf_lookup
	sudo make install DESTDIR=/tmp/fpm_zeroconf_lookup
	fpm -s dir -t rpm -f \
		-C /tmp/fpm_zeroconf_lookup \
		--name zeroconf_lookup \
		--version $(VERSION) \
		--iteration 1 \
		--description "Find HTTP Servers in the .local domain using Zeroconf" .
	sudo rm -rf /tmp/fpm_zeroconf_lookup

