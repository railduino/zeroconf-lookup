#
# Written by Volker Wiegand <volker@railduino.de>
#
# License: See https://github.com/volkerwiegand/zeroconf-lookup/blob/master/LICENSE
#

doc:
	pandoc -f odt -t markdown -o README.md README.odt

firefox:
	cd Firefox && zip -r -FS ../railduino-zeroconf-lookup.zip *

