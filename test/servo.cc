// Alexandre Kaspar <akaspar@mit.edu>

#include <avr/io.h>
#include <util/delay.h>
#include <stdint.h>

#ifndef BAUDRATE
#define BAUDRATE 9600
#endif

inline bool USART_ready_to_send(){
    return UCSR0A & (1 << UDRE0);
}
inline void USART_send(unsigned char data){
    while(!USART_ready_to_send());
    UDR0 = data; // put data in buffer
}
inline bool USART_ready_to_read(){
    return UCSR0A & (1 << RXC0);
}
inline char USART_read(){
    while(!USART_ready_to_read());
    return UDR0;
}
inline void USART_send(const char* str){
    while(*str != 0x00){
        USART_send(*str);
        ++str;
    }
}

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
    
    // set baudrate
    UBRR0H = (F_CPU/16/BAUDRATE - 1) >> 8;
    UBRR0L = (F_CPU/16/BAUDRATE - 1);
    
    // enable both receiver and transmitter with frame = [8data, 1 stp]
    UCSR0B |= (1 << RXEN0) | (1 << TXEN0);
    UCSR0C |= (1 << UCSZ01) | (1 << UCSZ00);

    char rbuffer[128];
    unsigned char size = 0;

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

        /*
        // store input characters
        char chr = USART_read();
        rbuffer[size % 128] = chr;
        size++;
        if(chr < 32){
            // echo back
            toggle_led();
            USART_send("echo:\r\n");
            for(unsigned char i = 0; i < size % 128; ++i){
                chr = rbuffer[i];
                if(chr == '\n' || chr == '\r')
                    continue;
                USART_send(chr);
            }
            USART_send("\r\n");
            size = 0;
        }*/
    }
    
    return 0;
}
