import ctypes
import os

myfolder = os.path.dirname(os.path.realpath(__file__))

class axisPos(ctypes.Structure):
    _fields_=[("xerror",ctypes.c_int), ("yerror",ctypes.c_int), 
              ("xposition",ctypes.c_float), ("yposition",ctypes.c_float)]
              
class LinearPositionComm:
	pos_receiver_lib = ctypes.CDLL(os.path.join(myfolder,'pos_decode.so'))
	
	def __enter__(self):
		print('Opening DRO port ' + self.port + ' at ' + str(self.baudrate) + ' baud')
		pos_receiver_lib = self.pos_receiver_lib
		pos_receiver_lib.init_comm.argtypes = ( ctypes.POINTER(ctypes.c_char), ctypes.c_int)
		pos_receiver_lib.get_x_pos.restype = ctypes.c_float
		pos_receiver_lib.get_y_pos.restype = ctypes.c_float
		pos_receiver_lib.get_axis_stat.restype = axisPos
		pos_receiver_lib.init_comm(self.port, self.baudrate)
		return self
    
	def __exit__(self, type, value, traceback):
		self.shutdown()

	def __init__(self, port, baudrate):
		self.port = port
		self.baudrate = int(baudrate)

	def shutdown(self):
		self.pos_receiver_lib.shutdown()
	
def getThreadId():
	"""Returns OS thread id - Specific to Linux"""
	libc = ctypes.cdll.LoadLibrary('libc.so.6')
	SYS_gettid = 224 # System dependent, see e.g. /usr/include/x86_64-linux-gnu/asm/unistd_64.h  
	#224 is the value on raspbian
	return libc.syscall(SYS_gettid)
   
print "Thread ID: %i"%getThreadId()
