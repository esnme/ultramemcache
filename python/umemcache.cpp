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

#define PY_SSIZE_T_CLEAN
#include <Python.h>
#include "structmember.h"
#include "Client.h"
#include <string.h>
#include <stdio.h>

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

#ifndef alloca
#define alloca _alloca
#endif

#ifndef MAX_ITEM_SIZE
#define MAX_ITEM_SIZE 1000000
#endif

//#define PRINTMARK() fprintf(stderr, "%s: MARK(%d)\n", __FILE__, __LINE__)
#define PRINTMARK()

typedef struct {
  PyObject_HEAD
    Client *client;
  SOCKETDESC desc;
  PyObject *sock;
  PyObject *host;
  int port;
  size_t maxSize;
} PyClient;

static PyObject *umemcache_MemcachedError;

int API_send(SOCKETDESC *desc, void *data, size_t cbData)
{
  PyClient *client = (PyClient *) desc->prv;

  if (client == NULL)
  {
    PyErr_Format(PyExc_IOError, "No client object");
    PRINTMARK();
    return -1;
  }

  PyObject *res;
  PyObject *pybuffer;
  PyObject *funcStr;
  int ret;

  funcStr = PyString_FromString("send");
  pybuffer = PyString_FromStringAndSize( (char *) data, cbData);
  res = PyObject_CallMethodObjArgs (client->sock, funcStr, pybuffer, NULL);
  Py_DECREF(funcStr);
  Py_DECREF(pybuffer);

  if (res == NULL)
  {
    return -1;
  }

  ret = (int) PyInt_AsLong(res);
  Py_DECREF(res);
  return ret;
}

int API_recv(SOCKETDESC *desc, void *data, size_t cbMaxData)
{
  PyClient *client = (PyClient *) desc->prv;

  if (client == NULL)
  {
    PyErr_Format(PyExc_IOError, "No client object");

    PRINTMARK();
    return -1;
  }

  PyObject *res;
  PyObject *bufSize;
  PyObject *funcStr;
  int ret;

  funcStr = PyString_FromString("recv");
  bufSize = PyInt_FromLong(cbMaxData);
  res = PyObject_CallMethodObjArgs (client->sock, funcStr, bufSize, NULL);
  Py_DECREF(funcStr);
  Py_DECREF(bufSize);

  if (res == NULL)
  {
    return -1;
  }

  ret = (int) PyString_GET_SIZE(res);
  memcpy (data, PyString_AS_STRING(res), ret);
  Py_DECREF(res);
  return ret;
}

void API_destroy(SOCKETDESC *desc)
{
  PyClient *client = (PyClient *) desc->prv;
  Py_DECREF(client->sock);
  client->sock = NULL;

}

int API_connect(SOCKETDESC *desc, const char *address, int port)
{
  PyClient *client = (PyClient *) desc->prv;

  char strTemp[256 + 1];
  PRINTMARK();
  snprintf (strTemp, 256, "%s:%d", address, port);
  PRINTMARK();

  PRINTMARK();

  PyObject *args = PyTuple_New(2);
  //Increment client->host before dropping into tuple
  Py_INCREF(client->host);
  PyTuple_SET_ITEM(args, 0, client->host);
  PyTuple_SET_ITEM(args, 1, PyInt_FromLong(client->port));
  PyObject *method = PyString_FromString("connect");

  PyObject *res = PyObject_CallMethodObjArgs(client->sock, method, args, NULL);

  PRINTMARK();

  //PyTuple_SET_ITEM doesn't increment ref counter
  //Py_DECREF(PyTuple_GET_ITEM(args, 1));
  Py_DECREF(args);
  Py_DECREF(method);

  if (res == NULL)
  {
    PRINTMARK();
    return 0;
  }

  Py_DECREF(res);

  PRINTMARK();
  return 1;
}

