// Alexandre Kaspar <akaspar@mit.edu>
#pragma once

typedef uint16_t Command;
typedef Command payload_t;

enum CommandType {
  Stop          = 0x00,
  SetDirection  = 0x01,
  SetSpeed      = 0x02,
  Trigger       = 0x03,
  // multiplexed commands (last 2 bits are for index 0,1,2,3)
  SetFrequency  = (1 << 2),
  SetDuty       = (2 << 2),
  // retrieval commands
  GetDirection  = (3 << 2),
  GetSpeed      = (4 << 2),
  GetFrequency  = (5 << 2),
  GetDuty       = (6 << 2),
  // configuration commands
  RadioLevel    = (7 << 2)
};

inline unsigned char get_index(CommandType cmd){
  return cmd & 0x03;
}
inline CommandType get_type(Command cmd){
  return static_cast<CommandType>((cmd >> 8) & 0xFF);
}
inline uint8_t get_data(Command cmd){
  return cmd & 0xFF;
}
inline Command make_command(CommandType type, uint8_t data) {
  return (type << 8) | data;
}

