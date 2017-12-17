// Alexandre Kaspar <akaspar@mit.edu>
#pragma once

#include <util/delay.h>
#include <stdint.h>
#include "RF24.h"
#include "usart.h"

#ifndef RADIO_NUM
#pragma error "You must define RADIO_NUM to either 0 (serial) or 1 (target)"
#endif

#ifndef RADIO_CE
#define RADIO_CE PC0
#endif
#ifndef RADIO_CSN
#define RADIO_CSN PC1
#endif

const unsigned char addresses[][6] = {"CTerm","Robot"};
RF24 radio(RADIO_CE, RADIO_CSN);

void radio_init(bool usart = false){
  // radio setup

  if(usart)
    USART_send("RF24 / Radio\r\n");
  while(!radio.begin()){
    USART_send("Initialization error\r\n");
    _delay_ms(3000);
  }

  // Set the PA Level low to prevent power supply related issues since this is a
  // getting_started sketch, and the likelihood of close proximity of the devices. RF24_PA_MAX is default.
  radio.setPALevel(RF24_PA_LOW);

  // Open a writing and reading pipe on each radio, with opposite addresses
  radio.openWritingPipe(addresses[RADIO_NUM]);
  radio.openReadingPipe(1, addresses[1 - RADIO_NUM]);
  
  // Start the radio listening for data
  radio.startListening();
  USART_send("Listening\r\n");
}
