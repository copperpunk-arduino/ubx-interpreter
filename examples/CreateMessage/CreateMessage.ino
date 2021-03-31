#include "UbxParser.h"

#define debug_port Serial

UbxParser ubx_;

void printBuffer(uint8_t msg_buffer[], int msg_length, Stream *port, int output_type=DEC)
{
    int i = 0;
    for (; i < msg_length-1; i++)
    {
        port->print(msg_buffer[i], output_type);
        port->print(F(","));
    }
    port->println(msg_buffer[i]);
}


void setup()
{
  debug_port.begin(115200);
  while(!debug_port){}

  uint8_t msg_class = 100;
  uint8_t msg_id = 101;
  const int payload_length = 4;
  uint8_t payload[payload_length] = {1,2,3,4};
  uint8_t msg_buffer[100];
  int msg_length = ubx_.buildMessage(msg_class, msg_id, payload_length, payload, msg_buffer);

  debug_port.print("Example message DEC: ");
  printBuffer(msg_buffer, msg_length, &debug_port);
  debug_port.print("Example message HEX: ");
  printBuffer(msg_buffer, msg_length, &debug_port, HEX);  
}

void loop()
{
  
}

