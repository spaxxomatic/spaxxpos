import ctypes
import os

myfolder = os.path.dirname(os.path.realpath(__file__))

class axisPos(ctypes.Structure):
    _fields_=[("xerror",ctypes.c_int), ("yerror",ctypes.c_int), 
              ("xposition",ctypes.c_float), ("yposition",ctypes.c_float)]
              
class LinearPositionComm:
    spaxxlib = ctypes.CDLL(os.path.join(myfolder,'spaxxlib.so'))
    
    def __enter__(self):
        print('Opening DRO port ' + self.port + ' at ' + str(self.baudrate) + ' baud')
        spaxxlib = self.spaxxlib
        spaxxlib.init_comm.argtypes = ( ctypes.POINTER(ctypes.c_char), ctypes.c_int)
        spaxxlib.get_x_pos.restype = ctypes.c_float
        spaxxlib.get_y_pos.restype = ctypes.c_float
        spaxxlib.get_axis_stat.restype = axisPos
        spaxxlib.init_comm(self.port, self.baudrate)
        spaxxlib.rpmmeterInitialize.argtype =  ctypes.c_int
        spaxxlib.gpioCfgInterfaces(7)
        spaxxlib.rpmmeterInitialize(self.rpm_sensor_pin)
        # uint8_t get_qs_status();
        # int qsServoInitialize(int qs_comm_isr_gpio_pin,int p_step_x_pin, int p_dir_x_pin, int p_step_y_pin, int p_dir_y_pin);
        spaxxlib.qsServoInitialize.argtypes = ( ctypes.c_int, ctypes.c_int, ctypes.c_int, ctypes.c_int, ctypes.c_int)
        # void qsServoShutdown();
        # void reset_servo_comm_interface();
        # void fetch_comm_module_stat();
        # void qs_set_stepdir(uint8_t addr);
        # void qs_move_rel_velocitybased(uint8_t addr, uint32_t velocity, uint32_t distance);
        spaxxlib.qs_move_rel_velocitybased.argtype = (ctypes.c_int, ctypes.c_int, ctypes.c_int)
        # void qs_mode_velocityimmediate(uint8_t addr, uint32_t velocity);
        spaxxlib.qs_mode_velocityimmediate.argtypes = (ctypes.c_int, ctypes.c_int)
        # void qs_enable_motor_driver(uint8_t addr, char motor_enable);
        spaxxlib.qs_enable_motor_driver.argtypes = (ctypes.c_int, ctypes.c_int)
        # void qs_hard_stop(uint8_t addr);
        spaxxlib.qs_hard_stop.argtypes = ctypes.c_int
        # void qs_stop_motion(uint8_t addr);
        spaxxlib.qs_stop_motion.argtypes = ctypes.c_int
        # void qs_restart_proc(uint8_t addr);
        spaxxlib.qs_restart_proc.argtypes = ctypes.c_int
        # void set_trace_level(int trace_level);
        spaxxlib.set_trace_level.argtypes = ctypes.c_int
        # int get_qs_comm_error();
        # int get_qs_comm_error_txt();
        # int rampup_wave(char axis, int duration);
        # int start_wave(char axis, int on);        
        
        return self
    
    def __exit__(self, type, value, traceback):
        self.spaxxlib.shutdown()
        self.spaxxlib.rpmmeterShutdown()

    def __init__(self, port, baudrate, rpm_sensor_pin):
        self.rpm_sensor_pin = rpm_sensor_pin
        self.port = port
        self.baudrate = int(baudrate)

    
def getThreadId():
    """Returns OS thread id - Specific to Linux"""
    libc = ctypes.cdll.LoadLibrary('libc.so.6')
    SYS_gettid = 224 # System dependent, see e.g. /usr/include/x86_64-linux-gnu/asm/unistd_64.h  
    #224 is the value on raspbian
    return libc.syscall(SYS_gettid)

