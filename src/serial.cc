// Alexandre Kaspar <akaspar@mit.edu>

#include <avr/io.h>
#include <util/delay.h>
#include <stdint.h>

#define RADIO_NUM 0

#include "usart.h"
#include "radio.h"
#include "control.h"

Control controller;

int main(void) {

  clock_init();
  USART_init();
  radio_init();

  Command cmd = 0;
  Command nextCmd = 0;

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
          c = USART_read();
          switch(c){
              // down arrow
            case 0x50:
            case 0xD0:
              if(controller.slower())
                cmd = make_command(SetSpeed, controller.speed);
              break;

              // left arrow
            case 0x4B:
            case 0xCB:
              if(controller.left())
                cmd = make_command(SetDirection, controller.direction);
              break;

              // right arrow
            case 0x4D:
            case 0xCD:
              if(controller.right())
                cmd = make_command(SetDirection, controller.direction);
              break;

              // top arrow
            case 0x48:
            case 0xC8:
              if(controller.faster())
                cmd = make_command(SetSpeed, controller.speed);
              break;

            default:
              USART_send("Unknown control: ");
              USART_sendNumber(code, 16);
              USART_send("\r\n");
              break;
          }
          break;

        case ' ':
          controller.stop();
          cmd = make_command(SetSpeed, controller.speed);
          break;

          // escape
        case 0x1B:
          controller.reset();
          cmd = make_command(Stop, 0x00);
          break;

        default:
          // ???
          USART_send("Unsupported input: ");
          USART_send(code);
          USART_send("\r\n");
          continue;
      } 
    }

    radio.startListening();
    
    bool timeout = false;                                   // Set up a variable to indicate if a response was received or not
    
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
      radio.read( &response, sizeof(Commmand) );
      USART_send("Got response ");
      USART_sendNumber(response, 16);
      USART_send("\r\n");
    }

    // Try again 1s later
    _delay_ms(1);
  }
  
  return 0;
}
