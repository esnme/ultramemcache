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

#ifndef __PACKETREADER_H__
#define __PACKETREADER_H__

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
  bool beginsWithString(const char *str, size_t cbsize);
};

#endif
