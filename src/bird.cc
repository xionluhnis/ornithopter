// Alexandre Kaspar <akaspar@mit.edu>

#include <avr/io.h>
#include <util/delay.h>
#include <stdint.h>

#ifndef WIRED

// control from serial board
#define RADIO_NUM 1
#include "radio.h"

#endif

#include "command.h"
#include "control.h"
#include "pwm.h"
#include "time.h"
#include "usart.h"

void toggle_led(){
  static char state = 0;
  DDRD |= (1 << PD2);
  if(state)
    PORTD |= (1 << PD2);
  else
    PORTD &= ~(1 << PD2);
  state = !state;
}

Control controller;

int main(void) {

  clock_init();
#ifdef WIRED
  USART_init();
#else
  radio_init();
#endif
  pwm0_init();
  pwm1_init();
  pwm2_init();

  uint8_t counter = 0;

  // LOOP
  while(1){

    // update speed and orientation
    /*
    uint16_t left = 0, right = 0;
    if(controller.direction == 0){
      left  = 100 * controller.speed;
      right = 100 * controller.speed;
    } else if(controller.direction < 0){
      left  = 100 * controller.speed;
      right = (100 + controller.direction) * controller.speed;
    } else {
      left  = (100 - controller.direction) * controller.speed;
      right = 100 * controller.speed;
    }*/
    // 
    OCR1A = left;
    OCR1B = right;

#ifdef WIRED
    unsigned char cmd = USART_read();
    control_input(controller, cmd);

#else
    // process input from user
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

      CommandType what = get_type(response);
      union {
        uint8_t u;
        int8_t  s;
      } data;
      data.u = get_data(response);
      switch(what){

        case Stop:
          if(data.u == 0xFE)
            controller.reset();
          else
            controller.stop();
          break;

        case SetDirection:
          controller.direction = data.s;
          break;

        case SetSpeed:
          controller.speed = data.u;
          break;

        default:
          USART_send("Unsupported command: ");
          USART_sendNumber(static_cast<uint8_t>(what), 16);
          USART_send("\r\n");
          break;
      }
    }

#endif

    toggle_led();
    ++counter;
  }
  
  return 0;
}