void *API_createSocket(int family, int type, int proto)
{
  /* Create a normal socket */
  PyObject *sockobj;
  //FIXME: PyModule will leak
  static PyObject *sockmodule = NULL;
  static PyObject *sockclass = NULL;
  static int once = 1;

  if (once)
  {
    /*FIXME: References for module or class are never released */
    sockmodule = PyImport_ImportModule ("socket");

    if (sockmodule == NULL)
    {
      PRINTMARK();
      return NULL;
    }
    sockclass = PyObject_GetAttrString(sockmodule, "socket");

    if (sockclass == NULL)
    {
      PRINTMARK();
      return NULL;
    }

    //FIXME: PyType will leak
    if (!PyType_Check(sockclass))
    {
      PRINTMARK();
      return NULL;
    }

    if (!PyCallable_Check(sockclass))
    {
      PRINTMARK();
      return NULL;
    }

    once = 0;
  }

  PRINTMARK();
  sockobj = PyObject_Call (sockclass, PyTuple_New(0), NULL);
  PRINTMARK();

  if (sockobj == NULL)
  {
    PRINTMARK();
    return NULL;
  }

  PRINTMARK();
  return sockobj;
}



int Client_init(PyClient *self, PyObject *args, PyObject *kwargs)
{
  /* Args:
  def __init__(self, address, protocol = "text", codec = "default"):
  */

  self->client = NULL;
  self->host = NULL;
  self->maxSize = MAX_ITEM_SIZE;


  char *address;
  PRINTMARK();

  static char *kwlist[] = {"address", "max_item_size", NULL};

  if (!PyArg_ParseTupleAndKeywords(args, kwargs, "s|i", kwlist, &address, &self->maxSize))
  {
    PRINTMARK();
    return -1;
  }

  PRINTMARK();
  char *offset = strchr (address, ':');

  if (offset == NULL)
  {
    PyErr_Format(umemcache_MemcachedError, "Invalid argument for address");
    return -1;
  }

  char *port = address + (offset - address) + 1;


  self->host = PyString_FromStringAndSize(address, (offset - address));
  self->port = atoi(port);
  Py_INCREF(self->host);
  PRINTMARK();
  self->sock = (PyObject *) API_createSocket(AF_INET, SOCK_STREAM, 0);
  PRINTMARK();

  self->desc.prv = self;
  self->desc.connect = API_connect;
  self->desc.destroy = API_destroy;
  self->desc.recv = API_recv;
  self->desc.send = API_send;
  PRINTMARK();
  self->client = new Client(&self->desc);
  PRINTMARK();

  return 0;
}

void Client_Destructor(PyClient *self)
{
  PRINTMARK();
  if (self->client)    delete self->client;
  PRINTMARK();
  Py_XDECREF(self->host);
  PRINTMARK();
  PyObject_Del(self);
  PRINTMARK();
}

PyObject *Client_connect(PyClient *self, PyObject *args)
{
  if (self->desc.prv == NULL)
  {
    return PyErr_Format(umemcache_MemcachedError, "Client can not be reconnected");
  }

  if (!self->client->connect (PyString_AS_STRING(self->host), self->port))
  {
    PRINTMARK();
    return NULL;
  }

  Py_RETURN_NONE;
}


PyObject *Client_is_connected(PyClient *self, PyObject *args)
{
  if (self->client->isConnected())
  {
    Py_RETURN_TRUE;
  }

  Py_RETURN_FALSE;
}

PyObject *Client_disconnect(PyClient *self, PyObject *args)
{
  self->client->disconnect(NULL);
  Py_RETURN_NONE;
}

typedef bool (Client::*PFN_COMMAND) (const char *key, size_t cbKey, void *data, size_t cbData, time_t expiration, int flags, bool async, size_t maxSize);

PyObject *Client_command(PyClient *self, PFN_COMMAND cmd, PyObject *args)
{
  char *pResult;
  size_t cbResult;
  char *pKey;
  size_t cbKey;
  char *pData;
  size_t cbData;
  int expire = 0;
  int flags = 0;
  int async = 0;

  if (!PyArg_ParseTuple (args, "s#s#|iib", &pKey, &cbKey, &pData, &cbData, &expire, &flags, &async))
  {
    return NULL;
  }

  bool bAsync = async ? true : false;

  if (!(self->client->*cmd)(pKey, cbKey, pData, cbData, expire, flags, bAsync, self->maxSize))
  {
    if (!PyErr_Occurred())
    {
      return PyErr_Format(umemcache_MemcachedError, "umemcache: %s", self->client->getError());
    }

    return NULL;
  }

  if (!self->client->isPipelined() && !async)
  {
    if (self->client->getResult(&pResult, &cbResult))
    {
      return PyString_FromStringAndSize(pResult, cbResult);
    }
    else
    {
      return PyErr_Format(umemcache_MemcachedError, "Could not retrieve result");
    }
  }

  Py_RETURN_NONE;
}

