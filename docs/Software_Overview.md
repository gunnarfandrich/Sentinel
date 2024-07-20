# Sentinel Software Overview

The following document details the software behind the Sentinel.

## Particle Photon 2 Software

The majority of the Photon Particle 2’s software
involves communicating with and controlling the many other
modules that compose the Sentinel.

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

```c

DateTime now = rtc.now();
Rmc rmc = Rmc(gps);

Particle.process();

// If a week has passed, pull the gps time and update
if (now.day() % 7 == 0)
{
    // Get Time and Parse
    int gpsUTCTime = rmcGetTime().toInt();
    int gpsDate = rmcGetDate().toInt();

    // UTC Time Parse
    int utcHour = gpsUTCTime / 10000;
    int utcMin = (gpsUTCTime / 100) % 100;
    int utcSec = gpsUTCTime % 100;

    // UTC Date Parse
    int gpsMM = gpsDate / 10000;
    int gpsDD = (gpsDate / 100) % 100;
    int gpsYY = gpsDate % 100;

    // Adjust RTC Time if innaccurate & GPS has signal
    if ((now.hour() != utcHour || now.minute() != utcMin || now.second() != utcSec) && rmc.northSouthIndicator[0] != '\0')
    {
        rtc.adjust(DateTime(gpsYY, gpsMM, gpsDD, utcHour, utcMin, utcSec));
    }
}
```

As seen in the code snippet above, in the main program loop, the
Photon P2 will if a week has passed. If a week has passed,
the RTC must be updated with the GPS time. If the RTC time
is not equal to the GPS time, and the GPS has a signal,
the RTC will be updated with the correct time.

Following GPS and RTC initialization, the Photon P2 will check if the user wants to connect to the cloud or remain offline.

```c
void checkConnection()
{
    // If the user requests a disconnect and the Particle is connected to the Cloud
    if((disconnect == true) && (Particle.connected() == true))
    {
        //Disconnect from cloud if you are viewing the screen
        Particle.disconnect();
        waitForNot(Particle.connected, 15000);
        Particle.process();
        if (Particle.connected())
        {
            Log.info("Failed to disconnect from Particle");
        }
    }
    // If the user requests a connect and the Particle is disconnected from the Cloud
    else if((disconnect == false) && (Particle.connected() == false))
    {
        // Connect to the cloud if possible, wait 15 seconds to allow for variable update
        Particle.connect();
        waitFor(Particle.connected, 15000);
        Particle.process();
        Serial.println("Connected");
    }
}
```

Next, the Photon P2 will check if the user has manually turned on the Jetson Orin and or the AB02S LoRaWAN communication module. Alternatively, the Photon P2 will check if the RTC indicates that the
correct amount of time has passed for the Jetson to take photos.

```c
    // If the Jetson is being manually turned on by user => power on the Jetson
    if (manualNanoON == TRUE)
    {
        jetsonCheck();
    }
```

```c
void jetsonCheck()
{
    // Manually turn on the Jetson
    digitalWrite(JETSON_PWR_CONTROL, HIGH);
    resetNano = FALSE;

    // Turn counter ON
    ctrOn = TRUE;

    // Wait for 5 second and inform user to wait
    delay(5000);

    // reset e-ink display
    meink.einkReset();
    meink.begin();
    meink.pleaseWaitMsg();
    meink.einkSleep();
    

    // While the ctr is on, check if the signal is recieved from the Jetson for at least 90 seconds => If it is, this loop will break
    while (ctrOn)
    {
        // Increment the ctr
        ++ctr;

        delay(1000);

        if (digitalRead(JETSON_RUNNING_CHK) == HIGH)
        {
            // Program has started
            started = true;
            tempctr1 = ctr;
        }
        // If the program has started and the Jetson has stopped running => program is finsihed
        if (digitalRead(JETSON_RUNNING_CHK) == LOW && started == true)
        {
            stopped = true;
            tempctr2 = ctr;
        }

        // If the ctr hits 1.5 minutes before recieving an OK signal then reset the counter and flag that the Jetson needs to be reset
        if (ctr >= 150) // WAS 150
        {
            // Reset the counter
            ctr = 0;

            // Turn off the power to the Jetson
            digitalWrite(JETSON_PWR_CONTROL, LOW);

            // Wait 5 seconds
            delay(5000);

            // Turn on the power again
            digitalWrite(JETSON_PWR_CONTROL, HIGH);

            // Reset program states
            started = false;
            stopped = false;
        }

        // jetson is done the ctr should just run since it wont be getting the reset ctr signal anymore and then will fall into this loop
        if (ctr >= 90) //WAS 90
        {
            // Turn off power to Jetson
            digitalWrite(JETSON_PWR_CONTROL, LOW);

            // Reset counter variables
            ctr = 0;
            ctrOn = FALSE;
            manualNanoON = FALSE;

            // Reset states
            started = false;
            stopped = false;
            tempctr1 = 0;
            tempctr2 = 0;

            delay(5000);
            
           
            meink.einkReset();
            meink.begin();
            meink.whiteScreen();
            meink.einkSleep();
        }
    }
}
```

