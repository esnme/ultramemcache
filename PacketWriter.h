#ifndef __AMPACKETWRITER_H__
#define __AMPACKETWRITER_H__

#include "socketdefs.h"
#include "mcdefs.h"

class PacketWriter
{
public:
	PacketWriter(size_t _cbSize);
	~PacketWriter(void);

	// Push/increment write cursor
	void push(void *data, size_t cbData);

	// Pull/Increment read cursor
	void pull(size_t cbSize);

	char *getStart();
	char *getEnd();
	char *getReadCursor();
	char *getWriteCursor();
	bool isDone();
	void reset();

	void writeChars (const void *data, size_t cbData);
	void writeChar (const char _value);
	void writeNumeric (UINT64 value);

	void finalize(int packetNumber);

	size_t getSize(void);


private:
	char *m_buffStart;
	char *m_buffEnd;
	char *m_readCursor;
	char *m_writeCursor;


};

#endif