PyObject *Client_set(PyClient *self, PyObject *args)
{
  return Client_command(self, &Client::set, args);
}

PyObject *Client_add(PyClient *self, PyObject *args)
{
  return Client_command(self, &Client::add, args);
}

PyObject *Client_replace(PyClient *self, PyObject *args)
{
  return Client_command(self, &Client::replace, args);
}

PyObject *Client_append(PyClient *self, PyObject *args)
{
  return Client_command(self, &Client::append, args);
}

PyObject *Client_prepend(PyClient *self, PyObject *args)
{
  return Client_command(self, &Client::prepend, args);
}

PyObject *Client_get(PyClient *self, PyObject *args)
{
  //[ ] def get(self, key):

  char *pKey;
  size_t cbKey;
  char *pData;
  size_t cbData;
  UINT64 cas;
  int flags;

  if (!PyArg_ParseTuple (args, "s#", &pKey, &cbKey))
  {
    return NULL;
  }

  if (self->client->isPipelined())
  {
    return PyErr_Format(umemcache_MemcachedError, "Operation cannot be performed inside a pipeline");
  }

  self->client->getBegin();

  self->client->getKeyWrite(pKey, cbKey);
  self->client->getFlush();

  bool bError = false;

  if (!self->client->getReadNext(&pKey, &cbKey, &pData, &cbData, &flags, &cas, &bError))
  {
    if (bError)
    {
      if (!PyErr_Occurred())
      {
        return PyErr_Format(umemcache_MemcachedError, "umemcache: %s", self->client->getError());
      }
      return NULL;
    }

    Py_RETURN_NONE;
  }

  PyObject *otuple = PyTuple_New(2);
  PyObject *ovalue = PyString_FromStringAndSize(pData, cbData);
  PyObject *oflags = PyInt_FromLong(flags);

  PyTuple_SET_ITEM(otuple, 0, ovalue);
  PyTuple_SET_ITEM(otuple, 1, oflags);

  while (self->client->getReadNext(&pKey, &cbKey, &pData, &cbData, &flags, &cas, &bError));

  if (bError)
  {
    Py_DECREF(otuple);

    if (!PyErr_Occurred())
    {
      return PyErr_Format(umemcache_MemcachedError, "umemcache: %s", self->client->getError());
    }

    return NULL;
  }

  return otuple;
}

PyObject *Client_gets(PyClient *self, PyObject *args)
{
  //[ ] def gets(self, key, default = None):

  char *pKey;
  size_t cbKey;
  char *pData;
  size_t cbData;
  UINT64 cas;
  int flags;

  if (!PyArg_ParseTuple (args, "s#", &pKey, &cbKey))
  {
    return NULL;
  }

  if (self->client->isPipelined())
  {
    return PyErr_Format(umemcache_MemcachedError, "Operation cannot be performed inside a pipeline");
  }

  self->client->getsBegin();

  self->client->getKeyWrite(pKey, cbKey);
  self->client->getFlush();

  bool bError = false;

  if (!self->client->getReadNext(&pKey, &cbKey, &pData, &cbData, &flags, &cas, &bError))
  {
    if (bError)
    {
      if (!PyErr_Occurred())
      {
        return PyErr_Format(umemcache_MemcachedError, "umemcache: %s", self->client->getError());
      }
      return NULL;
    }

    Py_RETURN_NONE;
  }



  PyObject *otuple = PyTuple_New(3);
  PyObject *ovalue = PyString_FromStringAndSize(pData, cbData);
  PyObject *oflags = PyInt_FromLong(flags);
  PyObject *ocas = PyLong_FromUnsignedLongLong(cas);

  PyTuple_SET_ITEM(otuple, 0, ovalue);
  PyTuple_SET_ITEM(otuple, 1, oflags);
  PyTuple_SET_ITEM(otuple, 2, ocas);

  while (self->client->getReadNext(&pKey, &cbKey, &pData, &cbData, &flags, &cas, &bError));

  if (bError)
  {
    Py_DECREF(otuple);

    if (!PyErr_Occurred())
    {
      return PyErr_Format(umemcache_MemcachedError, "umemcache: %s", self->client->getError());
    }

    return NULL;
  }

  return otuple;
}

