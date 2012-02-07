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
from umemcache import Client

MEMCACHED_ADDRESS = "127.0.0.1:11211"

class Testumemcache(unittest.TestCase):
    log = logging.getLogger('umemcache')

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
        except (RuntimeError):
            pass
        
        pass
        
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
        except(RuntimeError):
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
    
if __name__ == '__main__':
    unittest.main()

"""
if __name__ == '__main__':
    from guppy import hpy
    hp = hpy()
    hp.setrelheap()
    while True:
        unittest.main()
        heap = hp.heapu()
        print heap
"""        
        
