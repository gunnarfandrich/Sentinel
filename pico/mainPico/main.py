#importing relevant modules & classes
from machine import Pin, I2C, WDT, ADC, Timer
from time import sleep
import bme280       #importing BME280 library
import ds1307
import utime
#This timer is just used for lowering the pole
tim1 = Timer()

#initialized the I2C
i2c=I2C(1,sda=Pin(2), scl=Pin(3))

#Sets the timer and the temp sensor to the I2C channel
#ds = ds1307.DS1307(i2c)

#We need to talk about if this things actually needs to read this right now
#bme = bme280.BME280(i2c=i2c)

#Sets pins for motor forward and reverse start both off.
fwd = Pin(16, Pin.OUT)
rvs = Pin(17, Pin.OUT)

fwd.value(0)
rvs.value(0)

#This pin is used to turn on or off the nano
nano = Pin(18, Pin.OUT)
nano.value(0)

#Nano pin so it can tell its still working.
nano_work = Pin(6, Pin.IN, Pin.PULL_DOWN)

#Pins used so nano can tell pico to go up/down
fwd_nano = Pin(7, Pin.IN, Pin.PULL_DOWN)
rvs_nano = Pin(8, Pin.IN, Pin.PULL_DOWN)

#Pin used to check if the pico is stuck
nano_check = Pin(9, Pin.IN, Pin.PULL_DOWN)

#Both these pins are used to talk one other.
#I am using this to tell the other nano I am writing.
writing = Pin(4, Pin.OUT)
#This was going to be used to know when the other was reading but is not being used.
reading = Pin(5, Pin.OUT)

#1000 = 1 second
#Initialize the watch dog timer for 10 second but max is around 8s 
wdt = WDT(timeout=10000)
counter = 0
counter1 = 0
nano_reset = 0
#Test code to test ADC trigger
ADC_reading = 120000
timer_reset = 0

#This is test code in order to get into if statement.
variable = [0,0,0,1,8,0]
    
def counters(pin):
    global counter
    #Makes sure you cannot reset while it waiting for the nano to reset.
    if nano_reset == 0:
        counter = 0

while True:
        #Pulls the date and time from the timer.
        #writing.value(1)
        #variable = ds.datetime()
        #writing.value(1)
        #Reads the ADC from gpio 26. This will give a number between 0-65535 this is not being used
        #ADC_read = ADC(26)
        #This is set to 0 since we are no longer using this.
        ADC_reading = 0
        #sends a signal to the watch dog timer.
        wdt.feed()
        #The first if is always being skipped.
        if ADC_reading > 10000 or timer_reset == 1:
            #Did we get a spike then we are here.
            #This waits 1 second to see if it was just a random spike.
            sleep(1)
            if ADC_reading > 10000 or timer_reset == 1:
                #This is a flag that just says if this has already been set
                timer_reset = 1
                ADC_reading = 0
                counter += 1
                #Reverses for 120 seconds.
                if counter >= 1 and counter < 120:
                    #Just turns on the reverse
                    rvs.value(1)
                #Is triggered once its been 120 seconds resets flag and counter and turns off motor.
                elif counter == 120:
                    counter = 0
                    rvs.value(0)
                    timer_reset = 0
                
        
        #Checks if its 8:00 am
        #Variable[4] is hour, Variable[3] is am/pm with pm = 0 and am = 1, Variable[5] is the minute
        elif ((variable[4] == 8 and variable[3] == 1 and variable[5] == 0 or 1) or (nano_work.value() == 1) or (nano_reset == 1)):
            #Turns on the nano
            nano.value(1)
            #Test code to show it working
            print(counter)
            print(fwd_nano.value())
            #Need some type of timer in order to check if nano has froze.
            fwd.value(fwd_nano.value())
            rvs.value(rvs_nano.value())
            #Just waits 1 second so counter can step every second
            #sleep(1)
            #counter += 1
            #If this goes high resets counter
           # nano_check.irq(trigger=Pin.IRQ_RISING, handler= counters)
            #Waits 30 seconds for nano to give signal
