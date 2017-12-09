MMCU=atmega328p
SMCU=$(MMCU)
F_CPU = 20000000
BAUDRATE=9600
CC=avr-c++

DEFINE=-DF_CPU=$(F_CPU) -DBAUDRATE=$(BAUDRATE)
CFLAGS=-mmcu=$(MMCU) $(DEFINE) -Wall -Os -std=c++11 -Wno-write-strings -fdata-sections -ffunction-sections -Wl,-gc-sections
LFLAGS=

helloworld.out: test/helloworld.cc
	$(CC) $(CFLAGS) -I./ -o build/helloworld.out test/helloworld.cc $(LFLAGS)

helloworld.hex: helloworld.out
	avr-objcopy -O ihex build/helloworld.out build/helloworld.hex;\
	avr-size --mcu=$(MMCU) --format=avr build/helloworld.out

helloworld.fuses:
	avrdude -p $(SMCU) -P usb -c usbtiny -U lfuse:w:0x5E:m

helloworld: helloworld.hex helloworld.fuses
	avrdude -p $(SMCU) -P usb -c usbtiny -U flash:w:build/helloworld.hex