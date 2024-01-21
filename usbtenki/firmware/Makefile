# Name: Makefile
# Project: PowerSwitch
# Author: Christian Starkjohann
# Creation Date: 2004-12-29
# Tabsize: 4
# Copyright: (c) 2005 by OBJECTIVE DEVELOPMENT Software GmbH
# License: Proprietary, free under certain conditions. See Documentation.
# This Revision: $Id: Makefile,v 1.28 2014-08-23 20:47:56 cvs Exp $

SERIAL = /dev/avr
UISP = uisp -dprog=stk500 -dserial=$(SERIAL) -dpart=ATmega8
VERIFY = --verify
FUSE_H=0xc1
FUSE_L=0x1f
# The two lines above are for "uisp" and the AVR910 serial programmer connected
# to a Keyspan USB to serial converter to a Mac running Mac OS X.
# Choose your favorite programmer and interface.

SERNUMBER=

COMPILE = avr-gcc -Wall -Os -Iusbdrv -I. -mmcu=atmega8 -I../common -DF_CPU=12000000UL

COMMON_OBJECTS = usbdrv/usbdrv.o usbdrv/usbdrvasm.o serno.o i2c.o main.o eeprom.o adc.o
SENSOR_OBJECTS = mcp9800.o mcp9800.o lm75.o sser.o tsl256x.o chipcap2.o
# Note that we link usbdrv.o first! This is required for correct alignment of
# driver-internal global variables!

# symbolic targets:
all: mcp9800_00.hex mcp9800.hex lm75.hex sensirion.hex adc.hex tsl2561.hex tsl2568.hex bs02.hex sensirion_pc23.hex se95.hex adt7410.hex tachometer.hex pt100_rtd.hex mlx90614.hex cc2.hex

%.o: %.c
	$(COMPILE) -c $< -o $@

%.o: %.c %.h
	$(COMPILE) -c $< -o $@

.S.o:
	$(COMPILE) -x assembler-with-cpp -c $< -o $@
# "-x assembler-with-cpp" should not be necessary since this is the default
# file type for the .S (with capital S) extension. However, upper case
# characters are not always preserved on Windows. To ensure WinAVR
# compatibility define the file type manually.

.c.s:
	$(COMPILE) -S $< -o $@

flash:
	echo "flash_sensirion | flash_lm75 | flash_mcp9800 | flash_adc | flash_tsl2561 | flash_bs02 ?"

flash_sensirion:	sensirion.hex
	$(UISP) --erase --upload $(VERIFY) if=sensirion.hex

flash_mcp9800:	mcp9800.hex
	$(UISP) --erase --upload $(VERIFY) if=mcp9800.hex

flash_mcp9800_usb: mcp9800.hex
	avrdude -p m8 -P usb -c avrispmkII -Uflash:w:$< -B 1.0

flash_mcp9800_00: mcp9800_00.hex
	$(UISP) --erase --upload $(VERIFY) if=mcp9800_00.hex

flash_mcp9800_00_usb: mcp9800_00.hex
	avrdude -p m8 -P usb -c avrispmkII -Uflash:w:$< -B 1.0

flash_se95:	se95.hex
	$(UISP) --erase --upload $(VERIFY) if=$<

flash_se95_usb: se95.hex
	avrdude -p m8 -P usb -c avrispmkII -Uflash:w:$< -B 1.0

flash_adt7410: adt7410.hex
	$(UISP) --erase --upload $(VERIFY) if=$<

flash_adt7410_usb: adt7410.hex
	avrdude -p m8 -P usb -c avrispmkII -Uflash:w:$< -B 1.0

flash_lm75: lm75.hex
	$(UISP) --erase --upload $(VERIFY) if=lm75.hex

flash_lm75_usb: lm75.hex
	avrdude -p m8 -P usb -c avrispmkII -Uflash:w:$< -B 1.0

flash_adc: adc.hex
	$(UISP) --erase --upload $(VERIFY) if=adc.hex

flash_adc_usb: adc.hex
	avrdude -p m8 -P usb -c avrispmkII -Uflash:w:$< -B 1.0

flash_tsl2561: tsl2561.hex
	$(UISP) --erase --upload $(VERIFY) if=tsl2561.hex

flash_tsl2561_usb: tsl2561.hex
	avrdude -p m8 -P usb -c avrispmkII -Uflash:w:$< -B 1.0

flash_tsl2568: tsl2568.hex
	$(UISP) --erase --upload $(VERIFY) if=tsl2568.hex

flash_tsl2568_usb: tsl2568.hex
	avrdude -p m8 -P usb -c avrispmkII -Uflash:w:$< -B 1.0