#             if counter >= 120:
#                 if counter > 120:
#                     #This is test code in order to get into if statement.
#                     #variable = [0,0,0,1,8,1]
#                     counter += 1
#                     #Waits 60 seconds in order to make sure nano has turned back on
#                     if counter >= 360:
#                         nano_reset = 0
#                         counter = 0
#                 else:
#                     #This turns off the nano
#                     nano.value(0)
#                     #Sets flag to make sure nano turns back on if it hangs
#                     nano_reset = 1
#                     #Make sure it turns off wait for 6 seconds.
#                     sleep(6)
#                     #This waits a minute after resets to give nano time to tunr back on.
                    
        #check is its 12:00 pm
        elif (variable[4] == 12 and variable[3] == 0 and variable[5] == 0 or 1) or (nano_work.value() == 1) or nano_reset == 1:
            #Turns on the nano
            nano.value(1)
            #Need some type of timer in order to check if nano has froze.
            fwd.value(fwd_nano.value())
            rvs.value(rvs_nano.value())
            #Just waits 1 second so counter can step every second
            sleep(1)
            counter += 1
            #If this goes high resets counter
            nano_check.irq(trigger=Pin.IRQ_RISING, handler= counters)
            #Waits 30 seconds for nano to give signal
            if counter >= 120:
                if counter > 120:
                    #This is test code in order to get into if statement.
                    #variable = [0,0,0,1,8,2]
                    counter += 1
                    #Waits 60 seconds in order to make sure nano has turned back on
                    if counter >= 360:
                        nano_reset = 0
                        counter = 0
                else:
                    #This turns off the nano
                    nano.value(0)
                    #Sets flag to make sure nano turns back on if it hangs
                    nano_reset = 1
                    #Make sure it turns off wait for 6 seconds.
                    sleep(6)
                    #This waits a minute after resets to give nano time to tunr back on.
                    
        #Checks if its 4:00 pm 
        elif (variable[4] == 4 and variable[3] == 0 and variable[5] == 0 or 1) or (nano_work.value() == 1) or nano_reset == 1:
            #Turns on the nano
            nano.value(1)
            #Need some type of timer in order to check if nano has froze.
            fwd.value(fwd_nano.value())
            rvs.value(rvs_nano.value())
            #Just waits 1 second so counter can step every second
            sleep(1)
            counter += 1
            #If this goes high resets counter
            nano_check.irq(trigger=Pin.IRQ_RISING, handler= counters)
            #Waits 30 seconds for nano to give signal
            if counter >= 120:
                if counter > 120:
                    #This is test code in order to get into if statement.
                    #variable = [0,0,0,1,8,2]
                    counter += 1
                    #Waits 60 seconds in order to make sure nano has turned back on
                    if counter >= 360:
                        nano_reset = 0
                        counter = 0
                else:
                    #This turns off the nano
                    nano.value(0)
                    #Sets flag to make sure nano turns back on if it hangs
                    nano_reset = 1
                    #Make sure it turns off wait for 6 seconds.
                    sleep(6)
                    #This waits a minute after resets to give nano time to tunr back on.
        
        #Checks if its 6:00 pm
        elif (variable[4] == 6 and variable[3] == 0 and variable[5] == 0 or 1) or (nano_work.value() == 1) or nano_reset == 1:
            #Turns on the nano
            nano.value(1)
            #Need some type of timer in order to check if nano has froze.
            fwd.value(fwd_nano.value())
            rvs.value(rvs_nano.value())
            #Just waits 1 second so counter can step every second
            sleep(1)
            counter += 1
            #If this goes high resets counter
            nano_check.irq(trigger=Pin.IRQ_RISING, handler= counters)
            #Waits 30 seconds for nano to give signal
            if counter >= 120:
                if counter > 120:
                    #This is test code in order to get into if statement.
                    #variable = [0,0,0,1,8,1]
                    counter += 1
                    #Waits 60 seconds in order to make sure nano has turned back on
                    if counter >= 360:
                        nano_reset = 0
                        counter = 0
                else:
                    #This turns off the nano
                    nano.value(0)
                    #Sets flag to make sure nano turns back on if it hangs
                    nano_reset = 1
                    #Make sure it turns off wait for 6 seconds.
                    sleep(6)
                    #This waits a minute after resets to give nano time to tunr back on.
        
        #If nano is done just shuts it off and processor mostly sleeps 
        elif nano_work.value() == 0:
            #Turns off the nano
            nano.value(0)
            #Always makes sure its 0 when starting the nano to give it the 30 seconds.
            counter = 0
            #This is here just to save energy so its sleeping a lot saving energy
            sleep(6)
        