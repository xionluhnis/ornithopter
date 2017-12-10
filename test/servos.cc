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
    
    // PWM setup on 16-bit counter
    TCCR1A = (1 << COM1A1) | (0 << COM1A0) | (1 << COM1B1) | (0 << COM1B0); // clear OC1A/B on compare match
    TCCR1B = (0 << CS12) | (1 << CS11) | (0 << CS10) | (1 << WGM13); // prescaler /8, phase and frequency correct PWM, ICR1 TOP
    ICR1 = 25000; // 20 ms frequency
    DDRB |= (1 << PB1) | (1 << PB2);

    // PWM setup on 8-bit counter 0 (B)
    TCCR0A = (1 << COM0A1) | (0 << COM0A0) | (1 << COM0B1) | (0 << COM0B0) | (1 << WGM00);
    TCCR0B = (1 << CS02) | (0 << CS01) | (1 << CS00) | (1 << WGM02); // prescaler /1024, phase correct PWM, OCRA TOP
    OCR0A = 195; // 20MHz / 1024 / 2 / 195 = 50.08 Hz
    DDRD |= (1 << PD5) | (1 << PD6);

    // PWM setup on 8-bit counter 2 (B)
    TCCR2A = (1 << COM2B1) | (0 << COM2B0) | (1 << WGM20);
    TCCR2B = (1 << CS02) | (1 << CS01) | (1 << CS00) | (1 << WGM22); // prescaler /1024, phase correct PWM, OCRA TOP
    OCR2A = 195; // 20MHz / 1024 / 2 / 195 = 50.08 Hz
    DDRD |= (1 << PD3);

    // LOOP
    while(1){
    
        OCR1A = 1250;
        OCR1B = 1250;
        OCR0B = 10;
        OCR2B = 10;
        _delay_ms(1000);
        OCR1A = 1875;
        OCR1B = 1875;
        OCR0B = 15;
        OCR2B = 15;
        _delay_ms(1000);
        OCR1A = 2500;
        OCR1B = 2500;
        OCR0B = 20;
        OCR2B = 20;
        _delay_ms(1000);

        toggle_led();
    }
    
    return 0;
}
