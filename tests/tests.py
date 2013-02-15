# -*- coding: utf-8 -*-
"""
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
"""



from __future__ import with_statement

import time
import datetime
import logging
import unittest
import random
import socket
import sys
from umemcache import Client, MemcachedError


MEMCACHED_HOST = "127.0.0.1"
MEMCACHED_PORT = 11211
MEMCACHED_ADDRESS = "%s:%d" % (MEMCACHED_HOST, MEMCACHED_PORT)

class Testumemcache(unittest.TestCase):
    log = logging.getLogger('umemcache')

    def testTupleRefCrashFix(self):
        client = Client(MEMCACHED_ADDRESS)
        for i in range(1000):
            try:
                client.connect()
            except:
                pass

    def testRandomData(self):
        def random_bytes(size):
            return "".join(chr(random.randrange(0, 256)) for i in xrange(size))

        def random_str(size):
            return "".join(chr(random.randrange(33, 64)) for i in xrange(size))


        c = Client(MEMCACHED_ADDRESS);
        c.connect();

        count = 0

        for x in xrange(0, 10):
            key = random_str(random.randrange(1, 33))
            value = random_bytes(random.randrange(1, 5000))
            c.set(key, value, 0, 0, True)
            v2 = c.get(key)[0]

            count += 1

            if len(value) != len(v2):
                print "%d %d != %d" % (count, len(value), len(v2))

            #self.assertEquals(value, v2)

    def testBigDataFail(self):
    	c = Client(MEMCACHED_ADDRESS);
        c.connect();
        data = "31337" * 10000

        for x in xrange(0, 10):
            v = c.get("kaka" + str(x))
            c.set("fsdafbdsakjfjdkfjadklsafdsafdsaffdsafdasfdsafdasfsdafdsafdsafasdas" + str(x), data, 604830, 17, True)


    def testIncrDecrString(self):
    	c = Client(MEMCACHED_ADDRESS);
        c.connect();
        c.set("test", "hej")
    	self.assertRaises(Exception, c.incr, "test", 1)
    	self.assertRaises(Exception, c.decr, "test", 5)


    def testSetExpiration(self):
    	c = Client(MEMCACHED_ADDRESS);
        c.connect();
    	c.set("test", "1", 60, 2)

    def testSegfault(self):
        try:
            c = Client()
            assert False
        except(TypeError):
            pass


    def testConnect(self):
        c = Client(MEMCACHED_ADDRESS);
        c.connect();
        self.assertEquals(True, c.is_connected());
        pass


    def testDisconnect(self):
        c = Client(MEMCACHED_ADDRESS);
        c.connect();
        c.disconnect();
        self.assertEquals(False, c.is_connected());
        pass


    def testClose(self):
        c = Client(MEMCACHED_ADDRESS);
        c.connect();
        c.close();
        self.assertEquals(False, c.is_connected());
        pass

    def testConnectTwice(self):
        c = Client(MEMCACHED_ADDRESS);
        c.connect();
        c.disconnect();
        self.assertEquals(False, c.is_connected());

        try:
            c.connect();
            assert False
        except (MemcachedError):
            pass

        pass

    def testExceptionIsARuntimeError(self):
        c = Client(MEMCACHED_ADDRESS, 1)
        c.connect()
        self.assertRaises(RuntimeError, c.set, 'key1', 'xx')

    def testConnectCloseQuery(self):
        c = Client(MEMCACHED_ADDRESS);
        c.connect();
        c.disconnect();
        try:
            r = c.set("jonas", "kaka", 0, False)
            assert False
        except:
            pass


    def testOversizedKey(self):
        c = Client(MEMCACHED_ADDRESS);
        c.connect();
        data = "A" * (1000 * 1000 * 2)
        try:
            r = c.set("test", data)
            assert False
        except(MemcachedError):
            pass


    def testGet(self):
        c = Client(MEMCACHED_ADDRESS);
        c.connect();
        c.set("key", "value")
        self.assertEquals("value", c.get("key")[0])
        self.assertEquals(None, c.get("key23123"))
        pass

    def testGets(self):
        c = Client(MEMCACHED_ADDRESS);
        c.connect();
        c.set("key", "value")
        self.assertEquals("value", c.gets("key")[0])
        self.assertEquals(None, c.gets("key23123"))
        r = c.gets("key")
        self.assertEquals(3, len(r))
        pass


    def testSet(self):
        c = Client(MEMCACHED_ADDRESS);
        c.connect();
        c.set("key", "my new value")
        self.assertEquals("my new value", c.get("key")[0])
        pass


    def testGet_multi(self):
        c = Client(MEMCACHED_ADDRESS);
        c.connect();
        c.set("key1", "value1")
        c.set("key2", "value2")
        c.set("key3", "value3")
        r = c.get_multi(["key1", "key2", "key3", "key4"])

        self.assertEquals(("value1", 0), r["key1"])
        self.assertEquals(("value2", 0), r["key2"])
        self.assertEquals(("value3", 0), r["key3"])
        self.assertEquals(None, r.get("key4"))

        pass



    def testGets_multi(self):
        c = Client(MEMCACHED_ADDRESS);
        c.connect();
        c.set("key1", "value1")
        c.set("key2", "value2")
        c.set("key3", "value3")
        r = c.gets_multi(["key1", "key2", "key3", "key4"])

        self.assertEquals(3, len(r["key1"]))
        self.assertEquals(3, len(r["key2"]))
        self.assertEquals(3, len(r["key3"]))

        self.assertEquals("value1", r["key1"][0])
        self.assertEquals("value2", r["key2"][0])
        self.assertEquals("value3", r["key3"][0])
        self.assertEquals(None, r.get("key4"))
        pass

    def testAdd(self):
        c = Client(MEMCACHED_ADDRESS);
        c.connect();
        c.set("key1", "value1")
        self.assertEquals("NOT_STORED", c.add("key1", "value"))
        pass

    def testReplace(self):
        c = Client(MEMCACHED_ADDRESS);
        c.connect();
        c.set("key1", "value1")
        self.assertEquals("STORED", c.replace("key1", "value"))
        pass

        pass
    def testAppend(self):
        c = Client(MEMCACHED_ADDRESS);
        c.connect();
        c.set("key1", "a")
        self.assertEquals("STORED", c.append("key1", "b"))
        self.assertEquals("ab", c.get("key1")[0])
        pass

    def testPrepend(self):
        c = Client(MEMCACHED_ADDRESS);
        c.connect();
        c.set("key1", "a")
        self.assertEquals("STORED", c.prepend("key1", "b"))
        self.assertEquals("ba", c.get("key1")[0])
        pass

    def testDel(self):
        c = Client(MEMCACHED_ADDRESS);
        c.connect();
        c.set("key1", "a")
        c.delete("key1")
        self.assertEquals(None, c.get("key1"))
        pass


    def testCas(self):
        c = Client(MEMCACHED_ADDRESS);
        c.connect();
        c.set("key1", "a")
        value, flags, cas = c.gets("key1")
        self.assertEquals("STORED", c.cas("key1", "b", cas))
        c.set("key1", "a")
        self.assertEquals("EXISTS", c.cas("key1", "b", cas))

    def testCasGetsMulti(self):
        c = Client(MEMCACHED_ADDRESS)
        c.connect()
        c.set("key1", "a")
        c.set("key2", "a")
        results = c.gets_multi(("key1", "key2"))
        cas1 = results["key1"][2]
        cas2 = results["key2"][2]
        self.assertEquals("STORED", c.cas("key1", "b", cas1))
        self.assertEquals("STORED", c.cas("key2", "b", cas2))
        results = c.gets_multi(("key1", "key2"))
        cas1 = results["key1"][2]
        cas2 = results["key2"][2]
        c.set("key1", "a")
        self.assertEquals("EXISTS", c.cas("key1", "b", cas1))
        self.assertEquals("STORED", c.cas("key2", "b", cas2))

    def testIncr(self):
        c = Client(MEMCACHED_ADDRESS);
        c.connect();
        c.set("key1", "0")
        c.incr("key1", 313370)
        self.assertEquals("313370", c.get("key1")[0])

    def testDecr(self):
        c = Client(MEMCACHED_ADDRESS);
        c.connect();
        c.set("key1", "31337")
        c.decr("key1", 31337)
        self.assertEquals(0, long(c.get("key1")[0]))


    def testVersion(self):
        c = Client(MEMCACHED_ADDRESS);
        c.connect();
        v = c.version()
        l = v.split('.');

        self.assertEquals(3, len(l))

    def testStats(self):
        c = Client(MEMCACHED_ADDRESS);
        c.connect();
        d = c.stats()

        self.assertTrue (d.has_key("uptime"))
        self.assertTrue (d.has_key("bytes"))

    def testConnectFails(self):
        c = Client("130.244.1.1:31337");
        c.sock.settimeout(2)
        try:
            c.connect();
            assert False
        except:
            pass

        c.disconnect();
        pass

    def testConnectDNSOK(self):
        c = Client("localhost:11211");
        c.connect();
        c.disconnect();
        pass

    def testConnectDNSFails(self):
        c = Client("flensost:12111");
        try:
            c.connect();
            assert False
        except:
            pass

        c.disconnect();
        pass

    def testFlushAll(self):
        c = Client(MEMCACHED_ADDRESS);
        c.connect();
        c.set("key1", "31337")
        self.assertEquals(c.get("key1")[0], "31337")
        c.flush_all()
        self.assertEquals(c.get("key1"), None)

    def testMaxSize(self):
        c = Client(MEMCACHED_ADDRESS, 1)
        c.connect()
        self.assertRaises(MemcachedError, c.set, 'key1', 'xx')
        c.set("key1", "3")
        self.assertEquals(c.get("key1")[0], "3")

    def testSockAccess(self):
        # accessing the members before connect() is called
        c = Client(MEMCACHED_ADDRESS)
        self.assertEquals(c.host, MEMCACHED_HOST)
        self.assertEquals(c.port, MEMCACHED_PORT)
        self.assertTrue(isinstance(c.sock, socket.socket))
        c.sock.settimeout(2)
        c.connect()
        c.set("key1", "31337")
        self.assertEquals(c.get("key1")[0], "31337")

    def testReadOnly(self):
        # make sure once a Client class is created
        # host, port and sock are readonly
        c = Client(MEMCACHED_ADDRESS)
        self.assertEquals(c.host, MEMCACHED_HOST)
        self.assertEquals(c.port, MEMCACHED_PORT)
        self.assertTrue(isinstance(c.sock, socket.socket))

        for attr in ('sock', 'host', 'port'):
            self.assertRaises(TypeError, setattr, c, attr, 'booo')

    def testPipelineIncrDecr(self):
        c1 = Client(MEMCACHED_ADDRESS)
        c2 = Client(MEMCACHED_ADDRESS)
        c1.connect()
        c2.connect()

        # Incr/Decr
        c1.set("counter", "1")
        self.assertEquals("1", c1.get("counter")[0])
        c2.begin_pipeline()
        c2.incr("counter", 10)
        c2.decr("counter", 5)
        self.assertEquals("1", c1.get("counter")[0])
        c2.finish_pipeline()
        self.assertEquals("6", c1.get("counter")[0])    
            
    def testPipelineOverflow(self):
        c1 = Client(MEMCACHED_ADDRESS)
        c2 = Client(MEMCACHED_ADDRESS)
        c1.connect()
        c2.connect()
        c1.begin_pipeline()

        try:
            while True:
                c1.incr("counter", 10)
        except(OverflowError):
            return
        self.assertFalse("Expected exception")
        
            
    def testPipeline(self):
        c1 = Client(MEMCACHED_ADDRESS)
        c2 = Client(MEMCACHED_ADDRESS)
        c1.connect()
        c2.connect()

        # Basic Multi-Set
        c1.set("foo", "bar")
        self.assertEquals("bar", c1.get("foo")[0])
        c2.begin_pipeline()
        c2.set("foo", "bim")
        c2.set("foo2", "baz")
        self.assertEquals("bar", c1.get("foo")[0])
        res = c2.finish_pipeline()
        self.assertEquals(["STORED", "STORED"], res)
        self.assertEquals("bim", c1.get("foo")[0])
        self.assertEquals("baz", c1.get("foo2")[0])

        # Abort
        c2.begin_pipeline()
        c2.set("foo", "baz")
        c2.abort_pipeline()
        self.assertNotEqual("baz", c1.get("foo")[0])



        # Add
        c1.set("added1", "foo")
        c1.set("added2", "bar")
        self.assertEquals("foo", c1.get("added1")[0])
        self.assertEquals("bar", c1.get("added2")[0])
        c2.begin_pipeline()
        c2.add("added1", "bim")
        c2.add("added2", "baz")
        c2.add("added3", "oof")
        c2.add("added4", "rab")
        self.assertEquals("foo", c1.get("added1")[0])
        self.assertEquals("bar", c1.get("added2")[0])
        self.assertEquals(None, c1.get("added3"))
        self.assertEquals(None, c1.get("added4"))
        res = c2.finish_pipeline()
        self.assertEquals(["NOT_STORED", "NOT_STORED", "STORED", "STORED"], res)
        self.assertEquals("foo", c1.get("added1")[0])
        self.assertEquals("bar", c1.get("added2")[0])
        self.assertEquals("oof", c1.get("added3")[0])
        self.assertEquals("rab", c1.get("added4")[0])

        # Replace
        c1.set("replace1", "foo")
        c1.set("replace2", "bar")
        self.assertEquals("foo", c1.get("replace1")[0])
        self.assertEquals("bar", c1.get("replace2")[0])
        c2.begin_pipeline()
        c2.replace("replace1", "bim")
        c2.replace("replace2", "baz")
        c2.replace("replace3", "oof")
        c2.replace("replace4", "rab")
        self.assertEquals("foo", c1.get("replace1")[0])
        self.assertEquals("bar", c1.get("replace2")[0])
        self.assertEquals(None, c1.get("replace3"))
        self.assertEquals(None, c1.get("replace4"))
        res = c2.finish_pipeline()
        self.assertEquals(["STORED", "STORED", "NOT_STORED", "NOT_STORED"], res)
        self.assertEquals("bim", c1.get("replace1")[0])
        self.assertEquals("baz", c1.get("replace2")[0])
        self.assertEquals(None, c1.get("replace3"))
        self.assertEquals(None, c1.get("replace4"))

        # Append / Prepend
        c1.set("append1", "a,")
        self.assertEquals("a,", c1.get("append1")[0])
        c2.begin_pipeline()
        c2.append("append1", "b,")
        c2.append("append1", "c,")
        c2.append("append1", "d,")
        c2.prepend("append1", "x,")
        c2.prepend("append1", "y,")
        c2.prepend("append1", "z,")
        self.assertEquals("a,", c1.get("append1")[0])
        res = c2.finish_pipeline()
        self.assertEquals(["STORED"] * 6, res)
        self.assertEquals("z,y,x,a,b,c,d,", c1.get("append1")[0])

        # Delete
        c1.set("delete1", "foo")
        c1.set("delete2", "bar")
        self.assertEquals("foo", c1.get("delete1")[0])
        self.assertEquals("bar", c1.get("delete2")[0])
        self.assertEquals(None, c1.get("delete3"))
        c2.begin_pipeline()
        c2.delete("delete1")
        c2.delete("delete2")
        c2.delete("delete3")
        self.assertEquals("foo", c1.get("delete1")[0])
        self.assertEquals("bar", c1.get("delete2")[0])
        self.assertEquals(None, c1.get("delete3"))
        res = c2.finish_pipeline()
        self.assertEquals(["DELETED", "DELETED", "NOT_FOUND"], res)
        self.assertEquals(None, c1.get("delete1"))
        self.assertEquals(None, c1.get("delete2"))
        self.assertEquals(None, c1.get("delete3"))

        # CAS
        c1.set("cas1", "foo")
        c1.set("cas2", "bar")
        c1.set("cas3", "bim")
        gots = c1.gets_multi(["cas1", "cas2", "cas3"])
        self.assertEquals("foo", gots["cas1"][0])
        self.assertEquals("bar", gots["cas2"][0])
        self.assertEquals("bim", gots["cas3"][0])
        c1.set("cas1", "baz")
        c2.begin_pipeline()
        c2.cas("cas1", "betty", gots["cas1"][2])
        c2.cas("cas2", "wilma", gots["cas2"][2])
        c2.cas("cas3", "fred", gots["cas3"][2])
        self.assertEquals("baz", c1.get("cas1")[0])
        self.assertEquals("bar", c1.get("cas2")[0])
        self.assertEquals("bim", c1.get("cas3")[0])
        res = c2.finish_pipeline()
        self.assertEquals(["EXISTS", "STORED", "STORED"], res)
        gots = c1.gets_multi(["cas1", "cas2", "cas3"])
        self.assertEquals("baz", c1.get("cas1")[0])
        self.assertEquals("wilma", c1.get("cas2")[0])
        self.assertEquals("fred", c1.get("cas3")[0])

        # Flush All
        c1.set("flush1", "foo")
        c1.set("flush2", "bar")
        c2.begin_pipeline()
        c2.flush_all()
        self.assertEquals("foo", c1.get("flush1")[0])
        self.assertEquals("bar", c1.get("flush2")[0])
        c2.finish_pipeline()
        self.assertEquals(None, c1.get("flush1"))
        self.assertEquals(None, c1.get("flush2"))


    def testPipelineDoesNotAllowReads(self):
        c = Client(MEMCACHED_ADDRESS)
        c.connect()

        c.begin_pipeline()

        self.assertRaises(MemcachedError, c.get, "foo")
        self.assertRaises(MemcachedError, c.gets, "foo")
        self.assertRaises(MemcachedError, c.get_multi, ["foo"])
        self.assertRaises(MemcachedError, c.gets_multi, ["foo"])
        self.assertRaises(MemcachedError, c.version)
        self.assertRaises(MemcachedError, c.stats)


    def testExceptionIsARuntimeError(self):
        c = Client(MEMCACHED_ADDRESS, 1)
        c.connect()
        self.assertRaises(RuntimeError, c.set, 'key1', 'xx')

if __name__ == '__main__':
    leak = len(sys.argv) > 1 and sys.argv[-1] == '--leak'
    if not leak:
        unittest.main()
    else:
        sys.argv = sys.argv[:-1]
        try:
            from guppy import hpy
        except ImportError:
            print('You need to install guppy')
            sys.exit(0)

        hp = hpy()
        hp.setrelheap()

        while True:
            try:
                unittest.main()
            finally:
                heap = hp.heapu()
                print heap
