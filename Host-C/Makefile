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
  CFLAGS += -W -Wall -Wextra -Wshadow -Wstrict-prototypes -Wpointer-arith -Wcast-qual -Winline -Werror

  UNAME_S := $(shell uname -s)
  ifeq ($(UNAME_S),Linux)
    CFLAGS += $(shell pkg-config --cflags avahi-client)
  endif
  ifeq ($(UNAME_S),Linux)
    LDFLAGS += $(shell pkg-config --libs avahi-client)
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
	@./zeroconf_lookup -j

debug: zeroconf_lookup
	ZEROCONF_LOOKUP="verbose=1" ./zeroconf_lookup -j

deep: zeroconf_lookup
	ZEROCONF_LOOKUP="verbose=9" ./zeroconf_lookup -j

local: zeroconf_lookup
	./install-manifest.sh local

system: zeroconf_lookup
	sudo ./install-manifest.sh system

log: zeroconf_lookup
	tail -f /tmp/zeroconf_lookup.log

uninstall:
	rm -f ~/.mozilla/native-messaging-hosts/com.railduino.chrome.zeroconf_lookup.json
	rm -f ~/.config/google-chrome/NativeMessagingHosts/com.railduino.chrome.zeroconf_lookup.json
	rm -f ~/.config/chromium/NativeMessagingHosts/com.railduino.chrome.zeroconf_lookup.json
	rm -f ~/bin/zeroconf_lookup
	sudo rm -f /usr/lib/mozilla/native-messaging-hosts/com.railduino.chrome.zeroconf_lookup.json
	sudo rm -f /etc/opt/chrome/native-messaging-hosts/com.railduino.chrome.zeroconf_lookup.json
	sudo rm -f /etc/chromium/native-messaging-hosts/com.railduino.chrome.zeroconf_lookup.json
	sudo rm -f /usr/bin/zeroconf_lookup
	rm -f ~/bin/zeroconf_lookup.conf ~/zeroconf_lookup.conf
	sudo rm -f /etc/zeroconf_lookup.conf

