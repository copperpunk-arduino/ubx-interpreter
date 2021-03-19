#include "UbxParser.h"

#define debug_port Serial
#define data_port Serial1

UbxParser ubx_;

int msg_class_ = 10;
int msg_id_ = 100;
const int kPayloadLength = 4;
byte payload_[kPayloadLength] = {1, 2, 3, 4};
byte msg_buffer_[100];
void setup()
{
  debug_port.begin(115200);
  while (!debug_port) {}
  data_port.begin(115200);
  while(!data_port);
}

void loop()
{
  int msg_length = ubx_.BuildMessage(msg_class_, msg_id_, kPayloadLength, payload_, msg_buffer_);
  data_port.write(msg_buffer_, msg_length);
  if(ubx_.Read(&data_port)) {
    debug_port.print("Message received!");
    debug_port.println("\tMsg Class/ID: " + String(ubx_.MsgClass()) + "/" + String(ubx_.MsgId()));
  }
  msg_class_++;
  msg_id_+=2;
  delay(1000);
}
