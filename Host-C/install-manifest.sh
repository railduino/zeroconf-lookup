#!/bin/bash
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

set -e

_extens="knldjmfmopnpolahpmmgbagdohdnhkik"
_jsonid="com.railduino.zeroconf_lookup"
_inform="Find HTTP Services in the .local domain using Zeroconf (mDNS-SD)"
_binary="zeroconf_lookup"

if [[ $1 == "local" ]] ; then
	mkdir -p $HOME/bin
	rm -f $HOME/bin/$_binary
	install -v -m 0755 $_binary $HOME/bin

	_dest=$HOME/.mozilla/native-messaging-hosts
	mkdir -p $_dest
	_file=$_dest/$_jsonid.json

	cat >$_file <<-_EOF_
		{
		  "name": "$_jsonid",
		  "description": "$_inform",
		  "path": "$HOME/bin/$_binary",
		  "type": "stdio",
		  "allowed_extensions": [ "zeroconf_lookup@railduino.com" ]
		}
	_EOF_
	chmod 0644 $_file
	echo "created $_file"

	for _dest in	$HOME/.config/google-chrome/NativeMessagingHosts \
			$HOME/.config/chromium/NativeMessagingHosts ; do
		mkdir -p $_dest
		_file=$_dest/$_jsonid.json

		cat >$_file <<-_EOF_
			{
			  "name": "$_jsonid",
			  "description": "$_inform",
			  "path": "$HOME/bin/$_binary",
			  "type": "stdio",
			  "allowed_origins": [ "chrome-extension://$_extens/" ]
			}
		_EOF_
		chmod 0644 $_file
		echo "created $_file"
	done

	exit 0
fi

if [[ $1 == "system" ]] ; then
	rm -f /usr/bin/$_binary
	install -v -o root -g root -m 0755 $_binary /usr/bin

	_dest=/usr/lib/mozilla/native-messaging-hosts
	mkdir -p $_dest
	_file=$_dest/$_jsonid.json

	cat >$_file <<-_EOF_
		{
		  "name": "$_jsonid",
		  "description": "$_inform",
		  "path": "/usr/bin/$_binary",
		  "type": "stdio",
		  "allowed_extensions": [ "zeroconf_lookup@railduino.com" ]
		}
	_EOF_
	chmod 0644 $_file
	echo "created $_file"

	for _dest in	/etc/opt/chrome/native-messaging-hosts \
			/etc/chromium/native-messaging-hosts ; do
		mkdir -p $_dest
		_file=$_dest/$_jsonid.json

		cat >$_file <<-_EOF_
			{
			  "name": "$_jsonid",
			  "description": "$_inform",
			  "path": "/usr/bin/$_binary",
			  "type": "stdio",
			  "allowed_origins": [ "chrome-extension://$_extens/" ]
			}
		_EOF_
		chmod 0644 $_file
		echo "created $_file"
	done

	exit 0
fi

