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

CFLAGS = -g -Wall -Werror -Wextra

HDRS := $(wildcard *.h)
OBJS := $(patsubst %.c,%.o,$(wildcard *.c))

zeroconf_lookup: $(OBJS)
	ctags *.[ch]
	gcc $(CFLAGS) -o $@ $^

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
	ZEROCONF_LOOKUP="debug=1" ./zeroconf_lookup -j

mdnssd: zeroconf_lookup
	ZEROCONF_LOOKUP="debug=1;avahi=0" ./zeroconf_lookup -j

user: zeroconf_lookup
	./install.sh user

root: zeroconf_lookup
	sudo ./install.sh root

log: zeroconf_lookup
	tail -f /var/log/syslog

uninstall:
	rm -f ~/.config/google-chrome/NativeMessagingHosts/com.railduino.chrome.zeroconf_lookup.json
	rm -f ~/.config/chromium/NativeMessagingHosts/com.railduino.chrome.zeroconf_lookup.json
	rm -f ~/bin/zeroconf_lookup
	sudo rm -f /etc/opt/chrome/native-messaging-hosts/com.railduino.chrome.zeroconf_lookup.json
	sudo rm -f /etc/chromium/native-messaging-hosts/com.railduino.chrome.zeroconf_lookup.json
	sudo rm -f /usr/bin/zeroconf_lookup
	rm -f ~/bin/zeroconf_lookup.conf ~/zeroconf_lookup.conf
	sudo rm -f /etc/zeroconf_lookup.conf

