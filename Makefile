MMCU=atmega328p
SMCU=$(MMCU)
F_CPU = 20000000
BAUDRATE?=9600
CC=avr-c++
DEFINE=-DF_CPU=$(F_CPU) -DBAUDRATE=$(BAUDRATE) -DORNITHOPTER
INCLUDE=-I./libraries/Arduino/hardware/arduino/avr/libraries/SPI/src/ -I./libraries/RF24/
CFLAGS=-mmcu=$(MMCU) $(DEFINE) -Wall -Os -std=c++11 -Wno-write-strings -fdata-sections -ffunction-sections -Wl,-gc-sections
LFLAGS=
LFUSES=0x5E

build/radio.out: test/radio.cc
	$(CC) $(CFLAGS) $(DEFINE) -I./ $(INCLUDE) -o $@ $< ./libraries/Arduino/hardware/arduino/avr/libraries/SPI/src/SPI.cpp ./libraries/RF24/RF24.cpp $(LFLAGS)

build/%.out: test/%.cc
	$(CC) $(CFLAGS) $(DEFINE) -I./ -o $@ $< $(LFLAGS)

build/%.hex: build/%.out
	avr-objcopy -O ihex $< $@;\
	avr-size --mcu=$(MMCU) --format=avr $@

clean:
	rm -rf build/*

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

servos: build/servos.hex fuses
	avrdude -p $(SMCU) -P usb -c usbtiny -U flash:w:$<

accelerometer: build/accelerometer.hex fuses
	avrdude -p $(SMCU) -P usb -c usbtiny -U flash:w:$<

radio: build/radio.hex fuses
	avrdude -p $(SMCU) -P usb -c usbtiny -U flash:w:$<
