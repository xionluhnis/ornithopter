// Alexandre Kaspar <akaspar@mit.edu>
#pragma once

#include <stdint.h>
#include "command.h"

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

  bool input(unsigned char c, Command* output = 0){
    Command cmd = 0;
    switch(c){
      case 'a':
        if(slower())
          cmd = make_command(SetSpeed, speed);
        break;

      case 's':
        if(left())
          cmd = make_command(SetDirection, direction);
        break;
  
      case 'd':
        if(faster())
          cmd = make_command(SetSpeed, speed);
        break;
      
      case 'f':
        if(right())
          cmd = make_command(SetDirection, direction);
        break;
  
      case '0':
        speed = 100;
        cmd = make_command(SetSpeed, speed);
        break;
      case '1':
      case '2':
      case '3':
      case '4':
      case '5':
      case '6':
      case '7':
      case '8':
      case '9':
        speed = (c - '0') * 10;
        cmd = make_command(SetSpeed, speed);
        break;
  
      case '>':
        direction = 100;
        cmd = make_command(SetDirection, direction);
        break;
  
      case '<':
        direction = -100;
        cmd = make_command(SetDirection, direction);
        break;
  
      case ' ':
        stop();
        cmd = make_command(Stop, 0xFE);
        break;
  
      case 'q':
        reset();
        cmd = make_command(Stop, 0xFF);
        break;

        // trigger 0x00 => switch first button (running state)
      case 't':
        cmd = make_command(Trigger, 0x00);
        break;
        // trigger 0x01 => pull trigger action (e.g. pooping machine)
      case 'T':
        cmd = make_command(Trigger, 0x01);
        break;
  
      default:
        return false;
    }
    if(output)
      *output = cmd;
    return true;
  }
};
