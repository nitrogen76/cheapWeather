CC=gcc
LD=$(CC)

LIBUSB_CFLAGS=`pkg-config libusb-1.0 --cflags`
LIBUSB_LDFLAGS=`pkg-config libusb-1.0 --libs`

CFLAGS=-Wall -g -O  -I../common -fpic $(LIBUSB_CFLAGS)
LDFLAGS=-lm $(LIBUSB_LDFLAGS)

PREFIX=/usr/local

PROGS=usbtenkiget usbtenkiset

.PHONY : clean install

all: libtenki.so

libtenki.so: usbtenki.o convertRaw.o convert_type_k.o
	$(LD) -shared $(LDFLAGS) $^ -o $@

%.o: %.c %.h
	$(CC) $(CFLAGS) -c $<

clean:
	rm -f *.o $(PROGS)

install:
	@echo "Install not done yet. Sorry"

javah:
	$(JAVA_HOME)/bin/javah -classpath ../java/jusbtenki.jar com.raphnet.tenki.JNI
