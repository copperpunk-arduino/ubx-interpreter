/**
  *
  * UbxInterpreter.h 
  * A C++ class for reading/writing messages that conform to the U-blox UBX protocol
  * 
  * Parsing logic based on the UBX_Parser library written by Simon D. Levy
  * https://github.com/simondlevy/UBX_Parser
  * 
  * 
  * Copyright (C) 2021 Copperpunk
  *
  * This code is free software: you can redistribute it and/or modify
  * it under the terms of the GNU Lesser General Public License as
  * published by the Free Software Foundation, either version 3 of the
  * License, or (at your option) any later version.
  *
  * This code is distributed in the hope that it will be useful,
  * but WITHOUT ANY WARRANTY without even the implied warranty of
  * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  * GNU General Public License for more details.
  *
  * You should have received a copy of the GNU Lesser General Public License
  * along with this code.  If not, see <http://www.gnu.org/licenses/>.
  */

#ifndef _UBXINTERPRETER_H_
#define _UBXINTERPRETER_H_

#include "Arduino.h"

#define START_BYTE_1 0xB5
#define START_BYTE_2 0x62
#define PAYLOAD_OFFSET 6			 // SB1, SB2, Class, ID, Len LSB, Len MSB
#define PAYLOAD_LENGTH_ADDED_BYTES 8 // Additional bytes wrt to the payload: SB1, SB2, Class, Id, Len LSB, Len MSB, CK_A, CK_B

const int kBufferSize = 100;
const int kPayloadSize = 100;
const int kMessageLengthMax = kPayloadSize - 7;

class UbxInterpreter
{
public:
	UbxInterpreter();
	// ---------------- RX -------------------
	enum ParseState
	{
		GOT_NONE = 0,
		GOT_START_BYTE1,
		GOT_START_BYTE2,
		GOT_CLASS,
		GOT_ID,
		GOT_LENGTH1,
		GOT_LENGTH2,
		GOT_PAYLOAD,
		GOT_CHKA,
		GOT_MESSAGE
	};

	bool read(Stream *port);
	ParseState parse(uint8_t next_byte);
	uint32_t unpackUint32(int offset);
	int32_t unpackInt32(int offset);
	uint16_t unpackUint16(int offset);
	int16_t unpackInt16(int offset);
	uint8_t unpackUint8(int offset);
	int8_t unpackInt8(int offset);
	uint8_t msgClass() { return _rx_msg_class; };
	uint8_t msgId() { return _rx_msg_id; };

	// ---------------- TX -------------------
	void setHeaderValues(uint8_t msg_class, uint8_t msg_id, uint16_t payload_length);
	void prepareMessage();
	int writeMessage(Stream *port);
	int forwardLastMessageReceived(Stream *port);
	void printWriteBuffer(Stream *port, int output_type = DEC);
	int messageBuffer(uint8_t msg_buffer[]);

	template <typename T>
	void packValue(T value, int offset)
	{
		memcpy(&_tx_buffer[PAYLOAD_OFFSET + offset], &value, sizeof(value));
	}

private:
	//---------- Rx ------------//
	void addToChecksum(int b);
	int32_t unpack(int offset, int size);

	ParseState _state;
	uint8_t _rx_msg_class;
	uint8_t _rx_msg_id;
	int _rx_payload_length;
	uint8_t _chka;
	uint8_t _chkb;
	uint8_t _payload[kPayloadSize];
	uint16_t _count;
	//---------- Tx ------------//
	void calculateChecksum(uint8_t payload[], int payload_length, uint8_t &chka, uint8_t &chkb);

	uint8_t _tx_buffer[kBufferSize];
	uint16_t _tx_payload_length = 0;
	int _tx_buffer_write_length = 0;
};
#endif