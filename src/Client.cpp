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

#include "Client.h"
#include <ctype.h>
#include <stdio.h>
#include <assert.h>

//#define PRINTMARK() fprintf(stderr, "%s: MARK(%d)\n", __FILE__, __LINE__)		
#define PRINTMARK() 		

Client::Client (SOCKETDESC *sockdesc) 
  :	m_writer (1024 * 1200)
  , m_reader (1024 * 1200)
{
  m_sock = sockdesc;
  m_error = "Unspecified error";
}

Client::~Client (void)
{
  disconnect(NULL);
}


void Client::setError(const char *message)
{
  assert (m_error == NULL);
  m_error = message;
}

const char *Client::getError(void)
{
  return m_error;
}

bool Client::connect(const char *address, int port)
{
  m_reader.reset();
  m_writer.reset();

  if (!m_sock->connect (m_sock, address, port))
  {
    setError("connect() failed or timed out");
    return false;
  }

  return true;
}

bool Client::readLine(void)
{
  while (!m_reader.haveLine())
  {
    size_t bytesToRead = m_reader.getEndPtr () - m_reader.getWritePtr();

    if (bytesToRead > 65536)
    {
      bytesToRead = 65536;
    }

    int result = m_sock->recv (m_sock, m_reader.getWritePtr(), bytesToRead);

    if (result == -1)
    {
      disconnect(NULL);
      return false;
    }
    else
    if (result == 0)
    {
      setError("Connection reset by peer");
      return false;
    }

    m_reader.push(result);
  }

  return true;
}

bool Client::sendWriteBuffer(void)
{
  while (true)
  {
    size_t bytesToSend = m_writer.getWriteCursor() - m_writer.getReadCursor();

    if (bytesToSend == 0)
    {
      return true;
    }

    int bytesSent = m_sock->send (m_sock, m_writer.getReadCursor(), bytesToSend);

    if (bytesSent == -1)
    {
      disconnect(NULL);
      return false;
    }
    else
    if (bytesSent == 0)
    {
      setError("Connection reset by peer");
      return false;
    }

    m_writer.pull (bytesSent);
  }
}


bool Client::getResult(char **pData, size_t *cbSize)
{
  if (pData != NULL)
  {

    *pData = m_reader.getReadPtr();

    if (m_reader.getBytesLeft() < 2)
    {
      m_reader.skip();
      return false;
    }

    *cbSize = m_reader.getBytesLeft() - 2;
  }

  m_reader.skip();
  return true;
}

bool Client::cas(const char *key, size_t cbKey, UINT64 casUnique, void *data, size_t cbData, time_t expiration, int flags, bool async)
{

  if (cbData > 1000 * 1000)
  {
    setError("Data size can't be larger than 1000 000 bytes");
    return false;
  }

  m_writer.writeChars("cas ", 4);
  m_writer.writeChars(key, cbKey);
  m_writer.writeChar(' ');
  m_writer.writeNumeric(flags);
  m_writer.writeChar(' ');
  m_writer.writeNumeric(expiration);
  m_writer.writeChar(' ');
  m_writer.writeNumeric(cbData);
  m_writer.writeChar(' ');
  m_writer.writeNumeric(casUnique);

  if (async)
  {
    m_writer.writeChars(" noreply", 8);
  }
  m_writer.writeChars("\r\n", 2);
  m_writer.writeChars(data, cbData);
  m_writer.writeChars("\r\n", 2);

  if (!sendWriteBuffer())
  {
    return false;
  }

  if (async)
  {
    return true;
  }

  if (!readLine())
  {
    return false;
  }

  return true;

}


