from machine import UART, Pin


import utime, time

gpsModule = UART(0, baudrate=9600, tx=Pin(16), rx=Pin(17))
print(gpsModule)

buff = bytearray(255)

TIMEOUT = False
FIX_STATUS = False

latitude = ""
longitude = ""
satellites = ""
GPStime = ""

class GPS(object):
    def getGPS(self, gpsModule):
        global FIX_STATUS, TIMEOUT, latitude, longitude, satellites, GPStime
    
        timeout = time.time() + 8 
        while True:
            if (gpsModule.any()) > 0 :
                line=(gpsModule.read())
                buff = str(line)
                #print(buff)
                parts = buff.split(',')
            
                if (parts[0] == "b'$GPRMC"):
                    GPStime = parts[1][0:2] + ":" + parts[1][2:4] + ":" + parts[1][4:6]
                    #print(GPStime)
                    FIX_STATUS = True
                    break
                    
                if (time.time() > timeout):
                    TIMEOUT = True
                    break
                utime.sleep_ms(500)
        
    def convertToDegree(self, RawDegrees):

        RawAsFloat = float(RawDegrees)
        firstdigits = int(RawAsFloat/100) 
        nexttwodigits = RawAsFloat - float(firstdigits*100) 
    
        Converted = float(firstdigits + nexttwodigits/60.0)
        Converted = '{0:.6f}'.format(Converted) 
        return str(Converted)
    
    def printGPSdata(self, gpsModule):
        global FIX_STATUS, TIMEOUT, latitude, longitude, satellites, GPStime
        if(FIX_STATUS == True):
            print("Printing GPS data...")
            print(" ")
            print("Time: "+GPStime)
            print("----------------------")
#                print('hi')
            FIX_STATUS = False
        
        if(TIMEOUT == True):
            print("No GPS data is found.")
            TIMEOUT = False
    
    
