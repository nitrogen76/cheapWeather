CC=gcc
LD=$(CC)

#LIBUSB_CFLAGS=-DUSE_OLD_LIBUSB
#LIBUSB_LDFLAGS=-lusb -L/usr/local/opt/libusb-compat/lib

LIBUSB_CFLAGS=`pkg-config libusb-1.0 --cflags`
LIBUSB_LDFLAGS=`pkg-config libusb-1.0 --libs`

USBTENKI_OBJS=usbtenki.o convertRaw.o convert_type_k.o

CFLAGS=-Wall -g -O -I../common -I../library $(LIBUSB_CFLAGS)
LDFLAGS=-lm `pkg-config libusb-1.0 --libs`

vpath %.c ../library
vpath %.h ../library

PREFIX=/usr/local

PROGS=usbtenkiget usbtenkiset

.PHONY : clean install

all: usbtenkiget usbtenkiset

usbtenkiget: $(USBTENKI_OBJS) main.o timestamp.o
	$(LD) $^ -o $@ $(LDFLAGS)

usbtenkiset: $(USBTENKI_OBJS) usbtenkiset.o
	$(LD) $^ -o $@ $(LDFLAGS)

%.o: %.c %.h
	$(CC) $(CFLAGS) -c $<

clean:
	rm -f *.o $(PROGS)

install:
	@echo "Install not done yet. Sorry"

