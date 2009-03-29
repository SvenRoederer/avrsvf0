# avrsvf0
# Makefile
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; version 2 of the License.
# 
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
# 
# 10.02.2009 ASR  First version
#

avrsvf0: main.c config.c config.h hexreader.c hexreader.h
	gcc -o avrsvf0 main.c config.c hexreader.c

test:
	./avrsvf0 -datmega16 -s -e -ifags3_altdeserial_sw.hex -pf -vf -F -ofoobar1 -mp -f0xff89ff
	./avrsvf0 -datmega128 -chd1 -ctd0 -chi8 -cti0 -s -e -ifags3_driverboard3_sw.hex -pf -vf -F -ofoobar2 -mp -f0xff89ff

clean:
	rm avrsvf0

