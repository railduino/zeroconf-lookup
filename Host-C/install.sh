#!/bin/bash
# vim: set ts=8 tw=0 noet :
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

# Exit on error
set -e -o pipefail

# Remember the args
_prog=${0##*/}
_args="$*"
_name="Zeroconf-Lookup"


if [[ ! -r CMakeLists.txt ]] ; then
	echo "Oops! Missing CMakeLists.txt - can't build. Stop." >&2
fi


_user_id=$(id -u)
_op_sys=$(uname -s)
_user_dir="$HOME"
if [[ $_op_sys == "Linux" ]] ; then
	_root_dir="/usr"
	_cmake_gen="Unix Makefiles"
	_compile="make"
	_install="make install"
	_out_dir="."
elif [[ $_op_sys == "Darwin" ]] ; then
	_root_dir="/usr/local"
	_cmake_gen="Xcode"
	_compile="xcodebuild -configuration Release build"
	_install="xcodebuild -configuration Release install"
	_out_dir="./Release"
else
	echo "Error: unknown operating system $_op_sys" >&2
	exit 1
fi


function usage
{
	cat >&2 <<-EOF
		Syntax: $_prog [-u] [-d <dir>]
		Option: -u   Uninstall (default is install)
		        -d   (Un-)Install executable into <dir> (without /bin)
		             Default when UID == 0 : $_root_dir
		             Default when UID != 0 : $_user_dir
		To install system wide use sudo or su

	EOF

	exit 1
}


while getopts ":d:u" opt ; do
	case $opt in
		d)
			if [[ $_user_id -eq 0 ]] ; then
				_root_dir="$OPTARG"
			else
				_user_dir="$OPTARG"
			fi
		;;

		u)
			_uninst=1
		;;

		\?)

			usage
		;;
	esac
done


if [[ $_uninst ]] ; then
	_manifest="com.railduino.zeroconf_lookup.json"
	_program="zeroconf_lookup"

	if [[ $_op_sys == "Linux" ]] ; then
		for _dir in "/usr/lib/mozilla/native-messaging-hosts" \
			    "/etc/opt/chrome/native-messaging-hosts" \
			    "/etc/chromium/native-messaging-hosts" ; do
			_path="$_dir/$_manifest"
			if [[ -r "$_path" ]] ; then
				echo "Deleting $_path"
				sudo rm "$_path"
			fi
		done

		for _dir in "~/.mozilla/native-messaging-hosts" \
			    "~/.config/google-chrome/NativeMessagingHosts" \
			    "~/.config/chromium/NativeMessagingHosts" ; do
			_path="$_dir/$_manifest"
			if [[ -r "$_path" ]] ; then
				echo "Deleting $_path"
				rm "$_path"
			fi
		done
	fi

	if [[ $_op_sys == "Darwin" ]] ; then
		for _dir in "/Library/Application Support/Mozilla/NativeMessagingHosts" \
			    "/Library/Google/Chrome/NativeMessagingHosts" \
			    "/Library/Application Support/Chromium/NativeMessagingHosts" ; do
			_path="$_dir/$_manifest"
			if [[ -r "$_path" ]] ; then
				echo "Deleting $_path"
				sudo rm "$_path"
			fi
		done

		for _dir in "~/Library/Application Support/Mozilla/NativeMessagingHosts" \
			    "~/Library/Application Support/Google/Chrome/NativeMessagingHosts" \
			    "~/Library/Application Support/Chromium/NativeMessagingHosts" ; do
			_path="$_dir/$_manifest"
			if [[ -r "$_path" ]] ; then
				echo "Deleting $_path"
				rm "$_path"
			fi
		done
	fi

	_path="$_root_dir/bin/zeroconf_lookup"
	if [[ -x "$_path" ]] ; then
		echo "Deleting $_path"
		sudo rm "$_path"
	fi

	_path="$_user_dir/bin/zeroconf_lookup"
	if [[ -x "$_path" ]] ; then
		echo "Deleting $_path"
		rm "$_path"
	fi

	echo "All done. Exit."
	exit 0
fi


if  [[ $_user_id -eq 0 ]] ; then
	_exec_dir="$_root_dir"
else
	_exec_dir="$_user_dir"
fi


cat <<-EOF
	Operating system is .......: $_op_sys
	My User ID is .............: $_user_id ($(id -u -n))
	Executable directory is ...: $_exec_dir/bin
	CMake generator is ........: $_cmake_gen

EOF
read -p "Press ^C to cancel or Enter to continue " _input


echo ""
echo "###########################################################"
echo "###########################################################"
echo "##############  Build executable with CMake  ##############"
echo "###########################################################"
echo "###########################################################"
echo ""

rm -rf build 2>/dev/null || sudo rm -rf build
mkdir build
cd build
cmake -DCMAKE_INSTALL_PREFIX:PATH="$_exec_dir" .. -G "$_cmake_gen"
$_compile


echo ""
echo "###########################################################"
echo "###########################################################"
echo "##############  Uninstall current Manifests  ##############"
echo "###########################################################"
echo "###########################################################"
echo ""

# we are still in build
$_out_dir/zeroconf_lookup -u
sudo $_out_dir/zeroconf_lookup -u


echo ""
echo "###########################################################"
echo "###########################################################"
echo "##############  Install executable  #######################"
echo "###########################################################"
echo "###########################################################"
echo ""

$_install


echo ""
echo "###########################################################"
echo "###########################################################"
echo "##############  Install Firefox / Chrome Manifests ########"
echo "###########################################################"
echo "###########################################################"
echo ""

$_exec_dir/bin/zeroconf_lookup -i


echo "All done. Exit."
exit 0