PyObject *Client_get_multi(PyClient *self, PyObject *okeys)
{
  //[ ] def get_multi(self, keys):

  char *pKey;
  size_t cbKey;
  char *pData;
  size_t cbData;
  UINT64 cas;
  int flags;

  if (self->client->isPipelined())
  {
    return PyErr_Format(umemcache_MemcachedError, "Operation cannot be performed inside a pipeline");
  }

  self->client->getBegin();

  PyObject *iterator = PyObject_GetIter(okeys);

  if (iterator == NULL)
  {
    return NULL;
  }

  PyObject *arg;

  while ( (arg = PyIter_Next(iterator)))
  {
    PyObject *ostr;

    if (PyString_Check(arg))
    {
      ostr = arg;
    }
    else
    {
      ostr = PyObject_Str(arg);
    }

    self->client->getKeyWrite(PyString_AS_STRING(ostr), PyString_GET_SIZE(ostr));
    if (ostr != arg)
    {
      Py_DECREF(ostr);
    }

    Py_DECREF(arg);
  }

  Py_DECREF(iterator);
  self->client->getFlush();

  PyObject *odict = PyDict_New();

  bool bError = false;

  while (self->client->getReadNext(&pKey, &cbKey, &pData, &cbData, &flags, &cas, &bError))
  {
    PyObject *okey  = PyString_FromStringAndSize(pKey, cbKey);
    PyObject *otuple = PyTuple_New(2);
    PyObject *ovalue = PyString_FromStringAndSize(pData, cbData);
    PyObject *oflags = PyInt_FromLong(flags);

    PyTuple_SET_ITEM(otuple, 0, ovalue);
    PyTuple_SET_ITEM(otuple, 1, oflags);
    PyDict_SetItem (odict, okey, otuple);

    Py_DECREF(otuple);
    Py_DECREF(okey);
  }

  if (bError)
  {
    Py_DECREF(odict);

    if (!PyErr_Occurred())
    {
      return PyErr_Format(umemcache_MemcachedError, "umemcache: %s", self->client->getError());
    }

    return NULL;
  }

  return odict;
}

PyObject *Client_gets_multi(PyClient *self, PyObject *okeys)
{
  //[ ] def gets_multi(self, keys):

  char *pKey;
  size_t cbKey;
  char *pData;
  size_t cbData;
  UINT64 cas;
  int flags;

  if (self->client->isPipelined())
  {
    return PyErr_Format(umemcache_MemcachedError, "Operation cannot be performed inside a pipeline");
  }

  self->client->getsBegin();

  PyObject *iterator = PyObject_GetIter(okeys);

  if (iterator == NULL)
  {
    return NULL;
  }

  PyObject *arg;

  while ( (arg = PyIter_Next(iterator)))
  {
    PyObject *ostr;

    if (PyString_Check(arg))
    {
      ostr = arg;
    }
    else
    {
      ostr = PyObject_Str(arg);
    }

    self->client->getKeyWrite(PyString_AS_STRING(ostr), PyString_GET_SIZE(ostr));
    if (ostr != arg)
    {
      Py_DECREF(ostr);
    }

    Py_DECREF(arg);
  }

  Py_DECREF(iterator);
  self->client->getFlush();

  PyObject *odict = PyDict_New();

  bool bError = false;

  while (self->client->getReadNext(&pKey, &cbKey, &pData, &cbData, &flags, &cas, &bError))
  {
    PyObject *okey  = PyString_FromStringAndSize(pKey, cbKey);
    PyObject *otuple = PyTuple_New(3);
    PyObject *ovalue = PyString_FromStringAndSize(pData, cbData);
    PyObject *oflags = PyInt_FromLong(flags);
    PyObject *ocas = PyLong_FromUnsignedLongLong(cas);

    PyTuple_SET_ITEM(otuple, 0, ovalue);
    PyTuple_SET_ITEM(otuple, 1, oflags);
    PyTuple_SET_ITEM(otuple, 2, ocas);
    PyDict_SetItem (odict, okey, otuple);

    Py_DECREF(otuple);
    Py_DECREF(okey);
  }

  if (bError)
  {
    Py_DECREF(odict);

    if (!PyErr_Occurred())
    {
      return PyErr_Format(umemcache_MemcachedError, "umemcache: %s", self->client->getError());
    }

    return NULL;
  }

  return odict;
}

