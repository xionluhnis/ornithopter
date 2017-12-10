// Alexandre Kaspar <akaspar@mit.edu>

#include <avr/io.h>
#include <util/delay.h>

int main(void) {

    char state = 0;
    // set clock divider to /1
    // @see https://www.avrprogrammers.com/howto/sysclk-prescaler
    CLKPR = (1 << CLKPCE);
    CLKPR = (0 << CLKPS3) | (0 << CLKPS2) | (0 << CLKPS1) | (0 << CLKPS0);

    DDRD |= (1 << PD2);
    while(1){
        if(state)
            PORTD |= (1 << PD2);
        else
            PORTD &= ~(1 << PD2);
        state = !state;
        _delay_ms(1000);
    }

    return 0;
}
