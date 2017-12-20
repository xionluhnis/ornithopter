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
#define PWM_MAX 2500UL
#define PWM_MID 1875UL
#define PWM_MIN 1250UL
// our angles
#define PWM_TOP (PWM_MID + (PWM_MAX - PWM_MID) * TOP_ANGLE / 90UL)
#define PWM_BOT (PWM_MID - (PWM_MID - PWM_MIN) * BOT_ANGLE / 90UL)
#define PWM_INV(a) (PWM_MIN + PWM_MAX - a)

// maximum allowed consecutive timeouts before going into landing
#ifndef MAX_TIMEOUTS
#define MAX_TIMEOUTS -1
#endif
// timeout in milliseconds
#ifndef RADIO_TIMEOUT
#define RADIO_TIMEOUT 100
#endif
// radio poll every so often (milliseconds)
#ifndef RADIO_POLLING_PERIOD
#define RADIO_POLLING_PERIOD 1000 
#endif

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

// global variables
Control controller;
unsigned long maxFrequency = 10UL; // 10 toggles per second
bool running = false;
// counters / timers
unsigned long counter = 0;
uint8_t numTimeouts = 0;

bool check_run_switch();
void update_wings();
void update_tail();
void radio_input();

int main(void) {

  clock_init(); // /1 prescaler
#ifdef WIRED
  USART_init();
  USART_send("Bird initializing.\r\n");
#else
  radio_init();
#endif
  pwm0_init();
  pwm1_init();
  pwm2_init();
  // and finally the time system
  time_init();

  // switch => pullup
  PORTC |= (1 << PC3);
  DDRD |= (1 << PD7); // trigger signal

#ifdef WIRED
  USART_send("\r\nDDRB=");
  USART_sendNumber(DDRB, 2);
  USART_send("\r\nDDRC=");
  USART_sendNumber(DDRC, 2);
  USART_send("\r\nDDRD=");
  USART_sendNumber(DDRD, 2);
  USART_send("\r\n\r\nPORTB=");
  USART_sendNumber(PORTB, 2);
  USART_send("\r\nPORTC=");
  USART_sendNumber(PORTC, 2);
  USART_send("\r\nPORTD=");
  USART_sendNumber(PORTD, 2);
  USART_send("\r\n");
#endif

  // LOOP
  while(1){
    // USART_sendNumber(millis());
    // USART_send("\r\n");

    // switches
    if(check_run_switch())
      running = !running;

    // update speed
    update_wings();

    // update orientation
    update_tail();

#ifdef WIRED
    if(USART_ready_to_read()){
      unsigned char c = USART_read();
      USART_send("Input: ");
      USART_send(c);
      USART_send("\r\n");
      Command cmd = 0;
      if(controller.input(c, &cmd)){
        USART_send("Command: ");
        USART_sendNumber(cmd, 16);
        USART_send("\r\n");
      }
    }

#else
    radio_input();
#endif

    toggle_led();
    ++counter;

#ifdef WIRED
    if(counter % 10000 == 0){
      USART_send("m=");
      USART_sendNumber(millis());
      USART_send("\r\n");
    }
#endif

    // second led
    if(seconds() % 2)
      PORTD |= (1 << PD4);
    else
      PORTD &= ~(1 << PD4);
  }
  
  return 0;
}


bool check_run_switch(){
  static unsigned long lastSwitch = 0;
  bool trigger = false;
  bool runSwitch = PINC & (1 << PC3); // is the switch being pressed?
  if(!runSwitch){
    unsigned long now = millis();
    trigger = now > lastSwitch + 1000UL;
    lastSwitch = now; // so we can press as long as we want (and only count after release)
  }
  return trigger;
}

void update_wings(){
  static unsigned long lastToggle = 0;
  static enum {
    TOP,
    MIDDLE,
    BOTTOM
  } wingState = MIDDLE;
  // cases
  if(!running || (MAX_TIMEOUTS >= 0 && numTimeouts > MAX_TIMEOUTS)){
    // if not active, stay in mid position
    wingState = MIDDLE;
    OCR1A = PWM_MID;
    OCR1B = PWM_MID;
  } else if(!controller.speed){
    // special case, we go at the top angle
    wingState = TOP;
    OCR1A = PWM_TOP;
    OCR1B = PWM_BOT;
  } else {
    unsigned long now = millis();
    // full speed (100) => 10Hz toggle cycle (1 up + 1 down)
    unsigned long deltaThreshold = 1000UL * maxFrequency / ((unsigned long)controller.speed) / 2UL;
    // Note: / 2 is because we need two toggles (1 up + 1 down)
    if(now  >= lastToggle + deltaThreshold){
      if(wingState == TOP)
        wingState = BOTTOM;
      else
        wingState = TOP;
      lastToggle = now;
      // change PWM
      if(wingState == BOTTOM) {
        OCR1A = PWM_BOT;
        OCR1B = PWM_TOP;
      } else {
        OCR1A = PWM_TOP;
        OCR1B = PWM_BOT;
      }
    }
  }
}

void update_tail(){
  // TODO implement this!
}

void radio_input(){
  static unsigned long lastRadio = 0;
  unsigned long now = millis();

  // only poll every so often
  // TODO the polling should depend on speed
  // TODO communication should be done with interrupts, not
  if(now < lastRadio + RADIO_POLLING_PERIOD)
    return;
  else
    lastRadio = now;

  // process input from user
  bool timeout = false;
  while ( ! radio.available() ){
    if(millis() > now + RADIO_TIMEOUT){
      timeout = true;
      break;
    }
  }
  if ( timeout ){
#if MAX_TIMEOUTS == -1
    return; // we just skip this time
#endif
    // If it times out too many times, we stop the bird by precaution
    // Note: do not let count overflow (else the bird may be in an odd state)
    if(numTimeouts < 255)
      ++numTimeouts;
  }else{
    // reset timeout counter
    numTimeouts = 0;

    // get radio command
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

      case Trigger:
        // different types of triggers
        switch(data.u){
          case 0x00:
            running = !running;
            break;
          
          case 0x01: {
              static bool triggered = false;
              triggered = !triggered;
              if(triggered)
                PORTD |= (1 << PD7);
              else
                PORTD &= ~(1 << PD7);
            } break;

          default:
            break;
        }
        break;

      default:
#ifdef WIRED
        // this actualy never gets reached right now
        USART_send("Unsupported command: ");
        USART_sendNumber(static_cast<uint8_t>(what), 16);
        USART_send("\r\n");
#endif
        break;
    }
  }

}
