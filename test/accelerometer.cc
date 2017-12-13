// Alexandre Kaspar <akaspar@mit.edu>

#include <avr/io.h>
#include <util/delay.h>
#include <util/twi.h>
#include <stdint.h>

#ifndef BAUDRATE
#define BAUDRATE 9600
#endif
#ifndef I2C_BAUDRATE
#define I2C_BAUDRATE 100000
#endif
#define I2C_READ 0x01
#define I2C_WRITE 0x00

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
inline void USART_sendNumber(unsigned char num, unsigned char base = 10){
  static unsigned char numbers[] = "0123456789ABCDEF";
  unsigned char digits[8];
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

void toggle_led(){
  static char state = 0;
  DDRD |= (1 << PD2);
  if(state)
    PORTD |= (1 << PD2);
  else
    PORTD &= ~(1 << PD2);
  state = !state;
}

#define I2C_DEBUG(str) 

inline void I2C_init(){
    I2C_DEBUG(USART_send(". init\r\n"));
    // 20M / 100k - 16 = 184
    // I2C_BAUDRATE = F_CPU / (16 + 2 * TWBR * PRESCALER)
    // TWBR = (F_CPU / I2C_BAUDRATE - 16) / 2 / PRESCALER
    // TWSR = 2; // PRESCALER=16
    TWSR = 0;
    // unity prescaling
    TWBR = ((F_CPU / I2C_BAUDRATE ) - 16) / 2 / 1;
}
inline void I2C_wait(){
    // USART_send(". wait\r\n");
    while( !(TWCR & (1<<TWINT)) );
}
inline bool I2C_start(unsigned char addr){
    I2C_DEBUG(USART_send(". start with addr="));
    I2C_DEBUG(USART_sendNumber(addr));
    I2C_DEBUG(USART_send("\r\n"));
    bool reading = addr & I2C_READ;
    TWCR = 0; // reset control register
    // send START condition
    TWCR = (1 << TWINT) | (1 << TWSTA) | (1 << TWEN);
    I2C_wait();
    // check status (different from START?)
    unsigned char status = TWSR & 0xF8;
    I2C_DEBUG(USART_send(". status="); USART_sendNumber(status, 16); USART_send("\r\n"));
    switch(status){
        case 0x08: // start transmitted
        case 0x10: // repeated start transmitted
            break;
        default:
            USART_send("Start failed. Status="); USART_sendNumber(status, 16); USART_send("\r\n");
            return false;
    }
    // load address into register
    TWDR = addr;
    // start transmission
    TWCR = (1 << TWINT) | (1 << TWEN);
    I2C_wait();
    // return ack status
    status = TWSR & 0xF8;
    unsigned char okStatus = reading ? 0x40 : 0x18;
    if(status != okStatus){
        if(reading)
            USART_send("Start receive failed. ");
        else
            USART_send("Start transmit failed. ");
        USART_send("Status=");
        USART_sendNumber(status, 16);
        USART_send("\r\n");
    }
    return status == okStatus; // transmitted and ack received (MT_SLA_ACK)
}
inline bool I2C_write(unsigned char data){
    I2C_DEBUG(USART_send(". write data="); USART_sendNumber(data); USART_send("\r\n"));
    // load data into register
    TWDR = data;
    // start transmission
    TWCR = (1 << TWINT) | (1 << TWEN);
    I2C_wait();
    // return ack status
    unsigned char status = TWSR & 0xF8;
    if(status != 0x28){
        USART_send("Write failed. Status="); USART_sendNumber(status, 16); USART_send("\r\n");
    }
    return status == 0x28; // transmitted and data acked (MT_DATA_ACK)
}
inline unsigned char I2C_read(bool ack = true){
    I2C_DEBUG(USART_send(". read, ack="); USART_send(ack ? "true" : "false"); USART_send("\r\n"));
    // start TWI module with/without data acknowledgement
    if(ack)
        TWCR = (1 << TWINT) | (1 << TWEN) | (1 << TWEA);
    else
        TWCR = (1 << TWINT) | (1 << TWEN);
    I2C_wait();
    unsigned char data = TWDR;
    I2C_DEBUG(USART_send("Read "); USART_sendNumber(data); USART_send("\r\n"));
    return data;
}
inline void I2C_stop(){
    I2C_DEBUG(USART_send(". stop\r\n"));
    // send STOP condition
    TWCR = (1 << TWINT) | (1 << TWEN) | (1 << TWSTO);
}
inline bool I2C_transmit(unsigned char addr, unsigned char *data, unsigned char length){
    if(!I2C_start(addr | I2C_WRITE)){
        I2C_stop();
        return false;
    }
    for(unsigned char i = 0; i < length; ++i){
        if(!I2C_write(data[i])){
            I2C_stop();
            return false;
        }
    }
    I2C_stop();
    return true;
}
inline bool I2C_receive(unsigned char addr, unsigned char *data, unsigned char length){
    if(!I2C_start(addr | I2C_READ)){
        I2C_stop();
        return false;
    }
    for(unsigned char i = 0; i < length - 1; ++i)
        data[i] = I2C_read(true);
    data[length-1] = I2C_read(false);
    I2C_stop();
    return true;
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

    _delay_ms(1000);
    USART_send("Starting I2C\r\n");

    // TWI setup
    unsigned char addr = 0x53 << 1; // LSB is for read (1) or write (0)
    unsigned char data[6]; // 3x 16bits
    I2C_init();

    _delay_ms(10);

    USART_send("I2C_init() done\r\n");
    USART_send("Transmitting Power Up\r\n");

    // enable accelerometer
    data[0] = 0x2D; // POWER_CTL register
    data[1] = 8; // turn on measure bit
    if(!I2C_transmit(addr, data, 2))
        USART_send("Issue with power control\r\n"); // write 8 into register 0x2D
    else
        USART_send("Power up transmitted\r\n");
    // wait for wake up
    _delay_ms(10);

    // LOOP
    while(1){
        toggle_led();
        
        // select X0 register
        data[0] = 0x32;
        if(!I2C_transmit(addr, data, 1))
            USART_send("Issue selecting X0 register\r\n"); // select register
        I2C_receive(addr, data, 6); // read 6 bytes from register 0x32

#ifndef BINARY_MODE
#define BINARY_MODE 1
#endif

#if BINARY_MODE == 1
        // send frame
        USART_send(1);
        USART_send(2);
        USART_send(3);
        USART_send(4);
        // send acceleration data
        for(unsigned char i = 0; i < 6; ++i)
            USART_send(data[i]);
#else
        // send acceleration data
        for(unsigned char i = 0; i < 6; ++i)
            USART_sendNumber(data[i]);
        USART_send("\r\n");
#endif
    }
    
    return 0;
}
