#!/bin/bash
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

set -e

source ./variables

if [[ $1 == "install" ]] ; then
	if [[ -n $DESTDIR ]] ; then
		while [[ $DESTDIR =~ /$ ]] ; do
			DESTDIR=${DESTDIR%/}
		done
		echo "[setup] DESTDIR = $DESTDIR"
		bindir=$(echo "$DESTDIR/$bindir" | sed -e 's#//*#/#g')
		sysconfdir=$(echo "$DESTDIR/$sysconfdir" | sed -e 's#//*#/#g')
	fi
	mkdir -v -p "$bindir"
	install -v zeroconf_lookup "$bindir"
	mkdir -v -p "$sysconfdir"
	"$bindir/zeroconf_lookup" -i
	exit 0
fi

if [[ $1 == "uninstall" ]] ; then
	./zeroconf_lookup -u
	if [[ -x "$bindir/zeroconf_lookup" ]] ; then
		rm -f -v "$bindir/zeroconf_lookup"
	fi
	if [[ -x "$sysconfdir/zeroconf_lookup.conf" ]] ; then
		rm -f -v "$sysconfdir/zeroconf_lookup.conf"
	fi
	exit 0
fi

echo "Usage: $0 install|uninstall" >&2
exit 1

