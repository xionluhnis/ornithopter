MMCU=atmega328p
SMCU=$(MMCU)
F_CPU = 8000000
BAUDRATE=9600
CC=avr-c++

DEFINE=-DF_CPU=$(F_CPU) -DBAUDRATE=$(BAUDRATE)
CFLAGS=-mmcu=$(MMCU) $(DEFINE) -Wall -Os -std=c++11 -Wno-write-strings -fdata-sections -ffunction-sections -Wl,-gc-sections
LFLAGS=

build/%.out: test/%.cc
	$(CC) $(CFLAGS) -I./ -o $@ $< $(LFLAGS)

build/%.hex: build/%.out
	avr-objcopy -O ihex $< $@;\
	avr-size --mcu=$(MMCU) --format=avr $@

fuses:
	avrdude -p $(SMCU) -P usb -c usbtiny -U lfuse:w:0xE2:m

led: build/led.hex fuses
	avrdude -p $(SMCU) -P usb -c usbtiny -U flash:w:$<

helloworld: build/helloworld.hex fuses
	avrdude -p $(SMCU) -P usb -c usbtiny -U flash:w:$<
