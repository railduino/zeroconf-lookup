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

UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Linux)
  BINDIR = /usr/bin
endif
ifeq ($(UNAME_S),Darwin)
  BINDIR = /usr/local/bin
endif

all: zeroconf_lookup

zeroconf_lookup: main.go
	go build -o $@

.PHONY: update clean tags uninstall

update:
	go get -u github.com/google/go-cmp/cmp
	go get -u github.com/grandcat/zeroconf
	go get -u github.com/spf13/viper

clean:
	rm -f zeroconf_lookup tags

tags:
	ctags *.[ch]

run: zeroconf_lookup
	./zeroconf_lookup -r

debug: zeroconf_lookup
	rm -f /tmp/zeroconf_lookup.log
	-./zeroconf_lookup -r -v
	cat /tmp/zeroconf_lookup.log

deep: zeroconf_lookup
	./zeroconf_lookup -r -v -v -v -v -v -v

install: zeroconf_lookup
	mkdir -p $(BINDIR)
	install zeroconf_lookup $(BINDIR)
	$(BINDIR)/zeroconf_lookup -i

user-install: zeroconf_lookup
	mkdir -p $(HOME)/bin
	install zeroconf_lookup $(HOME)/bin
	$(HOME)/bin/zeroconf_lookup -i

uninstall: zeroconf_lookup
	./zeroconf_lookup -u
	rm -f $(BINDIR)/zeroconf_lookup

user-uninstall: zeroconf_lookup
	./zeroconf_lookup -u
	rm -f $(HOME)/bin/zeroconf_lookup

log: zeroconf_lookup
	tail -f /tmp/zeroconf_lookup.log

