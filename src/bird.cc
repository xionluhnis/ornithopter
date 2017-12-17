// Alexandre Kaspar <akaspar@mit.edu>

#include <avr/io.h>
#include <util/delay.h>
#include <stdint.h>

// angle definition for wings
#ifndef TOP_ANGLE
#define TOP_ANGLE 40
#endif
#ifndef BOT_ANGLE
#define BOT_ANGLE 20
#endif
// PWM conversion
// 90deg  = 1ms   = 1250
// 0deg   = 1.5ms = 1875
// -90deg = 2ms   = 2500
#define PWM_MAX 2500
#define PWM_MID 1875
#define PWM_MIN 1250
// our angles
#define PWM_TOP (PWM_MID + (PWM_MAX - PWM_MID) * TOP_ANGLE / 90)
#define PWM_BOT (PWM_MID - (PWM_MID - PWM_MIN) * BOT_ANGLE / 90)

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
uint8_t maxFrequency = 10;
bool running = false;

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
  // and finally the time system
  time_init();

  uint8_t counter = 0;
  unsigned long lastToggle = 0;
  enum {
    TOP,
    MIDDLE,
    BOTTOM
  } lastState = MIDDLE;

  // switch => pullup
  PORTC |= (1 << PC3);

  // LOOP
  while(1){

    // switches
    bool runSwitch = PINC & (1 << PC3);
    if(!runSwitch){
      running = !running;
    }

    // update speed
    if(!running){
      OCR1A = PWM_MID;
    } else if(!controller.speed){
      // special case, we go at the top angle
      OCR1A = PWM_TOP;
      OCR1B = PWM_TOP;
    } else {
      unsigned long now = millis();
      // full speed (100) => 10Hz toggling up/down
      unsigned long diff = now - lastToggle;

      //
      // floating point version:
      // float toggleMilliseconds = controller.speed * 10UL / maxFrequency;
      // 
      // better, only ints:
      if(diff * maxFrequency > controller.speed * 10UL){
        switch(lastState){
          case TOP:
            lastState = BOTTOM;
            break;
          case MIDDLE:
          case BOTTOM:
          default:
            lastState = TOP;
            break;
        }
        lastToggle = now;
      }
      if(lastState == BOTTOM){
        OCR1A = PWM_BOT;
        OCR1B = PWM_BOT;
      } else {
        OCR1A = PWM_TOP;
        OCR1B = PWM_TOP;
      }
    }

    // update orientation
    // TODO

#ifdef WIRED
    if(USART_ready_to_read()){
      unsigned char c = USART_read();
      controller.input(c);
    }

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

    // second led
    if(seconds() % 2)
      PORTD |= (1 << PD4);
    else
      PORTD &= ~(1 << PD4);
  }
  
  return 0;
}
