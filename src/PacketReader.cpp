/*
Copyright (c) 2011, Jonas Tarnstrom and ESN Social Software AB
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:
1. Redistributions of source code must retain the above copyright
   notice, this list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright
   notice, this list of conditions and the following disclaimer in the
   documentation and/or other materials provided with the distribution.
3. All advertising materials mentioning features or use of this software
   must display the following acknowledgement:
   This product includes software developed by ESN Social Software AB (www.esn.me).
4. Neither the name of the ESN Social Software AB nor the
   names of its contributors may be used to endorse or promote products
   derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY ESN SOCIAL SOFTWARE AB ''AS IS'' AND ANY
EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL ESN SOCIAL SOFTWARE AB BE LIABLE FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "PacketReader.h"
#include "mcdefs.h"
#include <assert.h>
#include "socketdefs.h"

#include <stdio.h>

#define BYTEORDER_UINT16(_x) (_x)
#define BYTEORDER_UINT32(_x) (_x)

PacketReader::PacketReader (size_t _cbSize)
{
	m_buffStart = new char[_cbSize];
	m_writeCursor = m_buffStart;
	m_buffEnd = m_buffStart + _cbSize;
	m_readCursor = m_buffStart;
	m_packetEnd = NULL;
}

PacketReader::~PacketReader (void)
{
	delete m_buffStart;
}

void PacketReader::skip()
{
	assert (m_readCursor <= m_packetEnd);

	m_readCursor = m_packetEnd;

	if (m_readCursor == m_writeCursor)
	{
		m_readCursor = m_buffStart;
		m_writeCursor = m_buffStart;
		m_packetEnd = NULL;
	}
}

void PacketReader::reset()
{
	m_readCursor = m_buffStart;
	m_writeCursor = m_buffStart;
	m_packetEnd = NULL;
}


void PacketReader::push(size_t _cbData)
{
	//fprintf (stderr, "%s: Pushing %u bytes\n", __FUNCTION__, _cbData);
	m_writeCursor += _cbData;
}


void PacketReader::setEndPtr (char *ptr)
{
	m_packetEnd = ptr;
}


char *PacketReader::getReadPtr()
{
	return m_readCursor;
}

char *PacketReader::getWritePtr()
{
	return m_writeCursor;
}

char *PacketReader::getStartPtr()
{
	return m_buffStart;
}

char *PacketReader::getEndPtr()
{
	return m_buffEnd;
}

extern void PrintBuffer(FILE *file, void *_offset, size_t len, int perRow);


bool PacketReader::readNumeric (UINT64 *value)
{
	// Scan integer part
	UINT64 intValue = 0;

	while (m_readCursor < m_packetEnd)
	{
		int chr = (int) (unsigned char) *(m_readCursor);

		switch (chr)
		{
		case '0':
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
		case '9':
			intValue = intValue * 10ULL + (UINT64) (chr - 48);
			m_readCursor ++;
			break;

		default:
			goto END_LOOP;
			break;
		}
	}

	END_LOOP:

	if (m_readCursor == m_packetEnd)
	{
		return false;
	}
	
	*value = intValue;
	return true;
}


bool PacketReader::haveLine()
{
	m_packetEnd = NULL;

	char *ptr = m_readCursor;

	while (ptr < m_writeCursor)
	{
		if (*ptr == '\n')
		{
			m_packetEnd = ptr + 1;
			return true;
		}

		ptr ++;
	}

	return false;
}



UINT8 *PacketReader::readUntil(size_t *_outLen, char value)
{
	char *start = m_readCursor;
	char *end = m_readCursor;

	while (end < m_packetEnd && *end != value)
	{
		end ++;
	}

	if (end == m_packetEnd)
	{
		return NULL;
	}

	*_outLen = (end - start);

	m_readCursor = end;

	return (UINT8 *) start;
}

UINT8 *PacketReader::readBytes(size_t cbsize)
{
	if (m_readCursor + cbsize > m_packetEnd)
	{
		return NULL;
	}

	UINT8 *ret = (UINT8 *) m_readCursor;
	m_readCursor += cbsize;

	return ret;
}

size_t PacketReader::getBytesLeft()
{
	return (m_packetEnd - m_readCursor);
}

void PacketReader::rewind(size_t num)
{
	m_readCursor -= num;
}

size_t PacketReader::getSize()
{
	return m_buffEnd - m_buffStart;
}