PyObject *Client_begin_pipeline(PyClient *self, PyObject *args)
{
  if (!self->client->pipelineBegin())
  {
    return PyErr_Format(umemcache_MemcachedError, "pipeline already started");
  }

  Py_RETURN_NONE;
}

PyObject *Client_abort_pipeline(PyClient *self, PyObject *args)
{
  self->client->pipelineAbort();
  Py_RETURN_NONE;
}

PyObject *Client_finish_pipeline(PyClient *self, PyObject *args)
{
  char *pResult;
  size_t cbResult;

  if (!self->client->pipelineFlush())
  {
    return PyErr_Format(umemcache_MemcachedError, "error flushing pipeline");
  }

  PyObject *oresults = PyList_New(0);

  while (self->client->getNextPipelineResult(&pResult, &cbResult))
  {
    PyObject *oresult = PyString_FromStringAndSize(pResult, cbResult);
    PyList_Append(oresults, oresult);
    Py_DECREF(oresult);
  }

  return oresults;
}

PyObject *Client_delete(PyClient *self, PyObject *args)
{
  char *pResult;
  size_t cbResult;
  char *pKey;
  size_t cbKey;
  int expire = -1;
  int async = 0;

  if (!PyArg_ParseTuple (args, "s#|ib", &pKey, &cbKey, &expire, &async))
  {
    return NULL;
  }

  time_t tsExpire = expire;

  if (!self->client->del(pKey, cbKey, (tsExpire == -1) ? NULL : (time_t *) &tsExpire, async ? true : false))
  {
    if (!PyErr_Occurred())
    {
      return PyErr_Format(umemcache_MemcachedError, "umemcache: %s", self->client->getError());
    }

    return NULL;
  }

  if (!self->client->isPipelined() && !async)
  {
    if (self->client->getResult(&pResult, &cbResult))
    {
      return PyString_FromStringAndSize(pResult, cbResult);
    }
    else
    {
      return PyErr_Format(umemcache_MemcachedError, "Could not retrieve result");
    }
  }

  Py_RETURN_NONE;
}


PyObject *Client_cas(PyClient *self, PyObject *args)
{
  //[ ] def cas(self, key, data, cas_unique, expiration = 0, flags = 0, async = False):

  char *pResult;
  size_t cbResult;
  char *pKey;
  size_t cbKey;
  char *pData;
  size_t cbData;
  int expire = 0;
  int flags = 0;
  int async = 0;

  UINT64 cas;

  if (!PyArg_ParseTuple (args, "s#s#K|iib", &pKey, &cbKey, &pData, &cbData, &cas, &expire, &flags, &async))
  {
    return NULL;
  }

  if (!self->client->cas(pKey, cbKey, cas, pData, cbData, expire, flags, async ? true : false, self->maxSize))
  {
    if (!PyErr_Occurred())
    {
      return PyErr_Format(umemcache_MemcachedError, "umemcache: %s", self->client->getError());
    }

    return NULL;
  }

  if (!self->client->isPipelined() && !async)
  {
    if (self->client->getResult(&pResult, &cbResult))
    {
      return PyString_FromStringAndSize(pResult, cbResult);
    }
    else
    {
      return PyErr_Format(umemcache_MemcachedError, "Could not retrieve result");
    }
  }

  Py_RETURN_NONE;
}


