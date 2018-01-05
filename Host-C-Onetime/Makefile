#
# Written by Volker Wiegand <volker@railduino.de>
#
# License: See https://github.com/volkerwiegand/zeroconf-lookup/blob/master/LICENSE
#

CFLAGS = -g -O0

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
	@./zeroconf_lookup -j -v -d

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

