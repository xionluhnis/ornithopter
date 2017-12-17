// Alexandre Kaspar <akaspar@mit.edu>

#include <avr/io.h>
#include <util/delay.h>
#include <stdint.h>
#include <string.h>

#define RADIO_NUM 0

#include "command.h"
#include "control.h"
#include "radio.h"
#include "time.h"
#include "usart.h"

char buffer[256];
uint8_t length;

Control controller;
Command cmd = 0;
bool interactive = false;

void toggle_led(){
  static char state = 0;
  DDRD |= (1 << PD2);
  if(state)
    PORTD |= (1 << PD2);
  else
    PORTD &= ~(1 << PD2);
  state = !state;
}

void processInput();
void processCommand();

int main(void) {

  clock_init();
  USART_init();
  radio_init(true);

  // LOOP
  while(1){
    // clear
    toggle_led();

    if(cmd){
      radio.stopListening();
      USART_send("Sending ");
      USART_sendNumber(cmd, 16);
      // USART_send("\r\n");
      if (!radio.write( &cmd, sizeof(Command) ))
        USART_send(" ... failed!\r\n");
      else
        USART_send(" ok!\r\n");
      cmd = 0;
    }

    // reading something?
    if(USART_ready_to_read()){
      if(interactive){
        processInput();
      } else {

        unsigned char c = USART_read();
        if((c >= '0' && c <= '9') // digits
          || (c >= 'A' && c <= 'Z') // upper case
          || (c >= 'a' && c <= 'z') // lower case
          || c == '\r' || c == '\n' || c == ' '
        ){
          if(c == '\r' || c == '\n')
            
            processCommand();
          else {
            if(length < 255){
              buffer[length] = c;
              ++length;
              USART_send(c);
            } else
              processCommand();
          }
        } else if(c == 0x7F){ // backspace
          if(length)
            --length;
          USART_send(c);
        } else {
          // ???
          USART_send("Unsupported input: ascii=");
          USART_send(c);
          USART_send(", code=");
          USART_sendNumber(c, 16);
          USART_send("\r\n");
        }
      }
    }

    /*
    radio.startListening();
    
    bool timeout = false;
    
    uint16_t count = 0;
    while ( ! radio.available() ){
      ++count;
      if(count > 2000){
        timeout = true;
        break;
      }
    }
        
    if ( timeout ){
      USART_send("Response timed out.\r\n");
    }else{
      Command response = 0;
      radio.read( &response, sizeof(Command) );
      if(response == 0x00 || response == 0xFF || response == 0xFFFF){
        continue; // skip it
      }
      USART_send("Got response ");
      USART_sendNumber(response, 16);
      USART_send("\r\n");
    }
    */

    // Try again 1s later
    _delay_ms(1);
    
  }
  
  return 0;
}

void processCommand(){
  USART_send("\r\n");
  buffer[length] = 0; // null-terminate
  USART_send("command(");
  USART_sendNumber(length);
  USART_send("): ");
  for(uint8_t i = 0; i < length; ++i)
    USART_send(buffer[i]);
  USART_send("\r\n");

  if(strcmp(buffer, "i") == 0 || strcmp(buffer, "interactive") == 0){
    interactive = true;
    USART_send("Interactive mode enabled\r\n");
  } else if(strncmp(buffer, "set", 3)){
    // setter command
    
  } else if(strncmp(buffer, "get", 3)){
    // getter command

  
  } else {
    USART_send("Not supported\r\n");
  }

  // clear buffer
  for(uint8_t i = 0; i < 255; ++i)
    buffer[i] = 0;
  length = 0;
}
void processInput(){
  USART_send("processInput()\r\n");
  unsigned char c = USART_read();
  USART_sendNumber(c, 16);
  USART_send("\r\n");

  // control commands
  if(controller.input(c, &cmd)){
    return;
  }
  // non-control inputs
  switch(c){

    case 0x0D: // delete, return
    case 0x7F:
      interactive = false;
      USART_send("Interactive mode disabled\r\n");
      break;

    default:
      USART_send("Unknown control: ");
      USART_sendNumber(c, 16);
      USART_send("\r\n");
      break; 
  }
}
