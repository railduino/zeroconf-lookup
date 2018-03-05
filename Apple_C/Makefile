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

.PHONY: clean wipe

clean:
	rm -rf _build _install zeroconf_lookup-$(VERSION)

wipe:
	rm -rf _build _install zeroconf_lookup*

build: clean
	mkdir -p _build
	cd _build && cmake ..
	cd _build && make

xcode: clean
	mkdir -p _build
	cd _build && cmake .. -G Xcode

pkg: clean
	mkdir -p _build
	cd _build && cmake .. -DCMAKE_INSTALL_PREFIX=../_install/usr/local
	cd _build && make
	cd _build && make install

tgz: clean
	mkdir -p _build
	cd _build && cmake ..
	rm -rf zeroconf_lookup-$(VERSION)
	mkdir zeroconf_lookup-$(VERSION)
	cp src/* _build/config.h zeroconf_lookup-$(VERSION)
	tar -c -v -z -f zeroconf_lookup-$(VERSION).tgz zeroconf_lookup-$(VERSION)
	shasum -a 256 zeroconf_lookup-$(VERSION).tgz > zeroconf_lookup-$(VERSION).tgz.sha256
	rm -rf zeroconf_lookup-$(VERSION)

install: build
	mkdir -p _build
	cd _build && make install

sudo: build
	mkdir -p _build
	cd _build && sudo make install

