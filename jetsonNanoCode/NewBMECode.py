import time
import board
#import busio
from adafruit_bme280 import basic as adafruit_bme280
#import Adafruit_BME280
import datetime
import sys

# Create sensor object, using the board's default I2C bus.
i2c = board.I2C()  # uses board.SCL and board.SDA

bme280 = adafruit_bme280.Adafruit_BME280_I2C(i2c, address=0x77)

# change this to match the location's pressure (hPa) at sea level
bme280.sea_level_pressure = 1013.25

# Get initial readings
temperature = bme280.temperature
humidity = bme280.relative_humidity
pressure = bme280.pressure

# Close I2C connection before redirecting sys.stdout
#i2c.deinit()

'''
while True:
    sys.stdout = open('/home/nvidia/Documents/sentinel_home/Weather/WeatherInformation/' + datetime.datetime.now().strftime("Date_%Y.%m.%d_Time_%H.%M.%S") + '.txt', 'wt')
    print(datetime.datetime.now().strftime("----Date_%Y.%m.%d_Time_%H.%M.%S----"))
    print("\n          Temperature: %0.1f C" % bme280.temperature)
    print("          Humidity: %0.1f %%" % bme280.relative_humidity)
    print("          Pressure: %0.1f hPa" % bme280.pressure)
    sys.stdout.close()
    break
'''

while True:
    sys.stdout = open('/home/nvidia/Documents/sentinel_home/Weather/WeatherInformation/' + datetime.datetime.now().strftime("Date_%Y.%m.%d_Time_%H.%M.%S") + '.txt', 'wt')
    print(datetime.datetime.now().strftime("----Date_%Y.%m.%d_Time_%H.%M.%S----"))
    print("\n          Temperature: %0.1f C" % temperature)
    print("          Humidity: %0.1f %%" % humidity)
    print("          Pressure: %0.1f hPa" % pressure)
    #sys.stdout.close()
    break
