#
# Written by Volker Wiegand <volker@railduino.de>
#
# See https://github.com/railduino/zeroconf-lookup
#

all: README.md

README.md: README.odt
	pandoc -f odt -t markdown -o $@ $<

firefox:
	cd Firefox && zip -r -FS ../railduino-zeroconf-lookup.xpi *

uninstall-all:
	cd Host-C  && make && make user-uninstall && sudo make uninstall && make clean
	cd Host-Go && make && make user-uninstall && sudo make uninstall && make clean

