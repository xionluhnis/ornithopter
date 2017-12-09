// Alexandre Kaspar <akaspar@mit.edu>

#include <avr/io.h>
#include <util/delay.h>

int main(void) {

    char state = 0;

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
