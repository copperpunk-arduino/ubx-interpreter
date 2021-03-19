/**
  *
  * UbxParser.h - A C++ class for parsing UBX messages from Ublox GPS
  * 
  * Based on the UBX_Parser library written by Simon D. Levy
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

/**
	* A class for parsing UBX messages.
	*/
#ifndef _UBXPARSER_H_
#define _UBXPARSER_H_

#include "Arduino.h"

#define START_BYTE_1 0xB5
#define START_BYTE_2 0x62

const int kBufferSize = 100;
const int kPayloadSize = 100;
const int kMessageLengthMax = kPayloadSize-7;
class UbxParser
{
public:
	UbxParser();
	bool Read(Stream *port);
	int BuildMessage(int msg_class, int msg_id, int payload_length, uint8_t payload[], uint8_t msg_buffer[]);
	void CalculateChecksum(uint8_t payload[], int payload_length, uint8_t &chka, uint8_t &chkb);
	static void PrintBuffer(uint8_t msg_buffer[], int msg_length, Stream *port, int output_type=DEC);
	uint8_t MsgClass();
	uint8_t MsgId();

protected:	
	uint8_t read_buffer_[kBufferSize];

	typedef enum
	{
		GOT_NONE,
		GOT_START_BYTE1,
		GOT_START_BYTE2,
		GOT_CLASS,
		GOT_ID,
		GOT_LENGTH1,
		GOT_LENGTH2,
		GOT_PAYLOAD,
		GOT_CHKA
	} state_t;

	state_t state_;
	uint8_t msgclass_;
	uint8_t msgid_;
	int msglen_;
	uint8_t chka_;
	uint8_t chkb_;
	int count_;
	uint8_t payload_[kPayloadSize];

	bool Parse(uint8_t b);
	void AddToChecksum(int b);
	uint32_t UnpackUint32(int offset);
	int32_t UnpackInt32(int offset);
	uint16_t UnpackUint16(int offset);
	int16_t UnpackInt16(int offset);
	uint8_t UnpackUint8(int offset);
	int8_t UnpackInt8(int offset);
	int32_t Unpack(int offset, int size);
};
#endif