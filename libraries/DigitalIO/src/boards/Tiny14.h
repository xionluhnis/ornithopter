#ifndef TINY_14_H
#define TINY_14_H

// ATMEL ATTINY84 / ARDUINO
//
//                           +-\/-+
//                     VCC  1|    |14  GND
//             (D 10)  PB0  2|    |13  AREF (D  0)
//             (D  9)  PB1  3|    |12  PA1  (D  1) 
//                     PB3  4|    |11  PA2  (D  2) 
//  PWM  INT0  (D  8)  PB2  5|    |10  PA3  (D  3) 
//  PWM        (D  7)  PA7  6|    |9   PA4  (D  4) 
//  PWM        (D  6)  PA6  7|    |8   PA5  (D  5)        PWM
//                           +----+

static const GpioPinMap_t GpioPinMap[] = {
  GPIO_PIN(A, 0),  // D0  = p0
  GPIO_PIN(A, 1),  // D1  = p1
  GPIO_PIN(A, 2),  // D2  = p2
  GPIO_PIN(A, 3),  // D3  = p3
  GPIO_PIN(A, 4),  // D4  = p4
  GPIO_PIN(A, 5),  // D5  = p5
  GPIO_PIN(A, 6),  // D6  = p6
  GPIO_PIN(A, 7),  // D7  = p7
  GPIO_PIN(B, 2),  // D8  = p8
  GPIO_PIN(B, 1),  // D9  = p9
  GPIO_PIN(B, 0)  //  D10 = p10
  /*
  GPIO_PIN(B, 0),  // D0  = p2
  GPIO_PIN(B, 1),  // D1  = p3
  GPIO_PIN(B, 3),  // D2  = p4
  GPIO_PIN(B, 2),  // D3  = p5
  GPIO_PIN(A, 7),  // D4  = p6
  GPIO_PIN(A, 6),  // D5  = p7
  GPIO_PIN(A, 5),  // D6  = p8
  GPIO_PIN(A, 4),  // D7  = p9
  GPIO_PIN(A, 3),  // D8  = p10
  GPIO_PIN(A, 2),  // D9  = p11
  GPIO_PIN(A, 1),  // D10 = p12
  GPIO_PIN(A, 0)   // D11 = p13
  */
};
#define NUM_DIGITAL_PINS (12)
#endif  // TINY_14_H