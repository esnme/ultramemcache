#include "PacketWriter.h"
#include <assert.h>

#define BYTEORDER_UINT16(_x) (_x)
#define BYTEORDER_UINT32(_x) (_x)

#include <ctype.h>
#include <stdio.h>
#include <string.h>

PacketWriter::PacketWriter(size_t _cbSize)
{
	m_buffStart = new char[_cbSize];
	m_buffEnd = m_buffStart + _cbSize;
	m_readCursor = m_buffStart;
	m_writeCursor = m_buffStart;
}

PacketWriter::~PacketWriter(void)
{
	delete m_buffStart;
}

// Push/increment write cursor
void PacketWriter::push(void *data, size_t cbData)
{
	assert (m_writeCursor + cbData < m_buffEnd);

	memcpy (m_writeCursor, data, cbData);
	m_writeCursor += cbData; 
}

// Pull/Increment read cursor
void PacketWriter::pull(size_t cbSize)
{
	assert (cbSize <= (m_writeCursor - m_readCursor));
	m_readCursor += cbSize;

	if (m_readCursor == m_writeCursor)
	{
		m_readCursor = m_buffStart;
		m_writeCursor = m_buffStart;
	}
}

char *PacketWriter::getStart()
{
	return m_buffStart;
}

char *PacketWriter::getEnd()
{
	return m_buffEnd;
}

char *PacketWriter::getReadCursor()
{
	return m_readCursor;
}

char *PacketWriter::getWriteCursor()
{
	return m_writeCursor;
}

bool PacketWriter::isDone()
{
	return (m_readCursor == m_writeCursor);
}

void PacketWriter::reset()
{
	m_readCursor = m_buffStart;
	m_writeCursor = m_buffStart;
}

void strreverse(char* begin, char* end)
{
	char aux;
	while (end > begin)
	aux = *end, *end-- = *begin, *begin++ = aux;
}
	
void PacketWriter::writeChar (const char _value)
{
	*(m_writeCursor++) = _value;
}

void PacketWriter::writeChars (const void *data, size_t cbData)
{
	memcpy (m_writeCursor, data, cbData);
	m_writeCursor += cbData;
}

void PacketWriter::writeNumeric (UINT64 value)
{
	char* wstr;
	wstr = m_writeCursor;
	// Conversion. Number is reversed.
	
	do *wstr++ = (char)(48 + (value % 10ULL)); while(value /= 10ULL);
	if (value < 0) *wstr++ = '-';

	// Reverse string
	strreverse(m_writeCursor,wstr - 1);
	m_writeCursor += (wstr - (m_writeCursor));
}


void PrintBuffer(FILE *file, void *_offset, size_t len, int perRow)
{
	size_t cnt = 0;

	char *offset = (char *) _offset;
	char *end = offset + len;
	
	int orgPerRow = perRow;

	fprintf (file, "%u %p --------------\n", len, _offset);

	while (offset != end)
	{
		fprintf (file, "%08x: ", cnt);

		if (end - offset < perRow)
		{
			perRow = end - offset;
		}

		for (int index = 0; index < perRow; index ++)
		{
			int chr = (unsigned char) *offset;

			if (isprint(chr))
			{
				fprintf (file, "%c", chr);
			}
			else
			{
				fprintf (file, ".");
			}

			offset ++;
		}

		offset -= perRow;

		for (int index = perRow; index < orgPerRow; index ++)
		{
			fprintf (file, " ");
		}

		fprintf (file, "    ");

		for (int index = 0; index < perRow; index ++)
		{
			int chr = (unsigned char) *offset;

			fprintf (file, "%02x ", chr);
			offset ++;
		}

		fprintf (file, "\n");

		cnt += perRow;
	}
}

void PacketWriter::finalize(int packetNumber)
{
#ifdef __FIX_IT_OR_REMOVE__
	size_t packetLen = (m_writeCursor - m_readCursor - MYSQL_PACKET_HEADER_SIZE);

	*((UINT32 *)m_buffStart) = packetLen;
	*((UINT8 *)m_buffStart + 3) = packetNumber;

	//PrintBuffer (stdout, m_readCursor, (m_writeCursor - m_readCursor), 16);
#endif
}

size_t PacketWriter::getSize(void)
{
	return (m_buffEnd - m_buffStart);
}
