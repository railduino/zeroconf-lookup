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

all: zeroconf_lookup

zeroconf_lookup: main.go
	go get -u github.com/spf13/viper
	go get -u github.com/grandcat/zeroconf
	go build -o $@

clean:
	rm -f zeroconf_lookup

install: zeroconf_lookup
	install zeroconf_lookup /usr/bin
	/usr/bin/zeroconf_lookup -i

install-user: zeroconf_lookup
	install -d ~/bin
	install zeroconf_lookup ~/bin
	~/bin/zeroconf_lookup -i

uninstall:
	/usr/bin/zeroconf_lookup -u

uninstall-user:
	~/bin/zeroconf_lookup -u

