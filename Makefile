# Copyright 2003 Marcin Siennicki <m.siennicki@cloos.pl>
# see COPYING file for details 

VERSION = 0
REVISION = 1
MINOR = 1

FULLVERSION = $(VERSION).$(REVISION).$(MINOR)

DISTNAME = libas5311-$(FULLVERSION)

SOURCEFILES = Makefile README INSTALL COPYING \
	pos_decode.c cssl.c cssl.h 

ifndef PREFIX
PREFIX = /usr
endif

LIBPATH = $(PREFIX)/lib
INCLUDEPATH = $(PREFIX)/include

all: spaxxpos pos_decode_lib

cssl.o:	cssl.c
	gcc  -std=gnu11 -Wall -D_GNU_SOURCE -g -O -c cssl.c

pos_decode.o:	pos_decode.c
	gcc  -std=gnu11 -Wall -D_GNU_SOURCE -g -O -mfpu=vfp -mfloat-abi=hard -lcssl -c pos_decode.c

spaxxpos: pos_decode.o cssl.o spaxxpos.o
	gcc  -std=gnu11 -g -O -o spaxxpos spaxxpos.o pos_decode.o cssl.o

spaxxpos.o:	spaxxpos.c
	gcc  -std=gnu11 -Wall -D_GNU_SOURCE -g -O -mfpu=vfp -mfloat-abi=hard -lcssl -c spaxxpos.c

pos_decode_lib: pos_decode.o cssl.o rpm_meter.o
	gcc -g -O -shared -o pos_decode.so pos_decode.o cssl.o rpm_meter.o -lpigpio

rpm_meter.o:	rpm_meter.c    
	gcc  -std=gnu11 -Wall -D_GNU_SOURCE -g -c rpm_meter.c -lpigpio -lrt
	#gcc -Wall -pthread -o test rpm_meter.c -lpigpio -lrt

rpm_meter_test:	rpm_meter.o  rpm_meter_test.c  
	gcc  -std=gnu11 -Wall -O -o rpm_meter_test rpm_meter_test.c rpm_meter.o -lpigpio -lm -lrt
    
clean:
	rm -fr .libs spaxxpos rpm_meter_test *.la *.lo *.o *.so
