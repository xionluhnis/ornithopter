// Alexandre Kaspar <akaspar@mit.edu>
#pragma once

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/atomic.h>
#include <stdint.h>
#include <usart.h>

// running at 20ms per interrupt on timer1
// => no fractional part for microseconds
// => easy to compute

volatile unsigned long timer1_overflow_count = 0;

ISR(TIMER1_OVF_vect) {
	timer1_overflow_count++;
}

unsigned long overflows() {
  unsigned long o;
  ATOMIC_BLOCK(ATOMIC_RESTORESTATE){
    o = timer1_overflow_count;
  }
  return o;
}

unsigned long seconds() {
  return overflows() / 50UL;
}

unsigned long millis() {
	unsigned long o;
  uint16_t t;
  ATOMIC_BLOCK(ATOMIC_RESTORESTATE){
    o = timer1_overflow_count;
    t = TCNT1;
  }
  unsigned long T = t;
  return 20UL * o + T * 2UL / 5000UL;
}

unsigned long micros() {
	unsigned long o;
  uint16_t t;
  ATOMIC_BLOCK(ATOMIC_RESTORESTATE){
    o = timer1_overflow_count;
    t = TCNT1;
  }
	return 20000UL * o + t * 2L / 5L;
}


void clock_init(uint8_t prescaler = 0) {
  // set clock divider to /1
  // @see https://www.avrprogrammers.com/howto/sysclk-prescaler
  CLKPR = (1 << CLKPCE);
  // CLKPR = (0 << CLKPS3) | (0 << CLKPS2) | (0 << CLKPS1) | (0 << CLKPS0);
  CLKPR = ((prescaler & 0x8) << CLKPS3) | ((prescaler & 0x4) << CLKPS2) | ((prescaler & 0x2) << CLKPS1) | ((prescaler & 0x1) << CLKPS0);
}

// Note: pwm1_init() must have been called, we assume it has (or hasn't and won't afterwards)
void time_init(){
	// this needs to be called before setup() or some functions won't
	// work there
	sei();

  // enable timer 1 overflow interrupt
  TIMSK1 |= (1 << TOIE1);

}

