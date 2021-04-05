#include "UbxInterpreter.h"

UbxInterpreter::UbxInterpreter()
{
    state_ = GOT_NONE;
    rx_msg_class_ = -1;
    rx_msg_id_ = -1;
    rx_msg_len_ = -1;
    chka_ = 0;
    chkb_ = 0;
    count_ = 0;

    tx_buffer_[0] = START_BYTE_1;
    tx_buffer_[1] = START_BYTE_2;
}

bool UbxInterpreter::read(Stream *port)
{
    while (port->available())
    {
        if (parse(port->read()) == GOT_MESSAGE)
        {
            return true;
        }
    }
    return false;
}

UbxInterpreter::ParseState UbxInterpreter::parse(uint8_t parse_byte)
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
            state_ = GOT_NONE;
        }
        break;
    case GOT_START_BYTE2:
        rx_msg_class_ = parse_byte;
        state_ = GOT_CLASS;
        addToChecksum(parse_byte);
        break;
    case GOT_CLASS:
        rx_msg_id_ = parse_byte;
        state_ = GOT_ID;
        addToChecksum(parse_byte);
        break;
    case GOT_ID:
        state_ = GOT_LENGTH1;
        rx_msg_len_ = parse_byte;
        addToChecksum(parse_byte);
        break;
    case GOT_LENGTH1:
        rx_msg_len_ += (parse_byte << 8);
        if (rx_msg_len_ < kMessageLengthMax)
        {
            state_ = GOT_LENGTH2;
            count_ = 0;
            addToChecksum(parse_byte);
        }
        else
        {
            state_ = GOT_NONE;
        }
        break;
    case GOT_LENGTH2:
        if (count_ < kPayloadSize)
        {
            addToChecksum(parse_byte);
            payload_[count_] = parse_byte;
            count_++;

            if (count_ == rx_msg_len_)
            {
                state_ = GOT_PAYLOAD;
            }
        }
        else
        {
            state_ = GOT_NONE;
        }
        break;
    case GOT_PAYLOAD:
        if (parse_byte == chka_)
        {
            state_ = GOT_CHKA;
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
            return GOT_MESSAGE;
        }
        break;
    default:
        break;
    }
    return state_;
}

void UbxInterpreter::addToChecksum(int b)
{
    chka_ = (chka_ + b) & 0xFF;
    chkb_ = (chkb_ + chka_) & 0xFF;
}

uint32_t UbxInterpreter::unpackUint32(int offset)
{
    return (uint32_t)unpack(offset, 4);
}

int32_t UbxInterpreter::unpackInt32(int offset)
{

    return unpack(offset, 4);
}

uint16_t UbxInterpreter::unpackUint16(int offset)
{
    return (uint16_t)unpack(offset, 2);
}

int16_t UbxInterpreter::unpackInt16(int offset)
{
    return (int16_t)unpack(offset, 2);
}

uint8_t UbxInterpreter::unpackUint8(int offset)
{
    return (uint8_t)unpack(offset, 1);
}

int8_t UbxInterpreter::unpackInt8(int offset)
{
    return (int8_t)unpack(offset, 1);
}

int32_t UbxInterpreter::unpack(int offset, int size)
{
    int32_t value = 0;

    for (int k = 0; k < size; ++k)
    {
        value <<= 8;
        value |= (0xFF & payload_[offset + (size - 1) - k]);
    }

    return value;
}

uint8_t UbxInterpreter::msgClass()
{
    return rx_msg_class_;
}

uint8_t UbxInterpreter::msgId()
{
    return rx_msg_id_;
}

void UbxInterpreter::setHeaderValues(uint8_t msg_class, uint8_t msg_id, uint16_t payload_length)
{
    tx_buffer_[2] = msg_class;
    tx_buffer_[3] = msg_id;
    tx_buffer_[4] = payload_length & 0xFF;
    tx_buffer_[5] = (payload_length >> 8) & 0xFF;
    tx_payload_length_ = payload_length;
    tx_buffer_write_length_ = payload_length + PAYLOAD_LENGTH_ADDED_BYTES;
}

void UbxInterpreter::prepareMessage()
{
    uint8_t chka, chkb;
    calculateChecksum(&tx_buffer_[2], tx_payload_length_ + 4, chka, chkb);
    tx_buffer_[tx_payload_length_ + 6] = chka;
    tx_buffer_[tx_payload_length_ + 7] = chkb;
}

int UbxInterpreter::writeMessage(Stream *port)
{
    return (port->write(tx_buffer_, tx_buffer_write_length_));
}

int UbxInterpreter::messageBuffer(uint8_t msg_buffer[])
{
    memcpy(msg_buffer, tx_buffer_, tx_buffer_write_length_);
    return tx_buffer_write_length_;
}

void UbxInterpreter::printWriteBuffer(Stream *port, int output_type)
{
    int i = 0;
    for (; i < tx_buffer_write_length_ - 1; i++)
    {
        port->print(tx_buffer_[i], output_type);
        port->print(F(","));
    }
    port->println(tx_buffer_[i]);
}
void UbxInterpreter::calculateChecksum(uint8_t *payload, int payload_length, uint8_t &chka, uint8_t &chkb)
{
    chka = 0;
    chkb = 0;
    for (int i = 0; i < payload_length; ++i)
    {
        chka = (chka + payload[i]) & 0xFF;
        chkb = (chkb + chka) & 0xFF;
    }
}