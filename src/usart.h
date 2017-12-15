// Alexandre Kaspar <akaspar@mit.edu>
#pragma once

#include <avr/io.h>
#include <util/delay.h>
#include <stdint.h>
#ifndef BAUDRATE
#define BAUDRATE 9600
#endif

inline void USART_init() {
    // set baudrate
    UBRR0H = (F_CPU/16/BAUDRATE - 1) >> 8;
    UBRR0L = (F_CPU/16/BAUDRATE - 1);
    
    // enable both receiver and transmitter with frame = [8data, 1 stp]
    UCSR0B |= (1 << RXEN0) | (1 << TXEN0);
    UCSR0C |= (1 << UCSZ01) | (1 << UCSZ00);
}

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
