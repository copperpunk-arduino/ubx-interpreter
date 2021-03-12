#include "UBX_Parser.h"

UBX_Parser::UBX_Parser()
{
    state_ = GOT_NONE;
    msgclass_ = -1;
    msgid_ = -1;
    msglen_ = -1;
    chka_ = 0;
    chkb_ = 0;
    count_ = 0;
}

bool UBX_Parser::Read(Stream *port)
{
    while (port->available())
    {
        if (Parse(port->read()))
        {
            return true;
        }
    }
    return false;
}


bool UBX_Parser::Parse(int b)
{
    // debug_port.print(b, HEX);
    // debug_port.print(",");
    // debug_port.println(state_);
    switch (state_)
    {
    case GOT_NONE:
        if (b == 0xB5)
        {
            state_ = GOT_SYNC1;
        }
        break;
    case GOT_SYNC1:
        if (b == 0x62)
        {
            state_ = GOT_SYNC2;
            chka_ = 0;
            chkb_ = 0;
        }
        else
        {
            // debug_port.println("bad b2");
            state_ = GOT_NONE;
        }
        break;
    case GOT_SYNC2:
        msgclass_ = b;
        state_ = GOT_CLASS;
        AddToChecksum(b);
        break;
    case GOT_CLASS:
        msgid_ = b;
        state_ = GOT_ID;
        AddToChecksum(b);
        break;
    case GOT_ID:
        state_ = GOT_LENGTH1;
        msglen_ = b;
        AddToChecksum(b);
        break;
    case GOT_LENGTH1:
        msglen_ += (b << 8);
        if (msglen_ < 93)
        {
            state_ = GOT_LENGTH2;
            count_ = 0;
            AddToChecksum(b);
        }
        else
        {
            state_ = GOT_NONE;
            // debug_port.println("bad len");
        }
        break;
    case GOT_LENGTH2:
        if (count_ < kPayloadSize)
        {
            AddToChecksum(b);
            payload_[count_] = b;
            count_++;

            if (count_ == msglen_)
            {
                state_ = GOT_PAYLOAD;
            }
        }
        else
        {
            // debug_port.println("overrun" + String(msgid));
            state_ = GOT_NONE;
        }
        break;
    case GOT_PAYLOAD:
        state_ = (b == chka_) ? GOT_CHKA : GOT_NONE;
        break;
    case GOT_CHKA:
        state_ = GOT_NONE;
        if (b == chkb_)
        {
            // debug_port.println("good " + String(msgid_));
            return true;
        }
        // else
        // {
        //     debug_port.println("bad " + String(msgid_));
        // }
        break;
    default:
        break;
    }
    // else
    // {
    //     debug_port.println("unk :" + String(b) + "/" + String(state));
    // }
    return false;
}

void UBX_Parser::AddToChecksum(int b)
{
    chka_ = (chka_ + b) & 0xFF;
    chkb_ = (chkb_ + chka_) & 0xFF;
}

uint32_t UBX_Parser::UnpackUint32(int offset)
{
    return (uint32_t)Unpack(offset, 4);
}

int32_t UBX_Parser::UnpackInt32(int offset)
{

    return Unpack(offset, 4);
}

uint16_t UBX_Parser::UnpackUint16(int offset)
{
    return (uint16_t)Unpack(offset, 2);
}

int16_t UBX_Parser::UnpackInt16(int offset)
{
    return (int16_t)Unpack(offset, 2);
}

uint8_t UBX_Parser::UnpackUint8(int offset)
{
    return (uint8_t)Unpack(offset, 1);
}

int8_t UBX_Parser::UnpackInt8(int offset)
{
    return (int8_t)Unpack(offset, 1);
}

long UBX_Parser::Unpack(int offset, int size)
{
    long value = 0; // four bytes on most Arduinos

    for (int k = 0; k < size; ++k)
    {
        value <<= 8;
        value |= (0xFF & payload_[offset + (size - 1) - k]);
    }

    return value;
}