PyObject *Client_incr(PyClient *self, PyObject *args)
{
  // def incr(self, key, increment, async = False):
  char *pResult;
  size_t cbResult;
  char *pKey;
  size_t cbKey;
  int async = 0;

  UINT64 increment;

  if (!PyArg_ParseTuple (args, "s#K|b", &pKey, &cbKey, &increment, &async))
  {
    return NULL;
  }

  if (!self->client->incr(pKey, cbKey, increment, async ? true : false))
  {
    if (!PyErr_Occurred())
    {
      return PyErr_Format(umemcache_MemcachedError, "umemcache: %s", self->client->getError());
    }

    return NULL;
  }

  if (!self->client->isPipelined() && !async)
  {
    if (self->client->getResult(&pResult, &cbResult))
    {
      pResult[cbResult] = '\0';

      if (strncmp (pResult, "CLIENT_ERROR", 12) == 0)
      {
        return PyErr_Format(umemcache_MemcachedError, pResult);
      }

      return PyString_FromStringAndSize(pResult, cbResult);
    }
    else
    {
      return PyErr_Format(umemcache_MemcachedError, "Could not retrieve result");
    }
  }

  Py_RETURN_NONE;
}

PyObject *Client_decr(PyClient *self, PyObject *args)
{
  // def incr(self, key, increment, async = False):
  char *pResult;
  size_t cbResult;
  char *pKey;
  size_t cbKey;
  int async = 0;

  UINT64 decrement;

  if (!PyArg_ParseTuple (args, "s#K|b", &pKey, &cbKey, &decrement, &async))
  {
    return NULL;
  }

  if (!self->client->decr(pKey, cbKey, decrement, async ? true : false))
  {
    if (!PyErr_Occurred())
    {
      return PyErr_Format(umemcache_MemcachedError, "umemcache: %s", self->client->getError());
    }

    return NULL;
  }

  if (!self->client->isPipelined() && !async)
  {
    if (self->client->getResult(&pResult, &cbResult))
    {
      pResult[cbResult] = '\0';

      if (strncmp (pResult, "CLIENT_ERROR", 12) == 0)
      {
        return PyErr_Format(umemcache_MemcachedError, pResult);
      }

      return PyString_FromStringAndSize(pResult, cbResult);
    }
    else
    {
      return PyErr_Format(umemcache_MemcachedError, "Could not retrieve result");
    }
  }

  Py_RETURN_NONE;
}

PyObject *Client_version(PyClient *self, PyObject *args)
{
  char *pVersion;
  size_t cbVersion;

  if (self->client->isPipelined())
  {
    return PyErr_Format(umemcache_MemcachedError, "Operation cannot be performed inside a pipeline");
  }

  if (!self->client->version(&pVersion, &cbVersion))
  {
    return PyErr_Format(umemcache_MemcachedError, "Could not retrieve version");
  }

  return PyString_FromStringAndSize(pVersion, cbVersion);
}

PyObject *Client_stats(PyClient *self, PyObject *args)
{
  char *pName;
  char *pValue;
  size_t cbName;
  size_t cbValue;

  if (self->client->isPipelined())
  {
    return PyErr_Format(umemcache_MemcachedError, "Operation cannot be performed inside a pipeline");
  }

  if (!self->client->stats(NULL, 0))
  {
    return PyErr_Format(umemcache_MemcachedError, "Stats command failed");
  }

  PyObject *odict = PyDict_New();

  while (self->client->getStats(&pName, &cbName, &pValue, &cbValue))
  {
    PyObject *oname  = PyString_FromStringAndSize(pName, cbName);
    PyObject *ovalue = PyString_FromStringAndSize(pValue, cbValue);

    PyDict_SetItem (odict, oname, ovalue);
  }

  return odict;
}

PyObject *Client_flush_all(PyClient *self, PyObject *args)
{
  char *pResult;
  size_t cbResult;
  int expire = -1;
  int async = 0;

  if (!PyArg_ParseTuple (args, "|ib", &expire, &async))
  {
    return NULL;
  }

  time_t tsExpire = expire;

  if (!self->client->flushAll((tsExpire == -1) ? NULL : (time_t *) &tsExpire, async ? true : false))
  {
    if (!PyErr_Occurred())
    {
      return PyErr_Format(umemcache_MemcachedError, "umemcache: %s", self->client->getError());
    }

    return NULL;
  }

  if (!self->client->isPipelined() && !async)
  {
    if (self->client->getResult(&pResult, &cbResult))
    {
      return PyString_FromStringAndSize(pResult, cbResult);
    }
    else
    {
      return PyErr_Format(umemcache_MemcachedError, "Could not retrieve result");
    }
  }

  Py_RETURN_NONE;
}

