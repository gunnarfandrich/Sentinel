# Sentinel Software Overview

The following document details the software behind the Sentinel.

## Particle Photon 2 Software

The majority of the Photon Particle 2’s software
involves communicating with/ controlling the many other
modules within the Sentinel.

### Boot Process

When the Photon P2 powers on,
the first step is to run the setup function, which
initializes many different elements of the Sentinel.

#### Communication Initialization

* GPIO Pins
* SPI Pins
* UART Pins
* I2C Pins

The Photon P2 brings the ultimate GPS module online during the communication initialization process. The GPS takes some time to boot, as it must lock onto multiple (typically three) GPS satellite signals to provide accurate location information. Once the required GPS satellite signals are acquired, the real time clock module polls the GPS and updates the device time, if it has fallen out of sync.

### Main Software Loop

Upon finishing initialization, the Photon P2 enters the main program loop.



After setup is complete, the
photon will enter the main loop and check if a week has
passed, meaning the RTC has to be updated with the GPS
time. If the RTC time is not equal to the GPS time, and the
GPS has a signal, the RTC will be updated with the correct
time. Next, if prompted by button press, the photon will
connect to Wi-Fi, or if the button was not pressed, remain
disconnected. Following the previous step, the Photon will
look to see if the button to manually turn on the
Orin/AB02S was pressed, or if the RTC indicates that the
correct amount of time has passed for the Jetson to take
photos; in the event that either of these are true, the photon
will enable GPIO pin D5, connected to the gate of the power
switching NMOS as seen in Figure 9, which will enable
both the Jetson Orin and the AB02S to run as described in
parts B and C. The power to these devices will be on for a
set amount of time, currently 90 seconds, to conserve as
much power as possible, as it would be impossible to keep
these devices on constantly due to power constraints. The
photon also has an “emergency flag” assigned to one of the
GPIO pins; for future iterations of the sentinel, once the
picture is taken and the AI learning model has performed an
analysis, the emergency flag on the Sentinel could be set
high, indicating something was detected by the AI. The
current version of the sentinel features an emergency flag
that is set manually, and sent as output to the AB02S. After
the 90 seconds has elapsed, the Orin and AB02S will shut
off, and the final step of the loop is to “pat the dog,” which
means sending a signal to the watchdog timer telling it not
to trigger a manual reset. After this step, the photon will
restart the loop and repeat the same process until manually
disconnected.



## Jetson Orin Nano Software

On boot, the Orin runs a shell script using Ubuntu’s
“Startup Applications” manager. This shell script, titled
“build.sh”, first compiles the .cpp program made to capture
a single RGB and depth frame from both D415 cameras,
however this only needs to be compiled once, thus it is
commented out for the remainder of the Mini III’s operation
until changes need to be made to the program. After
compiling the frame capture program, the shell script runs
the python script to gather weather data from the BME280
sensor and stores it with the given date to the Weather
folder. The next step is running the output of the frame
capture program, at which point the Mini III actually
captures depth and RGB images from both D415 cameras
and stores them with the appropriate date to the
PlantPictures folder on the Orin. The remaining code in the
shell script simply mounts a USB, if connected on startup,
and copies all of the gathered images and weather data onto
it. This allows the user to quickly gather all data from the
Mini III if physically present.
Apart from the startup code, there was other
software used in experimentation that came directly from
Intel’s RealSense code repository. This includes the Intel
RealSense SDK which was used to gather high quality RGB
and depth data of field and greenhouse plants, the
librealsense convert tool (rs-convert.cpp), used to convert
this gathered data to a usable form, and finally the
librealsense measure tool (rs-measure.cpp) which was used
to verify the accuracy of the cameras’ depth measurements
by measuring plant features captured by both cameras (see
Experimentation section).


## AB02S LoRaWAN Software & ThingSpeak

The purpose of the AB02S is to serve as a
LoRaWAN transmitter to convey select information
regarding the Sentinel to the users. Currently, the Sentinel is
transmitting 3 types of information: latitude, longitude, and
an emergency status. The longitude and latitude are read
from the AIR350Z GPS built directly into the AB02S
module, and the emergency flag is read as from an input pin
that is being transmitted from the P2. Due to the limited
bandwidth of LoRaWAN transmission, the longitude and
latitude are encoded into a byte array, with the integer part
of the number in one byte, and the decimal values in the
following 3 bytes. Since the bytes are being sent as unsigned
integers, the program will also dedicate one byte to indicate
whether the value is positive or negative. Similarly, the
program reads the emergency flag pin, and uses a single
byte to indicate whether the flag is reading 1 or 0. This gives
a total of 11 bytes per message, sent every 15 seconds when
the LoRaWAN device is powered on and connected. To
connect the device to the gateway, it must be registered
online through the Things Network [7], and the code must
be updated with the proper gateway identifiers. The code
was written in C++ following a Heltec template for
LoRaWAN transmission.
Because the information is transmitted as a byte
array, it must be properly decoded at the receiving end. By
using the Things Network website [7], we are able to view
in real-time the data being sent from the Sentinel, but only
in the form of a byte array, which is meaningless without the
proper decoding. This is where the uplink payload formatter
is implemented, which transforms our byte array back into
usable information. Using just a few lines of javascript, we
can use the location of the bytes in the array to determine
whether they are representing the values of the longitude,
latitude, or emergency flag status.
To create a more user-friendly environment for
viewing the LoRaWAN data, we integrated a webhook to
the ThingSpeak website [8]. Creating this webhook allows
the ThingsNetwork to automatically update our channel as it
receives data. By creating a public channel, information
could be made available to anyone with a link to the
channel, making it easy for multiple users to monitor the
Sentinel.
Figure 4: Sentinel ThingSpeak channel
Additionally, implementing the ThingSpeak
channel allows us to create custom e-mail alerts. Using the
‘Matlab Analysis’ tool allows us to send e-mails to the user
once a specific condition in the channel has been met. In
this case, we are triggering an e-mail alert once the
emergency flag variable is equal to 1. In future iterations,
the e-mail alert could be a very powerful tool when
combined with the AI learning onboard the Jetson. In the
body of the e-mail, we include the longitude and latitude by
reading directly from the channel.
Figure 5: E-mail alert using example GPS data
