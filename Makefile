#
# Written by Volker Wiegand <volker@railduino.de>
#
# License: See https://github.com/volkerwiegand/zeroconf-lookup/blob/master/LICENSE
#

all: README.md

README.md: README.odt
	pandoc -f odt -t markdown -o $@ $<

firefox:
	cd Firefox && zip -r -FS ../railduino-zeroconf-lookup.xpi *

