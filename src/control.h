// Alexandre Kaspar <akaspar@mit.edu>
#pragma once

#include <stdint.h>

struct Control {
  uint8_t speed;
  int8_t  direction;
  Control() : speed(0), direction(0) {}

  inline bool left(){
    if(direction > -100)
      direction -= 1;
    else
      return false;
    return true;
  }

  inline bool right(){
    if(direction < 100)
      direction += 1;
    else
      return false;
    return true;
  }

  inline bool faster(){
    if(speed < 100)
      speed += 1;
    else
      return false;
    return true;
  }
  
  inline bool slower(){
    if(speed > 0)
      speed -= 1;
    else
      return false;
    return true;
  }

  inline void stop(){
    speed = 0;
  }

  inline void reset(){
    speed = 0;
    direction = 0;
  }
};
