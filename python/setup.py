from distutils.core import setup, Extension
import shutil
import sys


    
libs = []

if sys.platform != "win32":
    libs.append("stdc++")
    
if sys.platform == "win32":
    libs.append("ws2_32")

shutil.copy("../Client.h", "./")
shutil.copy("../mcdefs.h", "./")
shutil.copy("../PacketWriter.h", "./")
shutil.copy("../PacketReader.h", "./")
shutil.copy("../socketdefs.h", "./")
shutil.copy("../PacketWriter.cpp", "./")
shutil.copy("../PacketReader.cpp", "./")
shutil.copy("../Client.cpp", "./")

module1 = Extension('umemcached',
                sources = ['umemcached.cpp', 'PacketReader.cpp', 'PacketWriter.cpp', 'Client.cpp'],
                include_dirs = ['./'],
                library_dirs = [],
                libraries=libs,
                define_macros=[('WIN32_LEAN_AND_MEAN', None)])
					
setup (name = 'umemcached',
       version = '1.0',
       description = '',
       ext_modules = [module1])
       
       
