import serial
import ConfigParser
import time
import os
import sys
import struct
import ctypes
import mmap

myfolder = os.path.dirname(os.path.realpath(__file__))

_pos_decode_lib = ctypes.CDLL(os.path.join(myfolder,'pos_decode.so'))
_pos_decode_lib.init_comm.argtypes = ( ctypes.POINTER(ctypes.c_char), ctypes.c_int)

configfile = os.path.join(myfolder,"dro.ini")
os.chdir(myfolder)
print "Reading config file %s"%configfile
config = ConfigParser.ConfigParser()
config.read(configfile)
port, baudrate = None, None
try:
    port = config.get('GENERAL', 'comport')    
    baudrate = config.get('GENERAL', 'baudrate')    
except Exception, e:
    print "Invalid config file: %s"%(str(e))
    exit(1)

#time.sleep(2)

print('Opening DRO port ' + port + ' at ' + baudrate)
_pos_decode_lib.init_comm(port, int(baudrate))

#time.sleep(2) #not working after the DLL init_commm has been called. Interference with the signal handler?

print "-----------------"
while(True):
	print float(_pos_decode_lib.get_x_pos())
	print float(_pos_decode_lib.get_y_pos())

_pos_decode_lib.shutdown()
