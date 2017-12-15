MMCU=atmega328p
SMCU=$(MMCU)
F_CPU = 20000000
BAUDRATE?=9600
CC=avr-c++
DEFINE=-DF_CPU=$(F_CPU) -DBAUDRATE=$(BAUDRATE) -DORNITHOPTER
INCLUDE=-I./libraries/Arduino/hardware/arduino/avr/libraries/SPI/src/ -I./libraries/RF24/
CFLAGS=-mmcu=$(MMCU) $(DEFINE) -Wall -Os -std=c++11 -Wno-write-strings -fdata-sections -ffunction-sections -Wl,-gc-sections -fshort-enums
LFLAGS=
LFUSES=0x5E
RADIO_CE?=PC0
RADIO_CSN?=PC1

SPI_CPP=./libraries/Arduino/hardware/arduino/avr/libraries/SPI/src/SPI.cpp
RF24_CPP=./libraries/RF24/RF24.cpp

build/car.out: src/car.cc
	$(CC) $(CFLAGS) $(DEFINE) -DRADIO_CE=$(RADIO_CE) -DRADIO_CSN=$(RADIO_CSN) -I./ -I./src/ $(INCLUDE) -o $@ $< $(SPI_CPP) $(RF24_CPP) $(LFLAGS)

build/serial.out: src/serial.cc
	$(CC) $(CFLAGS) $(DEFINE) -DRADIO_CE=$(RADIO_CE) -DRADIO_CSN=$(RADIO_CSN) -I./ -I./src/ $(INCLUDE) -o $@ $< $(SPI_CPP) $(RF24_CPP) $(LFLAGS)

build/radio.out: test/radio.cc
	@echo "Using pins RADIO_CE =" $(RADIO_CE) " and RADIO_CSN =" $(RADIO_CSN)
	$(CC) $(CFLAGS) $(DEFINE) -DRADIO_CE=$(RADIO_CE) -DRADIO_CSN=$(RADIO_CSN) -I./ $(INCLUDE) -o $@ $< $(SPI_CPP) $(RF24_CPP) $(LFLAGS)

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

dcmotor: build/dcmotor.hex fuses
	avrdude -p $(SMCU) -P usb -c usbtiny -U flash:w:$<

accelerometer: build/accelerometer.hex fuses
	avrdude -p $(SMCU) -P usb -c usbtiny -U flash:w:$<

radio: build/radio.hex fuses
	avrdude -p $(SMCU) -P usb -c usbtiny -U flash:w:$<

car: build/car.hex fuses
	avrdude -p $(SMCU) -P usb -c usbtiny -U flash:w:$<

serial: build/serial.hex fuses
	avrdude -p $(SMCU) -P usb -c usbtiny -U flash:w:$<
