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
EXEC_DIR ?= whatever
GENERATE ?= CodeBlocks
else
ifeq ($(shell uname),Linux)
EXEC_DIR ?= /usr
GENERATE ?= Unix Makefiles
else
ifeq ($(shell uname),Darwin)
EXEC_DIR ?= /usr/local
GENERATE ?= Xcode
else
unknown_system
endif
endif
endif


.PHONY: clean

build: clean
	mkdir -p build
	cd build && cmake -DCMAKE_INSTALL_PREFIX:PATH=$(EXEC_DIR) .. -G "$(GENERATE)"
	cd build && make
	@echo ""
	@echo "EXEC_DIR --> $(EXEC_DIR)"
	@echo "GENERATE --> $(GENERATE)"
	@echo ""

install-user: build
	mkdir -p $(EXEC_DIR)/bin
	cd build && make install
	cd build && sudo ./zeroconf_lookup -u
	$(EXEC_DIR)/bin/zeroconf_lookup -i

install-system: build
	sudo mkdir -p $(EXEC_DIR)/bin
	cd build && sudo make install
	cd build && ./zeroconf_lookup -u
	sudo $(EXEC_DIR)/bin/zeroconf_lookup -i

uninstall: build
	cd build && ./zeroconf_lookup -u
	cd build && sudo ./zeroconf_lookup -u
	sudo rm -f $(EXEC_DIR)/bin/zeroconf_lookup

clean:
	rm -rf build/[A-Za-z0-9]*

