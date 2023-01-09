from machine import I2C

# Create I2C object
i2c = machine.I2C(1, scl=machine.Pin(15), sda=machine.Pin(14))

# Print out any addresses found
devices = i2c.scan()

if devices:
    for d in devices:
        print(hex(d))

import machine
import utime
import ustruct
import sys

###############################################################################
# Constants

# MPPT I2C address
MPPT_ADDR = 0x12

# Registers
REG_DEVID = 0x00
REG_STATUS = 0x02

# Other constants
DEVID = 0xE5

###############################################################################
# Settings

# ialize I2C with pins
i2c = machine.I2C(1,
                  scl=machine.Pin(15),
                  sda=machine.Pin(14),
                  freq=10000)

# write to I2C register within a specific period to enable watchdog function
# failure to write to control register causes the 5V power to be cycled for a specific period of seconds
# writing 0xEA to WDEN enables the watch dog function
# WDCNT contains watchdog timeout in seconds, 0 disables
# WDPWROFF contains the power off period in seconds, default is 10s


###############################################################################
# Functions

def isKthBitSet(n, k):
    if ((n >> (k - 1)) and 1):
        return True
    else:
        return False
        
def reg_write(i2c, addr, reg, data):
    """
    Write bytes to the specified register.
    """
    
    # Construct message
    msg = bytearray()
    msg.append(data)
    
    # Write out message to register
    i2c.writeto_mem(addr, reg, msg)
    
def reg_read(i2c, addr, reg, nbytes=1):
    """
    Read byte(s) from specified register. If nbytes > 1, read from consecutive
    registers.
    """
    
    # Check to make sure caller is asking for 1 or more bytes
    if nbytes < 1:
        return bytearray()
    
    # Request data from specified register(s) over I2C
    data = i2c.readfrom_mem(addr, reg, nbytes)
    
    return data

###############################################################################
# Main

# Read device ID to make sure that we can communicate with the MPPT Solar Charger
# Should be 0x1012

def mpptread():
    data = reg_read(i2c, MPPT_ADDR, 0, 2)
    print(hex(int.from_bytes(data, "big")))
        
    # Wait before taking measurements
    utime.sleep(2.0)

    # Run forever
    #while True:
        
    # Read X, Y, and Z values from registers (16 bits each)
    #write address, read data

    data = reg_read(i2c, MPPT_ADDR, 0x06, 2)
    print("Solar Panel Voltage in V: " + str(int.from_bytes(data, "big")/1000))
    data = reg_read(i2c, MPPT_ADDR, 0x08, 2)
    print("Solar Panel Current in mA: " + str(int.from_bytes(data, "big")/1000))
    data = reg_read(i2c, MPPT_ADDR, 0x10, 2)
    print("Battery Voltage: " + str(int.from_bytes(data, "big")/1000))
    data = reg_read(i2c, MPPT_ADDR, 0x12, 2)
    print("Battery Load Current in mA: " + str(int.from_bytes(data, "big")/1000))
    data = reg_read(i2c, MPPT_ADDR, 0x14, 2)
    print("Estimated Battery charge current in mA: " + str(int.from_bytes(data, "big")/1000))
    
    data = reg_read(i2c, MPPT_ADDR, REG_STATUS, 2)
    STATUS_HEX = (int.from_bytes(data, "big"))
    print(hex(STATUS_HEX))
    if (STATUS_HEX or 0x0040):
        print('ALERT: CHARGING DISABLED')
        # If Alert is asserted in the 6th bit of the status register,
        # 5V power will be disbales due to low battery or night condition
        # Write a text file indicating this to loraWAN
        # Can add more alerts from REG_STATUS
        with open('ALERT.txt', 'w+') as f:
            f.write('ALERT: CHARGING DISABLED /n 5V power is disabled due to low-battery or night condtion')
            print('File was created')
            
    if (STATUS_HEX or 0x0008):
        print('Night Mode detected: solar panel below 3.5 V')
        # If 3rd bit of register is set, night is detected
        # night mode indicates the solar panel is below 3.5 V
        with open('NIGHT.txt', 'w+') as f:
            f.write('Night Mode detected: solar panel below 3.5 V')

    x = STATUS_HEX & 0x7
    # isolating bits 2:0 and checking charger state
    # charger states 0 through 6 detailed in manual
    if (x == 0x0):
        state = ('Night')
    if (x == 0x1):
        state = ('Idle')
    if (x == 0x2):
        state = ('VSRCV')
    if (x == 0x3):
        state = ('Scan')
    if (x == 0x4):
        state = ('Bulk')
    if (x == 0x5):
        state = ('Absorption')
    if (x == 0x6):
        state = ('FLOAT')


    utime.sleep(0.1)
