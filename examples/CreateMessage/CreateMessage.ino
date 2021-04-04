#include "UbxInterpreter.h"

#define debug_port Serial

UbxInterpreter ubx_;

void setup()
{
  debug_port.begin(115200);
  while(!debug_port){}

  uint8_t msg_class = 100;
  uint8_t msg_id = 101;
  const int payload_length = 4;
  uint8_t payload[payload_length] = {1,2,3,4};
  uint16_t value1 = 100;
  int16_t value2 = -200;
  uint8_t msg_buffer[100];
  ubx_.setHeaderValues(msg_class, msg_id, payload_length);
  ubx_.packValue(value1, 0);
  ubx_.packValue(value2, 2);
  ubx_.prepareMessage();
  // int msg_length = ubx_.getMessage(msg_buffer);
  // int msg_length = ubx_.buildMessage(msg_class, msg_id, payload_length, payload, msg_buffer);

  debug_port.print("Example message DEC: ");
  ubx_.printWriteBuffer(&debug_port);
  debug_port.print("Example message HEX: ");
  ubx_.printWriteBuffer(&debug_port, HEX);  
}

void loop()
{
  
}