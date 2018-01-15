#
# Written by Volker Wiegand <volker@railduino.de>
#
# See https://github.com/railduino/zeroconf-lookup
#

all: README.md

README.md: README.odt
	pandoc -f odt -t markdown -o $@ $<

chrome:
	cd Chrome && zip -r -FS ../railduino-zeroconf-lookup.zip *

firefox:
	cd Firefox && zip -r -FS ../railduino-zeroconf-lookup.xpi *

