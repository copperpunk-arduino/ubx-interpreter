/**
 * @file UbxParser.cpp
 */
#include "UbxParser.h"

#ifdef DEBUG
#define DebugPrint(x) port.print(x)
#define DebugPrintln(x) port.println(x)
#else
#define DebugPrint(x) 
#define DebugPrintln(x) 
#endif

/**
 * This is the default constructor.
 * It constructs things.
 */

UbxParser::UbxParser()
{
    state_ = GOT_NONE;
    msgclass_ = -1;
    msgid_ = -1;
    msglen_ = -1;
    chka_ = 0;
    chkb_ = 0;
    count_ = 0;
}

bool UbxParser::read(Stream *port)
{
    while (port->available())
    {
        if (parse(port->read()))
        {
            return true;
        }
    }
    return false;
}

/**
 * Process the next byte
 * @param[in] parse_byte next byte in the buffer to parse
 * @param[out] result: true if valid message received
 */
bool UbxParser::parse(uint8_t parse_byte)
{
    switch (state_)
    {
    case GOT_NONE:
        if (parse_byte == START_BYTE_1)
        {
            state_ = GOT_START_BYTE1;
        }
        break;
    case GOT_START_BYTE1:
        if (parse_byte == START_BYTE_2)
        {
            state_ = GOT_START_BYTE2;
            chka_ = 0;
            chkb_ = 0;
        }
        else
        {
            DebugPrintln("bad b2");
            state_ = GOT_NONE;
        }
        break;
    case GOT_START_BYTE2:
        msgclass_ = parse_byte;
        state_ = GOT_CLASS;
        addToChecksum(parse_byte);
        break;
    case GOT_CLASS:
        msgid_ = parse_byte;
        state_ = GOT_ID;
        addToChecksum(parse_byte);
        break;
    case GOT_ID:
        state_ = GOT_LENGTH1;
        msglen_ = parse_byte;
        addToChecksum(parse_byte);
        break;
    case GOT_LENGTH1:
        msglen_ += (parse_byte << 8);
        if (msglen_ < kMessageLengthMax)
        {
            state_ = GOT_LENGTH2;
            count_ = 0;
            addToChecksum(parse_byte);
        }
        else
        {
            state_ = GOT_NONE;
            DebugPrintln("Msg length too big");
        }
        break;
    case GOT_LENGTH2:
        if (count_ < kPayloadSize)
        {
            addToChecksum(parse_byte);
            payload_[count_] = parse_byte;
            count_++;

            if (count_ == msglen_)
            {
                state_ = GOT_PAYLOAD;
            }
        }
        else
        {
            DebugPrintln("overrun");
            state_ = GOT_NONE;
        }
        break;
    case GOT_PAYLOAD:
        if (parse_byte == chka_)
        {
            state_ = GOT_CHKA;
            DebugPrintln("good chka");
        }
        else
        {
        state_:
            GOT_NONE;
        }
        break;
    case GOT_CHKA:
        state_ = GOT_NONE;
        if (parse_byte == chkb_)
        {
            DebugPrintln("good msg");
            return true;
        }
        else
        {
            DebugPrintln("bad chkb. exp/rx: " + String(chkb_) + "/" + String(parse_byte));
        }
        break;
    default:
        DebugPrintln("unk :" + String(parse_byte) + "/" + String(state));
        break;
    }
    return false;
}

void UbxParser::addToChecksum(int b)
{
    chka_ = (chka_ + b) & 0xFF;
    chkb_ = (chkb_ + chka_) & 0xFF;
}

uint32_t UbxParser::unpackUint32(int offset)
{
    return (uint32_t)unpack(offset, 4);
}

int32_t UbxParser::unpackInt32(int offset)
{

    return unpack(offset, 4);
}

uint16_t UbxParser::unpackUint16(int offset)
{
    return (uint16_t)unpack(offset, 2);
}

int16_t UbxParser::unpackInt16(int offset)
{
    return (int16_t)unpack(offset, 2);
}

uint8_t UbxParser::unpackUint8(int offset)
{
    return (uint8_t)unpack(offset, 1);
}

int8_t UbxParser::unpackInt8(int offset)
{
    return (int8_t)unpack(offset, 1);
}

int32_t UbxParser::unpack(int offset, int size)
{
    int32_t value = 0;

    for (int k = 0; k < size; ++k)
    {
        value <<= 8;
        value |= (0xFF & payload_[offset + (size - 1) - k]);
    }

    return value;
}

int UbxParser::buildMessage(int msg_class, int msg_id, int payload_length, uint8_t payload[], uint8_t msg_buffer[])
{
    int index = 0;
    msg_buffer[index++] = START_BYTE_1;
    msg_buffer[index++] = START_BYTE_2;
    msg_buffer[index++] = msg_class;
    msg_buffer[index++] = msg_id;
    msg_buffer[index++] = payload_length & 0xFF;
    msg_buffer[index++] = (payload_length >> 8) & 0xFF;
    memcpy(&msg_buffer[index], &payload[0], payload_length);
    index += payload_length;
    uint8_t chka, chkb;
    calculateChecksum(&msg_buffer[2], payload_length+4, chka, chkb);
    msg_buffer[index++] = chka;
    msg_buffer[index] = chkb;

    return payload_length + 8;
}

void UbxParser::calculateChecksum(uint8_t *payload, int payload_length, uint8_t &chka, uint8_t &chkb)
{
    chka = 0;
    chkb = 0;
    for (int i = 0; i < payload_length; ++i)
    {
        chka = (chka + payload[i]) & 0xFF;
        chkb = (chkb + chka) & 0xFF;
    }
}

uint8_t UbxParser::msgClass()
{
    return msgclass_;
}

uint8_t UbxParser::msgId()
{
    return msgid_;
}

void UbxParser::printBuffer(uint8_t msg_buffer[], int msg_length, Stream *port, int output_type)
{
    int i = 0;
    for (; i < msg_length-1; i++)
    {
        port->print(msg_buffer[i], output_type);
        port->print(F(","));
    }
    port->println(msg_buffer[i]);
}