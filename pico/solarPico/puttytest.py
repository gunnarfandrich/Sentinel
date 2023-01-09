from machine import I2C
from machine import UART, Pin
import ds1307
import utime
import sys
import time
import gpsuart
#import mppti2c

# create RTC object
from machine import I2C, Pin
i2c=I2C(0,sda=Pin(0), scl=Pin(1))
led = Pin(25, Pin.IN)
utime.sleep_ms(100)

# create GPS object
gpsModule = UART(0, baudrate=9600, tx=Pin(16), rx=Pin(17))
gps = gpsuart.GPS(gpsModule)

# create a mppt object
i2c2 = machine.I2C(1, scl=machine.Pin(15), sda=machine.Pin(14))


buff = bytearray(255)

TIMEOUT = False
FIX_STATUS = False

latitude = ""
longitude = ""
satellites = ""
GPStime = ""

# constantly prints out solar voltage, current, and charger state
# via usart to putty through the pico usb with time stamp
sys.stdout.write('Displaying solar power system data:\n')
#i2c2.mppysolar()

#initilize rtc time
ds = ds1307.DS1307(i2c)
f=utime.localtime()
g=f[0],f[1],f[2],f[6],(f[3]),f[4],f[5]
ds.halt(False)
ds.datetime(g)

# while True:
# check RTC time 

print('initial datetime on ds1307 is...', ds.datetime())

variable = ds.datetime()
time.sleep(5)
#print('new datetime on ds1307 is...', variable)
print('The date is', str(variable[1]),'/', str(variable[2]), variable[0])
print('The time is', (variable[4]+5), ':', variable[5], ':',variable[6], 'UTC') #need to be in UTC time so add 5 for eastern time zone

# every "week" pull gps time and compare to time stamp
# simulate week has passed
time.sleep(5)
print('A week is passing...')
time.sleep(3)

# changing the day 
f=utime.localtime()
g=f[0],f[1],f[2]+3,f[6],(f[3]),f[4],f[5]

ds.halt(False)
ds.datetime(g)
print('A week later is...', ds.datetime())

utime.sleep(5)
print('5secs later is...', ds.datetime())

#check if a week has passed and pull gps time
if (f[2] % 1) == 0:
    # in practice this is how I would test for every 7 days (7th, 14th, 21st, 28th)
    # for testing I will manipulate the 7 to trigger on whatever day we're testing
    print('hi')
    gps.getGPS(gpsModule)
    gps.printGPSdata(gpsModule)


# success


# simulate time failure
# changing the day to something random
f=utime.localtime()
g=f[0],f[1],f[2]+14,f[6],(f[3]),f[4],f[5]

ds.halt(False)
ds.datetime(g)

# reset RTC
f=utime.localtime()
g=f[0],f[1],f[2],f[6],(f[3]),f[4],f[5]

ds.halt(False)
ds.datetime(g)

