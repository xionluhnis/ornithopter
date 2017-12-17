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
};

bool control_input(Control &controller, unsigned char c, Command* output = NULL){
  Command cmd;
  switch(c){
    case 'a':
      if(controller.slower())
        cmd = make_command(SetSpeed, controller.speed);
      break;

    case 's':
      if(controller.left())
        cmd = make_command(SetDirection, controller.direction);
      break;

    case 'd':
      if(controller.faster())
        cmd = make_command(SetSpeed, controller.speed);
      break;
    
    case 'f':
      if(controller.right())
        cmd = make_command(SetDirection, controller.direction);
      break;

    case '0':
      controller.speed = 100;
      cmd = make_command(SetSpeed, controller.speed);
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
      controller.speed = (c - '0') * 10;
      cmd = make_command(SetSpeed, controller.speed);
      break;

    case '>':
      controller.direction = 100;
      cmd = make_command(SetDirection, controller.direction);
      break;

    case '<':
      controller.direction = -100;
      cmd = make_command(SetDirection, controller.direction);
      break;

    case ' ':
      controller.stop();
      cmd = make_command(Stop, 0xFE);
      break;

    case 'q':
      controller.reset();
      cmd = make_command(Stop, 0xFF);
      break;

    default:
      return false;
  }
  if(output)
    *output = cmd;
}