PyObject *Client_set_timeout(PyClient *self, PyObject *args)
{

  PyObject *timeout;

  if (!PyArg_ParseTuple (args, "O", &timeout))
  {
    return NULL;
  }

  PyObject *method = PyString_FromString("settimeout");
  PyObject *res = PyObject_CallMethodObjArgs(self->sock, method, timeout, NULL);

  PRINTMARK();

  Py_DECREF(method);

  return res;
}

PyObject *Client_get_timeout(PyClient *self, PyObject *args)
{

  PyObject *method = PyString_FromString("gettimeout");
  PyObject *res = PyObject_CallMethodObjArgs(self->sock, method, NULL);

  PRINTMARK();

  Py_DECREF(method);

  return res;
}




/*
[X] def disconnect(self):
[X] def is_connected(self):
[X] def close(self):
[X] def set(self, key, data, expiration = 0, flags = 0, async = False):
[X] def add(self, key, data, expiration = 0, flags = 0, async = False):
[X] def replace(self, key, data, expiration = 0, flags = 0, async = False):
[X] def append(self, key, data, expiration = 0, flags = 0, async = False):
[X] def prepend(self, key, data, expiration = 0, flags = 0, async = False):
[X] def delete(self, key, expiration = 0, async = False):
[x] def get(self, key, default = None):
[X] def gets(self, key, default = None):
[x] def get_multi(self, keys):
[X] def gets_multi(self, keys):
[X] def cas(self, key, data, cas_unique, expiration = 0, flags = 0, async = False):
[X] def incr(self, key, increment, async = False):
[X] def decr(self, key, increment, async = False):
[X] def getr(self, key, default = None):

[X] def version(self):
[X] def stats(self):

[X] def flush_all(self, expiration = 0, async = False):

*/


static PyMethodDef Client_methods[] = {
  {"connect", (PyCFunction)            Client_connect,            METH_NOARGS, ""},
  {"is_connected", (PyCFunction)            Client_is_connected,            METH_NOARGS, ""},
  {"disconnect", (PyCFunction)            Client_disconnect,            METH_NOARGS, ""},
  {"close", (PyCFunction)            Client_disconnect,            METH_NOARGS, ""},
  {"set", (PyCFunction)            Client_set,            METH_VARARGS, "def set(self, key, data, expiration = 0, flags = 0, async = False)"},
  {"get", (PyCFunction)            Client_get,            METH_VARARGS, "def get(self, key, default = None)"},
  {"gets", (PyCFunction)            Client_gets,            METH_VARARGS, "def gets(self, key, default = None)"},
  {"get_multi", (PyCFunction)            Client_get_multi,            METH_O, "def get_multi(self, keys)"},
  {"gets_multi", (PyCFunction)            Client_gets_multi,            METH_O, "def gets_multi(self, keys)"},
  {"add", (PyCFunction)            Client_add,            METH_VARARGS, "def add(self, key, data, expiration = 0, flags = 0, async = False)"},
  {"replace", (PyCFunction)            Client_replace,            METH_VARARGS, "def replace(self, key, data, expiration = 0, flags = 0, async = False)"},
  {"append", (PyCFunction)            Client_append,            METH_VARARGS, "def append(self, key, data, expiration = 0, flags = 0, async = False)"},
  {"prepend", (PyCFunction)            Client_prepend,            METH_VARARGS, "def prepend(self, key, data, expiration = 0, flags = 0, async = False)"},
  {"delete", (PyCFunction)            Client_delete,            METH_VARARGS, "def delete(self, key, expiration = 0, async = False)"},
  {"cas", (PyCFunction)            Client_cas,            METH_VARARGS, "def cas(self, key, data, cas_unique, expiration = 0, flags = 0, async = False)"},
  {"incr", (PyCFunction)            Client_incr,            METH_VARARGS, "def incr(self, key, increment, async = False)"},
  {"decr", (PyCFunction)            Client_decr,            METH_VARARGS, "def decr(self, key, decrement, async = False)"},
  {"version", (PyCFunction)            Client_version,            METH_NOARGS, "def version(self)"},
  {"stats", (PyCFunction)        Client_stats, METH_NOARGS, "def stats(self)"},
  {"flush_all", (PyCFunction)        Client_flush_all, METH_VARARGS, "def flush_all(self, expiration = 0, async = False)"},
  {"set_timeout", (PyCFunction)       Client_set_timeout, METH_VARARGS, "def set_timeout(self, value)"},
  {"get_timeout", (PyCFunction)       Client_get_timeout, METH_NOARGS, "def get_timeout(self)"},
  {"begin_pipeline", (PyCFunction)       Client_begin_pipeline, METH_NOARGS, "def begin_pipeline(self)"},
  {"abort_pipeline", (PyCFunction)       Client_abort_pipeline, METH_NOARGS, "def abort_pipeline(self)"},
  {"finish_pipeline", (PyCFunction)      Client_finish_pipeline, METH_NOARGS, "def finish_pipeline(self)"},
  {NULL}
};

