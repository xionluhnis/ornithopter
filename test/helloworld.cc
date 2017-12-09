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
inline void USART_send(const char data){
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
    while(str != 0x00){
        USART_send(*str);
        ++str;
    }
}

void toggle_led(){
  static char state = 0;
  if(state)
    PORTD |= (1 << PD2);
  else
    PORTD &= ~(1 << PD2);
  state = !state;
}

int main(void) {

    DDRD |= (1 << PD2);
    while(1){
        toggle_led();
        _delay_ms(1000);
    }

    // set clock divider to /1
    // @see https://www.avrprogrammers.com/howto/sysclk-prescaler
    // CLKPR = (1 << CLKPCE);
    // CLKPR = (0 << CLKPS3) | (0 << CLKPS2) | (0 << CLKPS1) | (0 << CLKPS0);
    
    // set baudrate
    UBRR0H = (F_CPU/16/BAUDRATE - 1) >> 8;
    UBRR0L = (F_CPU/16/BAUDRATE - 1);
    
    // enable both receiver and transmitter with frame = [8data, 1 stp]
    UCSR0B |= (1 << RXEN0) | (1 << TXEN0);
    UCSR0C |= (1 << UCSZ01) | (1 << UCSZ00);
    
    // LOOP
    while(1){
        USART_send("Hello world!\r\n");
        // _delay_ms(3000);
        
        /*
        USART_send("Who are you?\r\n");
        _delay_ms(5000);
        if(USART_ready_to_read()){
            
        }
        */
    }
    
    return 0;
}
