/**
  *
  * UbxParser.h - A C++ class for parsing UBX messages from Ublox GPS
  * 
  * Based on the UbxParser library written by Simon D. Levy
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

#define kBufferSize 100
#define kPayloadSize 100
#define RAD2DEG 57.2957795130
#define DEG2RAD 0.0174532925


// #define debug_port Serial
class UbxParser
{

public:
	UbxParser();
	bool Read(Stream *port);

protected:	
	unsigned char read_buffer_[kBufferSize];

	typedef enum
	{
		GOT_NONE,
		GOT_SYNC1,
		GOT_SYNC2,
		GOT_CLASS,
		GOT_ID,
		GOT_LENGTH1,
		GOT_LENGTH2,
		GOT_PAYLOAD,
		GOT_CHKA
	} state_t;

	state_t state_;
	int msgclass_;
	int msgid_;
	int msglen_;
	unsigned char chka_;
	unsigned char chkb_;
	int count_;
	unsigned char payload_[kPayloadSize];

	bool Parse(int b);
	void AddToChecksum(int b);
	uint32_t UnpackUint32(int offset);
	int32_t UnpackInt32(int offset);
	uint16_t UnpackUint16(int offset);
	int16_t UnpackInt16(int offset);
	uint8_t UnpackUint8(int offset);
	int8_t UnpackInt8(int offset);
	long Unpack(int offset, int size);
};
#endif