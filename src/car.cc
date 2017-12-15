// Alexandre Kaspar <akaspar@mit.edu>

#include <avr/io.h>
#include <util/delay.h>
#include <stdint.h>

#define RADIO_NUM 1

#include "control.h"
#include "pwm.h"
#include "radio.h"
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
  USART_init();
  radio_init();
  pwm1_init();

  // PWM setup
  TCCR1A = (1 << COM1A1) | (0 << COM1A0) | (1 << COM1B1) | (0 << COM1B0); // clear OC1A/B on compare match
  TCCR1B = (0 << CS12) | (1 << CS11) | (0 << CS10) | (1 << WGM13); // prescaler /8, phase and frequency correct PWM, ICR1 TOP
  // ICR1 = 25000; // 20 ms frequency
  ICR1 = 10000;
  DDRB |= (1 << PB1) | (1 << PB2);

  // LOOP
  while(1){

    // update speed and orientation
    uint16_t left = 0, right = 0;
    if(controller.direction == 0){
      left  = 100 * controller.speed;
      right = 100 * controller.speed;
    } else if(controller < 0){
      left  = 100 * controller.speed;
      right = (100 + controller.direction) * controller.speed;
    } else {
      left  = (100 - controller.direction) * controller.speed;
      right = 100 * controller.speed;
    }
    OCR1A = left;
    OCR1B = right;

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
      radio.read( &response, sizeof(Commmand) );

      CommandType what = get_type(response);
      union {
        uint8_t u;
        int8_t  s;
      } data = get_data(response);
      switch(what){

        case Stop:
          controller.speed = 0;
          controller.direction = 0;
          break;

        case SetDirection:
          controller.direction = data.s;
          break;

        case SetSpeed:
          controller.speed = data.s;
          break;
      }
    }

    toggle_led();
  }
  
  return 0;
}
