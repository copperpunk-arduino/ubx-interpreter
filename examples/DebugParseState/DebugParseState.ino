#include "UbxInterpreter.h"

#define debug_port Serial
#define data_port Serial1

UbxInterpreter ubx_;

int msg_class_ = 10;
int msg_id_ = 100;
byte msg_buffer_[100];

#define VALUE_1_OFFSET 0
#define VALUE_2_OFFSET 2
uint16_t value1_ = 100;
int16_t value2_ = -200;
const int kPayloadLength = 4;

void setup()
{
    debug_port.begin(115200);
    while (!debug_port)
    {
    }
    data_port.begin(115200);
    while (!data_port)
        ;
    // Clear data_port buffer
    while (data_port.available())
    {
        data_port.read();
    }
}

/**
 * Examine how the parse state changes as it moves through the message buffer
 * This can be useful if your messages are not getting processed correctly
*/

void loop()
{
    // Until the first start byte is received, the expected ParseState is GOT_NONE (0)
    sendNoise();
    buildMessage();
    sendMessage();
    delay(10);
    while (data_port.available())
    {
        uint8_t b = data_port.read();
        UbxInterpreter::ParseState parse_state = ubx_.parse(b);
        debug_port.print(F("byte/parse state: "));
        debug_port.print(b, HEX);
        debug_port.print(F("/"));
        debug_port.println(parse_state);
        if (parse_state == UbxInterpreter::ParseState::GOT_MESSAGE)
        {
            debug_port.print("Message rx'd with class ");
            debug_port.print(ubx_.msgClass());
            debug_port.print(F(" and ID "));
            debug_port.print(ubx_.msgId());
            debug_port.print(F("\n"));
            debug_port.print("Value1 = ");
            debug_port.print(ubx_.unpackUint16(VALUE_1_OFFSET));
            debug_port.print(F(" and Value2 = "));
            debug_port.print(ubx_.unpackInt16(VALUE_2_OFFSET));
            debug_port.print(F("\n\n"));
        }
    }
    msg_class_++;
    msg_id_ += 2;
    value1_ += 0x4000;
    value2_ += 0x4000;

    delay(1000);
}

void sendNoise()
{
    // Bytes that are not part of our complete message
    uint8_t buf[6] = {0, 1, 2, 3, 4, 5};
    data_port.write(buf, 6);
}

void buildMessage()
{
    ubx_.setHeaderValues(msg_class_, msg_id_, kPayloadLength);
    ubx_.packValue(value1_, VALUE_1_OFFSET);
    ubx_.packValue(value2_, VALUE_2_OFFSET);
    ubx_.prepareMessage();
}

void sendMessage()
{
    debug_port.print(F("Send message with class "));
    debug_port.print(msg_class_);
    debug_port.print(F(" and ID "));
    debug_port.println(msg_id_);
    ubx_.writeMessage(&data_port);
}