static PyMemberDef Client_members[] = {
    {"max_item_size", T_INT, offsetof(PyClient, maxSize), READONLY, "Max item size"},
    {"sock", T_OBJECT_EX, offsetof(PyClient, sock), READONLY, "Socket instance"},
    {"host", T_OBJECT_EX, offsetof(PyClient, host), READONLY, "Host"},
    {"port", T_INT, offsetof(PyClient, port), READONLY, "Port"},
    {NULL}  /* Sentinel */
};


static PyTypeObject ClientType = {
  PyObject_HEAD_INIT(NULL)
  0,                /* ob_size        */
  "umemcache.Client",        /* tp_name        */
  sizeof(PyClient),        /* tp_basicsize   */
  0,                /* tp_itemsize    */
  (destructor) Client_Destructor,        /* tp_dealloc     */
  0,                /* tp_print       */
  0,                /* tp_getattr     */
  0,                /* tp_setattr     */
  0,                /* tp_compare     */
  0,                /* tp_repr        */
  0,                /* tp_as_number   */
  0,                /* tp_as_sequence */
  0,                /* tp_as_mapping  */
  0,                /* tp_hash        */
  0,                /* tp_call        */
  0,                /* tp_str         */
  0,                /* tp_getattro    */
  0,                /* tp_setattro    */
  0,                /* tp_as_buffer   */
  Py_TPFLAGS_DEFAULT,        /* tp_flags       */
  "Memcache client.\n\n"
  "Options:\n"
  "- address: memcache server address.\n"
  "- max_item_size: maximum size for an item in memcached.\n"
  "  Defaults to 1 million bytes",	/* tp_doc         */
  0,                /* tp_traverse       */
  0,                /* tp_clear          */
  0,                /* tp_richcompare    */
  0,                /* tp_weaklistoffset    */
  0,                /* tp_iter           */
  0,                /* tp_iternext       */
  Client_methods,                 /* tp_methods        */
  Client_members,            /* tp_members        */
  0,                /* tp_getset         */
  0,                /* tp_base           */
  0,                /* tp_dict           */
  0,                /* tp_descr_get      */
  0,                /* tp_descr_set      */
  0,                /* tp_dictoffset     */
  (initproc)Client_init,  /* tp_init           */
};

static PyMethodDef methods[] = {
  {NULL, NULL, 0, NULL}        /* Sentinel */
};


PyMODINIT_FUNC
  initumemcache(void)
{
  PyObject* m;

  m = Py_InitModule3("umemcache", methods, "");
  if (m == NULL)
    return;

  ClientType.tp_new = PyType_GenericNew;
  if (PyType_Ready(&ClientType) < 0)
    return;
  Py_INCREF(&ClientType);
  PyModule_AddObject(m, "Client", (PyObject *)&ClientType);

  umemcache_MemcachedError = PyErr_NewException("umemcache.MemcachedError",
      PyExc_RuntimeError, NULL); 
  PyModule_AddObject(m, "MemcachedError", (PyObject *)umemcache_MemcachedError);
}
