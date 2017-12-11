// Alexandre Kaspar <akaspar@mit.edu>

#include <avr/io.h>
#include <util/delay.h>
#include <stdint.h>

#include "RF24.h"

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
void USART_send(const char* str){
    while(*str != 0x00){
        USART_send(*str);
        ++str;
    }
}
template < typename Number >
void USART_sendNumber(Number num, unsigned char base){
  static unsigned char numbers[] = "0123456789ABCDEF";
  unsigned char digits[4 * sizeof(Number)];
  unsigned char length = 0;
  while(num > 0){
      digits[length] = num % base;
      num /= base;
      ++length;
  }
  switch(base) {
      case 16:
          USART_send("0x");
          break;
      case 2:
          USART_send("0b");
          break;
      default:
          break;
  }
  if(length == 0){
      USART_send('0');
      return;
  }
  for(unsigned char i = 0; i < length; ++i){
      USART_send(numbers[digits[length-i-1]]);
  }
}

void USART_sendNumber(unsigned long num) {
    USART_sendNumber(num, 10);
}

void USART_sendNumber(unsigned char num) {
    USART_sendNumber(num, 10);
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

#define RADIO_NUM 1

const unsigned char addresses[][6] = {"1Node","2Node"};
RF24 radio(PC1, PC2);

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

    // radio setup
    USART_send("RF24 / Radio\r\n");
    while(!radio.begin()){
        USART_send("Initialization error\r\n");
        _delay_ms(3000);
    }

    // Set the PA Level low to prevent power supply related issues since this is a
    // getting_started sketch, and the likelihood of close proximity of the devices. RF24_PA_MAX is default.
    radio.setPALevel(RF24_PA_LOW);

    // Open a writing and reading pipe on each radio, with opposite addresses
    radio.openWritingPipe(addresses[RADIO_NUM]);
    radio.openReadingPipe(1, addresses[1 - RADIO_NUM]);
    
    // Start the radio listening for data
    radio.startListening();
    
    typedef unsigned long payload_t;

    payload_t data[] = { 0UL, 100UL, 1000UL };
    unsigned char i = 0;
    unsigned long count = 0;

    // LOOP
    while(1){
        toggle_led();

        radio.stopListening(); // stop listening so that we can talk
        USART_send("Sending\r\n");
        if (!radio.write( &data[i], sizeof(payload_t) )){
            USART_send("Failed!\r\n");
        }
        radio.startListening();
        
        bool timeout = false;                                   // Set up a variable to indicate if a response was received or not
        
        count = 0;
        while ( ! radio.available() ){
          ++count;
          if(count > 2000){
            timeout = true;
            break;
          }
        }
            
        if ( timeout ){                                             // Describe the results
            USART_send("Failed, response timed out.\r\n");
        }else{
            payload_t got_time;                                 // Grab the response, compare, and send to debugging spew
            radio.read( &got_time, sizeof(payload_t) );
            USART_send("Sent ");
            USART_sendNumber(data[i]);
            USART_send(", got response ");
            USART_sendNumber(got_time);
            USART_send("\r\n");
        }
        i = (i + 1) % 3;

        // Try again 1s later
        _delay_ms(1000);
    }
    
    return 0;
}
