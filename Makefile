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

cssl.o:
	libtool compile gcc -Wall -D_GNU_SOURCE -g -O -c cssl.c

pos_decode.o:	pos_decode.c
	#libtool compile gcc -Wall -D_GNU_SOURCE -g -O -c test.c
	gcc -Wall -D_GNU_SOURCE -g -O -mfpu=vfp -mfloat-abi=hard -lcssl -c pos_decode.c

spaxxpos: pos_decode.o cssl.o spaxxpos.o
	gcc -g -O -o spaxxpos spaxxpos.o pos_decode.o cssl.o

spaxxpos.o:	spaxxpos.c
	gcc -Wall -D_GNU_SOURCE -g -O -mfpu=vfp -mfloat-abi=hard -lcssl -c spaxxpos.c

pos_decode_lib: pos_decode.o cssl.o 
	gcc -g -O -shared -o pos_decode.so pos_decode.o cssl.o 

clean:
	rm -fr .libs pos_decode *.la *.lo *.o *.so
