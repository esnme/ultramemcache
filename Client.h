#pragma once

#include "mcdefs.h"
#include "PacketWriter.h"
#include "PacketReader.h"

typedef struct SOCKETDESC
{
	void *prv;
	int (*send)(SOCKETDESC *desc, void *data, size_t cbData);
	int (*recv)(SOCKETDESC *desc, void *data, size_t cbMaxData);
	void (*destroy)(SOCKETDESC *desc);
	int (*connect)(SOCKETDESC *desc, const char *address, int port);
} _SOCKETDESC;

class Client
{
public:

	enum RESULT
	{
		STORED,
		NOT_STORED,
		EXISTS,
		NOT_FOUND,
	};

public:
	

	Client (SOCKETDESC *sockdesc);
	~Client (void);
	bool connect(const char *address, int port);
	bool isConnected(void);
	void disconnect(const char *reason);


	void getsBegin(void);
	void getBegin(void);
	void getKeyWrite(const char *key, size_t cbKey);
	bool getFlush(void);
	bool getReadNext(char **key, size_t *cbKey, char **data, size_t *cbData, int *flags, UINT64 *cas, bool *bError);


	bool set(const char *key, size_t cbKey, void *data, size_t cbData, time_t expiration, int flags, bool async);
	bool del(const char *key, size_t cbKey, time_t *expiration, bool async);
	bool add(const char *key, size_t cbKey, void *data, size_t cbData, time_t expiration, int flags, bool async);
	
	bool replace(const char *key, size_t cbKey, void *data, size_t cbData, time_t expiration, int flags, bool async);
	bool append(const char *key, size_t cbKey, void *data, size_t cbData, time_t expiration, int flags, bool async);
	bool prepend(const char *key, size_t cbKey, void *data, size_t cbData, time_t expiration, int flags, bool async);
	
	bool cas(const char *key, size_t cbKey, UINT64 casUnique, void *data, size_t cbData, time_t expiration, int flags, bool async);
	bool incr(const char *key, size_t cbKey, UINT64 increment, bool async);
	bool decr(const char *key, size_t cbKey, UINT64 decrement, bool async);
	bool version(char **version, size_t *cbVersion);
	bool stats(const char *arg, size_t cbArg);
	bool getStats(char **pName, size_t *cbName, char **pArg, size_t *cbArg);
	bool getResult(char **pData, size_t *cbSize);
	const char *getError(void);

private:
	bool command(const char *cmd, size_t cbCmd, const char *key, size_t cbKey, void *data, size_t cbData, time_t expiration, int flags, bool async);
	bool sendWriteBuffer(void);
	bool readLine(void);

	void setError(const char *message);

private:
	SOCKETDESC *m_sock;
	PacketWriter m_writer;
	PacketReader m_reader;
	
	const char *m_error;
};