ultramemcache
=============


ultramemcache is an Ultra fast memcache client written in highly optimized
C++ with Python bindings.

To install it just run Pip as usual::

    $ pip install umemcache

ultramemcache provides a **Client** class you can use to interact with
Memcache::

    >>> import umemcache
    >>> c = umemcache.Client('127.0.0.1:11211')
    >>> c.connect()
    >>> c.set('key', 'value')
    'STORED'
    >>> c.get('key')[0]
    'value'


By design, ultramemcache limits the size of memcached items to 1000*1000
bytes, but you can change this limitation by using the **max_item_size**
argument whencreating a **Client** class.