flash_bs02: bs02.hex
	$(UISP) --erase --upload $(VERIFY) if=bs02.hex

flash_sensirion_pc23: sensirion_pc23.hex
	$(UISP) --erase --upload $(VERIFY) if=sensirion_pc23.hex

flash_tachometer: tachometer.hex
	$(UISP) --erase --upload $(VERIFY) if=tachometer.hex

flash_tachometer_usb: tachometer.hex
	avrdude -p m8 -P usb -c avrispmkII -Uflash:w:$< -B 1.0

flash_pt100_rtd_usb: pt100_rtd.hex
	avrdude -p m8 -P usb -c avrispmkII -Uflash:w:$< -B 1.0

flash_mlx90614: mlx90614.hex
	avrdude -p m8 -P usb -c avrispmkII -Uflash:w:$< -B 1.0

flash_cc2: cc2.hex
	avrdude -p m8 -P usb -c avrispmkII -Uflash:w:$< -B 1.0

#
# Fuse high byte = 0xC1
#  | RSTDISBL | WDTON | SPIEN | CKOPT | EESAVE | BOOTSZ1-0 | BOOTRST |
#  |    1         1       0       0       0       0    0        1
#
# EESAVE: Eeprom not erased when doing a chip erase (flashing)!
#
# Fuse low byte = 0x1f
#  | BODLEVEL | BODEN | SUT1 | SUT0 | CKSEL3   2   1   0   |
#  |    0         0      0       1      1      1   1   1   |
#       
# BODLEVEL: Brownout at 4 volts.
#
fuse:
	$(UISP) --wr_fuse_h=$(FUSE_H) --wr_fuse_l=$(FUSE_L)

fuse_usb:
	avrdude -p m8 -P usb -c avrispmkII -Uhfuse:w:$(FUSE_H):m -Ulfuse:w:$(FUSE_L):m -B 10.0

reset:
	avrdude -p m8 -P usb -c avrispmkII -B 10.0

clean:
	rm -f *.hex *.elf *.lst *.o usbdrv/*.o usbdrv/usbdrv.s

# file targets:
mcp9800.elf: $(COMMON_OBJECTS) mcp9800.o interface_mcp9800.o
	$(COMPILE) -o $@ $^

mcp9800_00.elf: $(COMMON_OBJECTS) mcp9800.o interface_mcp9800_00.o
	$(COMPILE) -o $@ $^

lm75.elf: $(COMMON_OBJECTS) lm75.o interface_lm75.o
	$(COMPILE) -o $@ $^

sensirion.elf: $(COMMON_OBJECTS) sser.o interface_sensirion.o
	$(COMPILE) -o $@ $^

sensirion_pc23.elf: $(COMMON_OBJECTS) sser_pc2_pc3.o interface_sensirion.o
	$(COMPILE) -o $@ $^

adc.elf: $(COMMON_OBJECTS) interface_dummy.o
	$(COMPILE) -o $@ $^ 

tsl2568.elf: $(COMMON_OBJECTS) interface_tsl256x.o interface_tsl2568_channels.o tsl256x.o
	$(COMPILE) -o $@ $^ 

tsl2561.elf: $(COMMON_OBJECTS) interface_tsl256x.o interface_tsl2561_channels.o tsl256x.o
	$(COMPILE) -o $@ $^ 

bs02.elf: $(COMMON_OBJECTS) interface_bs02.o sser.o
	$(COMPILE) -o $@ $^

se95.elf: $(COMMON_OBJECTS) interface_se95.o mcp9800.o
	$(COMPILE) -o $@ $^

adt7410.elf: $(COMMON_OBJECTS) interface_adt7410.o mcp9800.o
	$(COMPILE) -o $@ $^

tachometer.elf: $(COMMON_OBJECTS) interface_int1_tachometer.o
	$(COMPILE) -o $@ $^

pt100_rtd.elf: $(COMMON_OBJECTS) interface_pt100_rtd.o mcp3423.o
	$(COMPILE) -o $@ $^

mlx90614.elf: $(COMMON_OBJECTS) interface_mlx90614.o mlx90614.o
	$(COMPILE) -o $@ $^

cc2.elf: $(COMMON_OBJECTS) chipcap2.o interface_cc2.o
	$(COMPILE) -o $@ $^

%.hex: %.elf
	rm -f $@
	avr-objcopy -j .text -j .data -O ihex $< $@
	./checksize $<

#main.hex:	main.elf
#	rm -f main.hex main.eep.hex
#	avr-objcopy -j .text -j .data -O ihex main.elf main.hex
#	./checksize main.elf
# do the checksize script as our last action to allow successful compilation
# on Windows with WinAVR where the Unix commands will fail.

#cpp:
#	$(COMPILE) -E main.c
