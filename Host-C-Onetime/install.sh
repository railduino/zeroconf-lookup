#!/bin/bash

set -e

_extens="knldjmfmopnpolahpmmgbagdohdnhkik"
_jsonid="com.railduino.zeroconf_lookup"
_inform="Find HTTP Services in the .local domain"
_binary="zeroconf_lookup"

if [[ $1 == "user" ]] ; then
	mkdir -p $HOME/bin
	rm -f $HOME/bin/$_binary
	install -v -m 0755 $_binary $HOME/bin

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

	_dest=$HOME/.mozilla/native-messaging-hosts
	mkdir -p $_dest
	_file=$_dest/$_jsonid.json

	cat >$_file <<-_EOF_
		{
		  "name": "$_jsonid",
		  "description": "$_inform",
		  "path": "$HOME/bin/$_binary",
		  "type": "stdio",
		  "allowed_extensions": [ "$_binary@railduino.com" ]
		}
	_EOF_
	chmod 0644 $_file
	echo "created $_file"

	exit 0
fi

if [[ $1 == "root" ]] ; then
	rm -f /usr/bin/$_binary
	install -o root -g root -m 0755 $_binary /usr/bin

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

	_dest=/usr/lib/mozilla/native-messaging-hosts
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

	exit 0
fi