bool Client::command(const char *cmd, size_t cbCmd, const char *key, size_t cbKey, void *data, size_t cbData, time_t expiration, int flags, bool async)
{
  if (cbData > 1000 * 1000)
  {
    setError("Data size can't be larger than 1000 000 bytes");
    return false;
  }

  m_writer.writeChars(cmd, cbCmd);
  m_writer.writeChar(' ');
  m_writer.writeChars(key, cbKey);
  m_writer.writeChar(' ');
  m_writer.writeNumeric(flags);
  m_writer.writeChar(' ');
  m_writer.writeNumeric(expiration);
  m_writer.writeChar(' ');
  m_writer.writeNumeric(cbData);

  if (async)
  {
    m_writer.writeChars(" noreply", 8);
  }
  m_writer.writeChars("\r\n", 2);
  m_writer.writeChars(data, cbData);
  m_writer.writeChars("\r\n", 2);

  if (!sendWriteBuffer())
  {
    PRINTMARK();
    return false;
  }

  if (async)
  {
    return true;
  }


  if (!readLine())
  {
    PRINTMARK();
    return false;
  }

  return true;
}

bool Client::set(const char *key, size_t cbKey, void *data, size_t cbData, time_t expiration, int flags, bool async)
{
  return command ("set", 3, key, cbKey, data, cbData, expiration, flags, async);
}

bool Client::add(const char *key, size_t cbKey, void *data, size_t cbData, time_t expiration, int flags, bool async)
{
  return command ("add", 3, key, cbKey, data, cbData, expiration, flags, async);
}

bool Client::replace(const char *key, size_t cbKey, void *data, size_t cbData, time_t expiration, int flags, bool async)
{
  return command ("replace", 7, key, cbKey, data, cbData, expiration, flags, async);
}

bool Client::append(const char *key, size_t cbKey, void *data, size_t cbData, time_t expiration, int flags, bool async)
{
  return command ("append", 6, key, cbKey, data, cbData, expiration, flags, async);
}

bool Client::prepend(const char *key, size_t cbKey, void *data, size_t cbData, time_t expiration, int flags, bool async)
{
  return command ("prepend", 7, key, cbKey, data, cbData, expiration, flags, async);
}

bool Client::del(const char *key, size_t cbKey, time_t *expiration, bool async)
{
  m_writer.writeChars("delete ", 7);
  m_writer.writeChars(key, cbKey);

  if (expiration)
  {
    m_writer.writeChar(' ');
    m_writer.writeNumeric(*expiration);
  }

  if (async)
  {
    m_writer.writeChars(" noreply", 8);
  }
  m_writer.writeChars("\r\n", 2);

  if (!sendWriteBuffer())
  {
    return false;
  }

  if (async)
  {
    return true;
  }

  if (!readLine())
  {
    return false;
  }

  return true;
}


bool Client::incr(const char *key, size_t cbKey, UINT64 increment, bool async)
{
  m_writer.writeChars("incr ", 5);
  m_writer.writeChars(key, cbKey);
  m_writer.writeChar(' ');
  m_writer.writeNumeric(increment);

  if (async)
  {
    m_writer.writeChars(" noreply", 8);
  }
  m_writer.writeChars("\r\n", 2);

  if (!sendWriteBuffer())
  {
    return false;
  }

  if (async)
  {
    return true;
  }

  if (!readLine())
  {
    return false;
  }

  return true;
}

bool Client::decr(const char *key, size_t cbKey, UINT64 decrement, bool async)
{
  m_writer.writeChars("decr ", 5);
  m_writer.writeChars(key, cbKey);
  m_writer.writeChar(' ');
  m_writer.writeNumeric(decrement);

  if (async)
  {
    m_writer.writeChars(" noreply", 8);
  }
  m_writer.writeChars("\r\n", 2);

  if (!sendWriteBuffer())
  {
    return false;
  }

  if (async)
  {
    return true;
  }

  if (!readLine())
  {
    return false;
  }

  return true;
}


void Client::getsBegin(void)
{
  m_writer.writeChars("gets", 4);
}

void Client::getBegin(void)
{
  m_writer.writeChars("get", 3);
}

void Client::getKeyWrite(const char *key, size_t cbKey)
{
  m_writer.writeChar(' ');
  m_writer.writeChars(key, cbKey);
}

bool Client::getFlush(void)
{
  m_writer.writeChars("\r\n", 2);
  if (!sendWriteBuffer())
  {
    return false;
  }

  return true;
}

