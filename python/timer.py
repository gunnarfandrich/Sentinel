import ds1307
import utime

from machine import I2C, Pin
i2c=I2C(0,sda=Pin(0), scl=Pin(1))
utime.sleep_ms(100)
ds = ds1307.DS1307(i2c)
print('initial datetime on ds1307 is...', ds.datetime())

f=utime.localtime()
g=f[0],f[1],f[2],f[6],(f[3]),f[4],f[5]

ds.halt(False)
ds.datetime(g)
print('new datetime on ds1307 is...', ds.datetime())

utime.sleep(5)
print('5secs later is...', ds.datetime())

variable = ds.datetime()

print('new datetime on ds1307 is...', variable)
print('The year is', variable[0])
print('The month is', variable[1])
print('The day is', variable[2])
#AM is 1 PM is 0
print('The am/pm is', variable[3])
print('The hour is', (variable[4]+5)) #need to be in UTC time so add 5 for eastern time zone
print('The minute is', variable[5])
print('The second is', variable[6])