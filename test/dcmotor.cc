// Alexandre Kaspar <akaspar@mit.edu>

#include <avr/io.h>
#include <util/delay.h>
#include <stdint.h>

void toggle_led(){
  static char state = 0;
  DDRD |= (1 << PD2);
  if(state)
    PORTD |= (1 << PD2);
  else
    PORTD &= ~(1 << PD2);
  state = !state;
}

int main(void) {

    // set clock divider to /1
    // @see https://www.avrprogrammers.com/howto/sysclk-prescaler
    CLKPR = (1 << CLKPCE);
    CLKPR = (0 << CLKPS3) | (0 << CLKPS2) | (0 << CLKPS1) | (0 << CLKPS0);
    
    // PWM setup on 8-bit counter 2 (B)
    TCCR2A = (1 << COM2B1) | (0 << COM2B0) | (1 << WGM20);
    TCCR2B = (1 << CS22) | (1 << CS21) | (1 << CS20) | (1 << WGM22); // prescaler /1024, phase correct PWM, OCRA TOP
    OCR2A = 200; // 20MHz / 1024 / 200 / 2 = 50 kHz
    DDRD |= (1 << PD3);

    // LOOP
    while(1){
    
        OCR2B = 10;
        _delay_ms(1000);
        OCR2B = 50;
        _delay_ms(1000);
        OCR2B = 100;
        _delay_ms(1000);

        toggle_led();
    }
    
    return 0;
}