bool Client::version(char **pVersion, size_t *cbVersion)
{
  m_writer.writeChars("version\r\n", 9);

  if (!sendWriteBuffer())
  {
    return false;
  }

  if (!readLine())
  {
    return false;
  }

  if (m_reader.readBytes(8) == NULL)
  {
    return false;
  }	

  *pVersion= (char *) m_reader.readUntil(cbVersion, '\r');

  if (*pVersion == NULL)
  {
    return false;
  }

  m_reader.skip();

  return true;
}

bool Client::stats(const char *arg, size_t cbArg)
{
  m_writer.writeChars("stats", 5);

  if (arg)
  {
    m_writer.writeChar(' ');
    m_writer.writeChars(arg, cbArg);
  }

  m_writer.writeChars("\r\n", 2);

  if (!sendWriteBuffer())
  {
    return false;
  }

  return true;
}

bool Client::getStats(char **pName, size_t *cbName, char **pArg, size_t *cbArg)
{
  if (!readLine())
  {
    return false;
  }

  if (m_reader.readBytes(5) == NULL)
  {
    m_reader.skip();
    return false;
  }

  *pName = (char *) m_reader.readUntil(cbName, ' ');

  if (m_reader.readBytes(1) == NULL)
  {
    return false;
  }

  *pArg = (char *) m_reader.readUntil(cbArg, '\r');
  m_reader.skip();

  return true;

}


bool Client::getReadNext(char **key, size_t *cbKey, char **data, size_t *cbData, int *_flags, UINT64 *_cas, bool *bError)
{
  *bError = false;

  if (!readLine())
  {
    *bError = true;
    return false;
  }

  if (m_reader.readBytes(6) == NULL)
  {
    // "END\r\n" was recieved
    m_reader.skip();
    return false;
  }

  *key = (char *) m_reader.readUntil(cbKey, ' ');

  if (*key == NULL)
  {
    *bError = true;
    return false;
  }

  *(*key + *cbKey) = '\0';


  if (m_reader.readBytes(1) == NULL)
  {
    *bError = true;
    return false;
  }

  UINT64 flags;
  UINT64 bytes;

  if (!m_reader.readNumeric(&flags))
  {
    *bError = true;
    return false;
  }

  *_flags = (int) flags;

  if (m_reader.readBytes(1) == NULL)
  {
    *bError = true;
    return false;
  }

  if (!m_reader.readNumeric(&bytes))
  {
    *bError = true;
    return false;
  }

  if (m_reader.getBytesLeft () > 2)
  {
    UINT64 cas;

    if (m_reader.readBytes(1) == NULL)
    {
      *bError = true;
      return false;
    }

    if (!m_reader.readNumeric(&cas))
    {
      *bError = true;
      return false;
    }

    *_cas = cas;
  }

  m_reader.skip();

  size_t cbExpect = bytes + 2;


  while (m_reader.getWritePtr() - m_reader.getReadPtr() < cbExpect)
  {
    size_t bytesToRead = cbExpect - (m_reader.getWritePtr() - m_reader.getReadPtr());

    if (bytesToRead > 65536)
    {
      bytesToRead = 65536;
    }

    int result = m_sock->recv(m_sock, m_reader.getWritePtr(), bytesToRead);

    if (result < 1)
    {
      *bError = true;
      disconnect(NULL);
      return false;
    }

    m_reader.push(result);
  }

  m_reader.setEndPtr(m_reader.getReadPtr() + bytes + 2);
  *data = (char *) m_reader.readBytes(bytes);
  *cbData = bytes;

  *((*data) + *cbData) = '\0';


  m_reader.skip();

  return true;
}


bool Client::isConnected(void)
{
  return (m_sock->prv) ? true : false;
}


void Client::disconnect(const char *reason)
{
  if (reason)
  {
    setError(reason);
  }

  if (m_sock->prv == NULL)
  {
    return;
  }

  m_sock->destroy(m_sock);
  m_sock->prv = NULL;
}
