MMCU=atmega328p
SMCU=$(MMCU)
F_CPU = 20000000
BAUDRATE?=9600
CC=avr-c++
DEFINE=-DF_CPU=$(F_CPU) -DBAUDRATE=$(BAUDRATE)
CFLAGS=-mmcu=$(MMCU) $(DEFINE) -Wall -Os -std=c++11 -Wno-write-strings -fdata-sections -ffunction-sections -Wl,-gc-sections
LFLAGS=
LFUSES=0x5E

build/%.out: test/%.cc
	$(CC) $(CFLAGS) $(DEFINE) -I./ -o $@ $< $(LFLAGS)

build/%.hex: build/%.out
	avr-objcopy -O ihex $< $@;\
	avr-size --mcu=$(MMCU) --format=avr $@

fuses:
	avrdude -p $(SMCU) -P usb -c usbtiny -U lfuse:w:$(LFUSES):m

led: build/led.hex fuses
	avrdude -p $(SMCU) -P usb -c usbtiny -U flash:w:$<

helloworld: build/helloworld.hex fuses
	avrdude -p $(SMCU) -P usb -c usbtiny -U flash:w:$<

echo: build/echo.hex fuses
	avrdude -p $(SMCU) -P usb -c usbtiny -U flash:w:$<

servo: build/servo.hex fuses
	avrdude -p $(SMCU) -P usb -c usbtiny -U flash:w:$<