Seen in the code snippet above, in the event that any of the preceeding conditions
are true, the photon will enable GPIO pin D5, connected to the gate of the power
switching NMOS, which will enable both the Jetson Orin and the AB02S. The power
to these devices will be on for a set amount of time, currently 90 seconds, to
conserve as much power as possible, as it would be impossible to keep
these devices on constantly due to power constraints.

If none of the preceeding conditions are met, the Photon P2 will check if the
E-ink display is to be updated.

```c
// Check if the EINK needs to be displayed
        if ((optionCnt == 1) && (display == true) && (manualNanoON == false))
        {
            //Serial.println("Option 1: 2 Hours");
           
            meink.einkReset();
            meink.begin();
            meink.twoHoursMesg();
            meink.einkSleep();
            
            checkHour = 2;
            ++optionCnt;
            delay(5000);
            
            meink.einkReset();
            meink.begin();
            meink.whiteScreen();
            meink.einkSleep();
            
            display = false;
        }
        else if ((optionCnt == 2) && (display == true) && (manualNanoON == false))
        {
            //Serial.println("Option 2: 3 Hours");
            
            meink.einkReset();
            meink.begin();
            meink.threeHoursMesg();
            meink.einkSleep();
            
            checkHour = 3;
            ++optionCnt;
            delay(5000);
            
            meink.einkReset();
            meink.begin();
            meink.whiteScreen();
            meink.einkSleep();
            
            display = false;
        }
        else if ((optionCnt == 3) && (display == true) && (manualNanoON == false))
        {
            //Serial.println("Option 3: 4 Hours");
            
            meink.einkReset();
            meink.begin();
            meink.fourHoursMesg();
            meink.einkSleep();
            
            checkHour = 4;
            optionCnt = 1;
            delay(5000);
            
            meink.einkReset();
            meink.begin();
            meink.whiteScreen();
            meink.einkSleep();
            
            display = false;
        }
```

The code snippet above details the process of updating the E-ink display,
depending on the display option that is selected.

The Photon P2 also possesses code for an “emergency flag” assigned to one
of the GPIO pins; for future iterations of the sentinel, once the
picture is taken and the AI learning model has performed an
analysis, the emergency flag on the Sentinel could be set
high, indicating something was detected by the AI.

```c
#define EMER_PIN D3
bool EMER_FLAG = 0;
```

```c
if (EMER_FLAG == 1)
{
    sendFlag();
}
```

The current version of the sentinel sets the emergency flag manually,
and is send as an output to the AB02S. After 90 seconds has elapsed,
the Orin and AB02S will shut off, and the final step of the loop is
to “pat the dog,” which means sending a signal to the watchdog timer
telling it not to trigger a manual reset. After this step, the photon
will restart the loop and repeat the same process until manually
disconnected.



## Jetson Orin Nano Software

On boot, the Jetson Orin runs a shell script, "build.sh" using Ubuntu’s
“Startup Applications” manager.

### Build.sh Script

```sh
#compile code
#g++ SingleFrameCaptureRealSense.cpp -o output -lGLU -lglut -lopencv_core -lrealsense2 -lopencv_imgproc -lopencv_imgcodecs
# gather weather data
echo "Capturing Weather Data"
python /home/nvidia/Documents/sentinel_home/NewBMECode.py

# gather camera data
./output

# mount usb if exists
echo "Mounting USB device if it exists"
sudo mount /dev/sda1 /USB_Transfer

# copy files to usb
echo "Copying files to USB device if present and mounted"
cp -a /home/nvidia/Documents/sentinel_home/Weather/WeatherInformation /USB_Transfer
cp -a /home/nvidia/Documents/sentinel_home/PlantPictures/TakenImages /USB_Transfer
```

Seen in the code snippet above, "build.sh" first compiles the .cpp program made
to capture a single RGB and depth frame from both D415 cameras,
however this only needs to be compiled once, thus it is
commented out for the remainder of the Sentinel's field operation
until changes need to be made to the program.

More information about the frame capture program can be found [here]
(./docs/ImageCaptureProgram.md)


After compiling the frame capture program, the shell script runs
the python script to gather weather data from the BME280
sensor and stores it with the given date to the Weather
folder.

The next step is running the output of the frame
capture program, at which point the Sentinel
captures depth and RGB images from both D415 cameras
and stores them with the appropriate date to the
**PlantPictures** folder on the Orin. The remaining code in the
shell script simply mounts a USB, if connected on startup,
and copies all of the gathered images and weather data onto
it. This allows the user to quickly gather all data from the
Sentinel.

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
