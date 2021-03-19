#include "UbxParser.h"

#define debug_port Serial
#define DEBUG

UbxParser ubx_;

void setup()
{
  debug_port.begin(115200);
  #ifdef DEBUG
  while(!debug_port){}
  #endif

  int msg_class = 100;
  int msg_id = 101;
  const int payload_length = 4;
  byte payload[payload_length] = {1,2,3,4};
  byte msg_buffer[100];
  int msg_length = ubx_.BuildMessage(msg_class, msg_id, payload_length, payload, msg_buffer);

  debug_port.print("Example message DEC: ");
  UbxParser::PrintBuffer(msg_buffer, msg_length, &debug_port);
  debug_port.print("Example message HEX: ");
  UbxParser::PrintBuffer(msg_buffer, msg_length, &debug_port, HEX);  
}

void loop()
{
  
}

