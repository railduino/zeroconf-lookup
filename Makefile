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

install-c-user: uninstall-all
	cd Host-C && make && make user-install

install-c-system: uninstall-all
	cd Host-C && make && sudo make install

install-go-user: uninstall-all
	cd Host-Go && make && make user-install

install-go-system: uninstall-all
	cd Host-Go && make && sudo make install

