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

LINUX_ROOT ?= /usr
APPLE_ROOT ?= /usr/local

.PHONY: clean Linux-build Apple-build

clean:
	rm -rf Linux/[A-Za-z0-9]* Apple/[A-Za-z0-9]*

Linux-build:
	mkdir -p Linux
	rm -rf Linux/[A-Za-z0-9]*
	cd Linux && cmake -DCMAKE_INSTALL_PREFIX:PATH=$(LINUX_ROOT) .. -G "Unix Makefiles"
	cd Linux && make
	@echo ""
	@echo "LINUX_ROOT --> $(LINUX_ROOT)"
	@echo ""

Linux-uninstall: Linux-build
	cd Linux && ./zeroconf_lookup -u && sudo ./zeroconf_lookup -u

Linux-install-user: Linux-build
	mkdir -p $(LINUX_ROOT)/bin
	cd Linux && make install
	$(LINUX_ROOT)/bin/zeroconf_lookup -i

Linux-install-system: Linux-build
	sudo mkdir -p $(LINUX_ROOT)/bin
	cd Linux && sudo make install
	sudo $(LINUX_ROOT)/bin/zeroconf_lookup -i

Apple-build:
	mkdir -p Apple
	rm -rf Apple/[A-Za-z0-9]*
	cd Apple && cmake -DCMAKE_INSTALL_PREFIX:PATH=$(APPLE_ROOT) .. -G "Unix Makefiles"
	cd Apple && make
	@echo ""
	@echo "APPLE_ROOT --> $(APPLE_ROOT)"
	@echo ""

Apple-uninstall: Apple-build
	cd Apple && ./zeroconf_lookup -u && sudo ./zeroconf_lookup -u

Apple-install-user: Apple-build
	mkdir -p $(APPLE_ROOT)/bin
	cd Apple && make install
	$(APPLE_ROOT)/bin/zeroconf_lookup -i

Apple-install-system: Apple-build
	sudo mkdir -p $(APPLE_ROOT)/bin
	cd Apple && sudo make install
	sudo $(APPLE_ROOT)/bin/zeroconf_lookup -i

