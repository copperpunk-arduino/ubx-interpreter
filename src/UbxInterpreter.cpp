#include "UbxInterpreter.h"

UbxInterpreter::UbxInterpreter()
{
    _state = GOT_NONE;
    _rx_msg_class = -1;
    _rx_msg_id = -1;
    _rx_payload_length = -1;
    _chka = 0;
    _chkb = 0;
    _count = 0;

    _tx_buffer[0] = START_BYTE_1;
    _tx_buffer[1] = START_BYTE_2;
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
    switch (_state)
    {
    case GOT_NONE:
        if (parse_byte == START_BYTE_1)
        {
            _state = GOT_START_BYTE1;
        }
        break;
    case GOT_START_BYTE1:
        if (parse_byte == START_BYTE_2)
        {
            _state = GOT_START_BYTE2;
            _chka = 0;
            _chkb = 0;
        }
        else
        {
            _state = GOT_NONE;
        }
        break;
    case GOT_START_BYTE2:
        _rx_msg_class = parse_byte;
        _state = GOT_CLASS;
        addToChecksum(parse_byte);
        break;
    case GOT_CLASS:
        _rx_msg_id = parse_byte;
        _state = GOT_ID;
        addToChecksum(parse_byte);
        break;
    case GOT_ID:
        _state = GOT_LENGTH1;
        _rx_payload_length = parse_byte;
        addToChecksum(parse_byte);
        break;
    case GOT_LENGTH1:
        _rx_payload_length += (parse_byte << 8);
        if (_rx_payload_length < kMessageLengthMax)
        {
            _state = _rx_payload_length > 0 ? GOT_LENGTH2 : GOT_PAYLOAD;
            _count = 0;
            addToChecksum(parse_byte);
        }
        else
        {
            _state = GOT_NONE;
        }
        break;
    case GOT_LENGTH2:
        if (_count < kPayloadSize)
        {
            addToChecksum(parse_byte);
            _payload[_count] = parse_byte;
            ++_count;

            if (_count == _rx_payload_length)
            {
                _state = GOT_PAYLOAD;
            }
        }
        else
        {
            _state = GOT_NONE;
        }
        break;
    case GOT_PAYLOAD:
        _state = (parse_byte == _chka) ? GOT_CHKA : GOT_NONE;
        break;
    case GOT_CHKA:
        _state = GOT_NONE;
        if (parse_byte == _chkb)
        {
            return GOT_MESSAGE;
        }
        break;
    default:
        break;
    }
    // Serial.print(parse_byte);
    // Serial.print(F("/"));
    // Serial.println(state_);
    return _state;
}

void UbxInterpreter::addToChecksum(int b)
{
    _chka = (_chka + b) & 0xFF;
    _chkb = (_chkb + _chka) & 0xFF;
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
        value |= (0xFF & _payload[offset + (size - 1) - k]);
    }

    return value;
}

void UbxInterpreter::setHeaderValues(uint8_t msg_class, uint8_t msg_id, uint16_t payload_length)
{
    _tx_buffer[2] = msg_class;
    _tx_buffer[3] = msg_id;
    _tx_buffer[4] = payload_length & 0xFF;
    _tx_buffer[5] = (payload_length >> 8) & 0xFF;
    _tx_payload_length = payload_length;
    _tx_buffer_write_length = payload_length + PAYLOAD_LENGTH_ADDED_BYTES;
}

void UbxInterpreter::prepareMessage()
{
    uint8_t chka, chkb;
    calculateChecksum(&_tx_buffer[2], _tx_payload_length + 4, chka, chkb);
    _tx_buffer[_tx_payload_length + 6] = chka;
    _tx_buffer[_tx_payload_length + 7] = chkb;
}

int UbxInterpreter::writeMessage(Stream *port)
{
    return (port->write(_tx_buffer, _tx_buffer_write_length));
}

int UbxInterpreter::forwardLastMessageReceived(Stream *port)
{
    _tx_buffer[2] = _rx_msg_class;
    _tx_buffer[3] = _rx_msg_id;
    _tx_buffer[4] = _rx_payload_length & 0xFF;
    _tx_buffer[5] = (_rx_payload_length >> 8) & 0xFF;
    memcpy(&_tx_buffer[6], _payload, _rx_payload_length);
    _tx_payload_length = _rx_payload_length;
    _tx_buffer[_tx_payload_length + 6] = _chka;
    _tx_buffer[_tx_payload_length + 7] = _chkb;
    _tx_buffer_write_length = _tx_payload_length + PAYLOAD_LENGTH_ADDED_BYTES;
    return (port->write(_tx_buffer, _tx_buffer_write_length));
}

int UbxInterpreter::messageBuffer(uint8_t msg_buffer[])
{
    memcpy(msg_buffer, _tx_buffer, _tx_buffer_write_length);
    return _tx_buffer_write_length;
}

void UbxInterpreter::printWriteBuffer(Stream *port, int output_type)
{
    int i = 0;
    for (; i < _tx_buffer_write_length - 1; ++i)
    {
        port->print(_tx_buffer[i], output_type);
        port->print(F(","));
    }
    port->println(_tx_buffer[i]);
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