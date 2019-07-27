import serial
import ConfigParser
import time
import os
import sys
import struct
import poslib
from poslib import LinearPositionComm
import threading

myfolder = os.path.dirname(os.path.realpath(__file__))

configfile = os.path.join(myfolder,"..","dro.ini")
os.chdir(myfolder)
print "Reading config file %s"%configfile
config = ConfigParser.ConfigParser()
config.read(configfile)

port, baudrate , rpm_meter_pin = None, None, None
try:
    port = config.get('GENERAL', 'comport')    
    baudrate = config.get('GENERAL', 'baudrate')    
    rpm_meter_pin = int(config.get('GENERAL', 'rpm_sensor_pin'))    
except Exception, e:
    print "Invalid config file: %s"%(str(e))
    exit(1)

#time.sleep(2)

#time.sleep(2) #not working after the DLL init_commm has been called. Interference with the signal handler?

with LinearPositionComm(config) as comm:
        for i in range(1,10):
            
        for i in range(1,1000):
        #print "X%.3f "%comm.spaxxlib.get_x_pos()
        #print "Y%.3f "%comm.spaxxlib.get_y_pos()
            rpm = comm.spaxxlib.get_rpm()
            j=0
            while(j < 1000):
                j+=1
            sys.stdout.write("RPM %i \r"%(rpm))


