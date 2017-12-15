// Alexandre Kaspar <akaspar@mit.edu>
#pragma once

#include <util/delay.h>
#include <stdint.h>
#include "RF24.h"
#include "serial.h"

#ifndef RADIO_CE
#define RADIO_CE PC0
#endif
#ifndef RADIO_CSN
#define RADIO_CSN PC1
#endif

typedef uint16_t Command;
typedef Command payload_t;

enum CommandType {
  Stop          = 0x00,
  SetDirection  = 0x01,
  SetSpeed      = 0x02,
  Trigger       = 0x03,
  // multiplexed commands (last 2 bits are for index 0,1,2,3)
  SetFrequency  = (1 << 2),
  SetDuty       = (2 << 2),
  // retrieval commands
  GetDirection  = (3 << 2),
  GetSpeed      = (4 << 2),
  GetFrequency  = (5 << 2),
  GetDuty       = (6 << 2),
  // configuration commands
  RadioLevel    = (7 << 2)
};

inline unsigned char get_index(CommandType cmd){
  return cmd & 0x03;
}
inline CommandType get_type(Command cmd){
  return cmd >> 8;
}
inline uint8_t get_data(Command cmd){
  return cmd & 0xFF;
}
inline Command make_command(CommandType type, uint8_t data) {
  return (type << 8) | data;
}

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
}
