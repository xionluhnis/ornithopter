// Alexandre Kaspar <akaspar@mit.edu>

#include <avr/io.h>
#include <util/delay.h>
#include <stdint.h>

#define RADIO_NUM 0

#include "control.h"
#include "radio.h"
#include "time.h"
#include "usart.h"

unsigned char buffer[256];
uint8_t length;

Control controller;
Command cmd = 0;
Command nextCmd = 0;

void toggle_led(){
  static char state = 0;
  DDRD |= (1 << PD2);
  if(state)
    PORTD |= (1 << PD2);
  else
    PORTD &= ~(1 << PD2);
  state = !state;
}

void processCommand();

int main(void) {

  clock_init();
  USART_init();
  radio_init(true);

  // LOOP
  while(1){
    // clear
    toggle_led();

    if(cmd)
      radio.stopListening();
    while(cmd){
      USART_send("Sending");
      if (!radio.write( &cmd, sizeof(Command) ))
        USART_send("... failed!\r\n");
      else
        USART_send(" ok!\r\n");
      cmd = nextCmd;
      nextCmd = 0;
    }

    // reading something?
    if(USART_ready_to_read()){
      unsigned char c = USART_read();
      // cases
      switch(c){
        case 0x00:
        case 0xE0:
        case 0x5B: // own linux
          c = USART_read();
          switch(c){
              // down arrow
            case 0x50:
            case 0xD0:
            case 0x42:
              if(controller.slower())
                cmd = make_command(SetSpeed, controller.speed);
              break;

              // left arrow
            case 0x4B:
            case 0xCB:
            case 0x44:
              if(controller.left())
                cmd = make_command(SetDirection, controller.direction);
              break;

              // right arrow
            case 0x4D:
            case 0xCD:
            case 0x43:
              if(controller.right())
                cmd = make_command(SetDirection, controller.direction);
              break;

              // top arrow
            case 0x48:
            case 0xC8:
            case 0x41:
              if(controller.faster())
                cmd = make_command(SetSpeed, controller.speed);
              break;

            default:
              USART_send("Unknown control: ");
              USART_sendNumber(c, 16);
              USART_send("\r\n");
              break;
          }
          break;

          // escape / delete
        case 0x1B:
        case 0x7F:
          controller.reset();
          cmd = make_command(Stop, 0xFF);
          break;

        default:
          if((c >= '0' && c <= '9') // digits
          || (c >= 'A' && c <= 'Z') // upper case
          || (c >= 'a' && c <= 'z') // lower case
          || c == '\r' || c == '\n' || c == ' '
          ){
            if(c == '\r' || c == '\n')
              
              processCommand();
            else {
              if(length < 256){
                buffer[length] = c;
                ++length;
                USART_send(c);
              } else
                processCommand();
            }
          } else {
            // ???
            USART_send("Unsupported input: ascii=");
            USART_send(c);
            USART_send(", code=");
            USART_sendNumber(c, 16);
            USART_send("\r\n");
          }
          continue;
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

    // Try again 1s later
    _delay_ms(1);
    */
  }
  
  return 0;
}

void processCommand(){
  USART_send("\r\n");
}
