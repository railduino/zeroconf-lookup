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

_inform="Find HTTP Servers in the .local domain using Zeroconf"
_binary="zeroconf_lookup"
_extens="eajgigammocepkmcgfcicpeljokgcchh"
_jsonid="com.railduino.zeroconf_lookup"

_camel="NativeMessagingHosts"
_dash="native-messaging-hosts"

_firefox_local="$HOME/.mozilla/$_dash"
_firefox_system="/usr/lib/mozilla/$_dash"

_chrome_local="$HOME/.config/google-chrome/$_camel $HOME/.config/chromium/$_camel"
_chrome_system="/etc/opt/chrome/$_dash /etc/chromium/$_dash"


#
# Call 'purge' as user - mixed mode
#
if [[ $1 == "purge" ]] ; then
	for _dest in $_firefox_local $_chrome_local ; do
		rm -f $_dest/$_jsonid.json
	done
	rm -f $HOME/bin/zeroconf_lookup
	rm -f $HOME/bin/zeroconf_lookup.conf
	rm -f $HOME/zeroconf_lookup.conf

	for _dest in $_firefox_system $_chrome_system ; do
		sudo rm -f $_dest/$_jsonid.json
	done
	sudo rm -f /usr/bin/zeroconf_lookup
	sudo rm -f /usr/bin/zeroconf_lookup.conf
	sudo rm -f /etc/zeroconf_lookup.conf

	exit 0
fi


#
# Call 'local' as user - avoid root
#
if [[ $1 == "local" ]] ; then
	mkdir -p $HOME/bin
	rm -f $HOME/bin/$_binary
	install -v -m 0755 $_binary $HOME/bin

	mkdir -p $_firefox_local
	_file=$_firefox_local/$_jsonid.json
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

	for _dest in $_chrome_local ; do
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


#
# Call 'system' as root - no sudo inside
#
if [[ $1 == "system" ]] ; then
	rm -f /usr/bin/$_binary
	install -v -o root -g root -m 0755 $_binary /usr/bin

	mkdir -p $_firefox_system
	_file=$_firefox_system/$_jsonid.json
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

	for _dest in $_chrome_system ; do
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

cat >&2 <<-_EOF_
	Usage: $0 purge | local | system
	    purge  --- remove all binaries and manifests (sudo inside)
	    local  --- install in user's home directory (without root)
	    system --- install in /usr and /opt (no sudo inside, call as root)
_EOF_

exit 1

