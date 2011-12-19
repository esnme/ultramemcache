#ifndef __AMPACKETREADER_H__
#define __AMPACKETREADER_H__

#include "mcdefs.h"

class PacketReader
{
private:
	char *m_buffStart;
	char *m_buffEnd;
	char *m_readCursor;
	char *m_writeCursor;
	char *m_packetEnd;

public:

	PacketReader (size_t cbSize);
	~PacketReader (void);

	void reset();
	void skip();
	void push(size_t _cbData);
	char *getReadPtr();
	char *getWritePtr();
	char *getStartPtr();
	char *getEndPtr();
	void setEndPtr (char *ptr);
	size_t getSize();
	bool haveLine();
	
	UINT8 *readUntil(size_t *_outLen, char value);
	UINT8 *readBytes(size_t cbsize);

	bool readNumeric (UINT64 *value);

	size_t getBytesLeft();
	void rewind(size_t num);
};

#endif