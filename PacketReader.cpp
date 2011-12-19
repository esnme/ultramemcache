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






