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

    // PWM setup
    TCCR1A = (1 << COM1A1) | (0 << COM1A0) | (1 << COM1B1) | (0 << COM1B0); // clear OC1A/B on compare match
    TCCR1B = (0 << CS12) | (1 << CS11) | (0 << CS10) | (1 << WGM13); // prescaler /8, phase and frequency correct PWM, ICR1 TOP
    ICR1 = 25000; // 20 ms frequency
    DDRB |= (1 << PB1) | (1 << PB2);

    // LOOP
    while(1){
    
        OCR1A = 1250;
        OCR1B = 1250;
        _delay_ms(1000);
        OCR1A = 1875;
        OCR1B = 1875;
        _delay_ms(1000);
        OCR1A = 2500;
        OCR1B = 2500;
        _delay_ms(1000);

        toggle_led();
    }
    
    return 0;
}
