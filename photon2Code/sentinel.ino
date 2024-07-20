// This #include statement was automatically added by the Particle IDE.
#include "eink_213.h"

#include "ds1307.h"
#include <Wire.h>
#include <iostream>
#include <string>
#include <numeric>
#include <iterator>
#include <cstdint>
#include <Particle-GPS.h>
#include "Serial2/Serial2.h"
using namespace std;

// Wifi-Credentials
char ssid[] = "ATT-WIFI-WRE";                // wifi SSID Shawn's iPhone WhiteSky-West20 ATT-WIFI-WRE
char password[] = "MIST1234";           // wifi password shawn070302 f357m76j MIST1234

// RTC
#define DS1307_ADDRESS 0x68
RTC_DS1307 rtc;

// GPS
Gps gps = Gps(&Serial1);
// Captures gps serial data every second
Timer timer = Timer(1, onSerialData);

// EINK
#define SS_PIN D18
#define SPI_SPEED 200000
#define EINK_BUSY_PIN D19
#define EINK_RST_PIN D14
#define DC_PIN A1

EINK_213 meink(SS_PIN, SPI_SPEED, EINK_BUSY_PIN, DC_PIN, EINK_RST_PIN);
int test = 1;

// BUTTONS:
// BUTTON A => Button to turn on Nano and upload USB Data
#define BUTTONA_PIN D6

// BUTTON B => Button to cycle data acquisition time
#define BUTTONB_PIN D7
int optionCnt = 1;
int checkHour = 2;
bool display = false;

// BUTTON C => Clears EINK after use
#define BUTTONC_PIN D10
bool clear = false;
bool disconnect = true;

// Debouncing variable
unsigned long buttonPress = millis(); 
 
// JETSON/ LORAWAN CONTROLS (Extra variables are to keep track of nano reset, turn the ctr on and off, and to actually cnt the ctr)
// GPIO controls the gate of power switching circuit, allows power to both the Jetson (12V) and the AB02S LoraWan device (5V)
#define JETSON_PWR_CONTROL D5 //was D7
#define JETSON_RUNNING_CHK D4 //was D6
bool resetNano = FALSE;
bool ctrOn = 0;
int ctr = 0;
int tempctr1 = 0;
int tempctr2 = 0;
bool started = false;
bool stopped = false;
bool manualNanoON = false;

// True enables the Jetson to run on startup rather than waiting for the right time
bool Jetson_test = false;

// Watchdog pin
#define WATCHDOG_TRIGGER A2


// Emergency flags
#define EMER_PIN D3
bool EMER_FLAG = 0;

// Necessary to run program when Particle is not connected to wifi
SYSTEM_THREAD(ENABLED);
SYSTEM_MODE(MANUAL);

void setup(){
    // Reset watchdog signal 
    resetwatchdog();
    // Initialize serial data lines and I2C line/clk speed.
    Serial.begin(9600);
    Wire.begin();
    
    // UART connection to Heltec - Not used, new AB02S has onboard GPS
    // Serial2.begin(9600);
    
    Serial.println("Beginning Setup!");
    // Begin gps
    // Initialize UART for GPS module
    gps.begin(9600);
    gps.sendCommand(PMTK_SET_NMEA_OUTPUT_ALLDATA);
    
    rtc.begin();
    
    // Initialize Buttons -- Buttons causing issues, disabled in current version
    //pinMode(BUTTONA_PIN, INPUT_PULLDOWN);
    //attachInterrupt(BUTTONA_PIN, nanoTurnOn, RISING);

   // pinMode(BUTTONB_PIN, INPUT_PULLDOWN);
   // attachInterrupt(BUTTONB_PIN, changePeriod, RISING);

   // pinMode(BUTTONC_PIN, INPUT_PULLDOWN);
   // attachInterrupt(BUTTONC_PIN, changeConnection, RISING);
    
    // Initialize Emergency Flag GPIO
    pinMode(EMER_PIN, OUTPUT);
    digitalWrite(EMER_PIN, LOW);
    
    // Initialize Jetson control pins
    // Controls power to Jetson and LoraWan 
    pinMode(JETSON_PWR_CONTROL, OUTPUT);
    digitalWrite(JETSON_PWR_CONTROL, LOW);

    // Checks if the Jetson is on/uploading data
    pinMode(JETSON_RUNNING_CHK, INPUT_PULLDOWN);

    Particle.variable("Image Period (Hrs)", &checkHour, INT);
    
    timer.start();
    
    delay(5000);
    
    // Pull GPS time on initialization
    // Get Time and Parse
    int gpsUTCTime = rmcGetTime().toInt();
    int gpsDate = rmcGetDate().toInt();
    
    // UTC Time Parse
    int utcHour = gpsUTCTime / 10000;
    int utcMin = (gpsUTCTime / 100) % 100;
    int utcSec = gpsUTCTime % 100;

    // UTC Date Parse
    int gpsDD = gpsDate / 10000;
    int gpsMM = (gpsDate / 100) % 100;
    int gpsYY = gpsDate % 100;

    // Adjust RTC
    Serial.println("GPS UNPARSED:");
    Serial.println(gpsDate);
    
    Rmc rmc = Rmc(gps);
    
    if(rmc.northSouthIndicator[0] != '\0'){ //Adjust if GPS signal received
        rtc.adjust(DateTime(gpsYY, gpsMM, gpsDD, utcHour, utcMin, utcSec));
    }
    
    meink.einkReset();
    meink.begin();
    meink.newTwoHoursMesg();
    meink.einkSleep();
    
    Serial.println("SETUP FINISHED!");
    
    WiFi.clearCredentials();
    WiFi.setCredentials(ssid,password);  // Set Wifi credentials, do not let photon use memory of old wifi connections
    WiFi.connect();
   
}

// Captures the gps serial data every second
void onSerialData()
{
    gps.onSerialData();
}

// This function is called every time Button A is pressed by the user
void nanoTurnOn()
{
    if(millis() - 5 > buttonPress){
        buttonPress = millis();
        manualNanoON = true;
        Serial.println("Nano Turning On");
    }
}

// This function is called every time Button B is pressed by the user
void changePeriod()
{
    if(millis() - 5 > buttonPress){
        buttonPress = millis();
        Serial.println("Changing Period");
        display = true;
    }
}

void jetsonCheck()
{

    //Serial.println("Jetson Check");
    // Manually turn on the Jetson
    digitalWrite(JETSON_PWR_CONTROL, HIGH);
    resetNano = FALSE;

    // Turn counter ON
    ctrOn = TRUE;

    // Wait for 5 second and inform user to wait
    delay(5000);
    
    
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

        Serial.println(digitalRead(JETSON_RUNNING_CHK));
        Serial.println(ctr);

        if (digitalRead(JETSON_RUNNING_CHK) == HIGH)
        {
            // Program has started
            started = true;
            //Serial.println("Started: " + started);
            tempctr1 = ctr;
        }
        // If the program has started and the Jetson has stopped running => program is finsihed
        if (digitalRead(JETSON_RUNNING_CHK) == LOW && started == true)
        {
            stopped = true;
            //Serial.println("Stopped: " + stopped);
            tempctr2 = ctr;
        }

        // If the ctr hits 1.5 minutes before recieving an OK signal then reset the counter and flag that the Jetson needs to be reset
        if (ctr >= 150) // WAS 150
        {
            // Flag that the Jetson needs to be reset
            //Serial.println("RESET NANO: " + resetNano);

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

            //Serial.println("JETSON DONE");

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
            

            Serial.println("Manual Nano: " + manualNanoON);
            // Sleep the Orin
        }
    }
}

void changeConnection()
{
    if(millis() - 5 > buttonPress){
        buttonPress = millis();
        disconnect = !disconnect;
        Serial.println("Changing connection");
    }
}

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
            Serial.println("Disconnect Failed");
        }

        Serial.println("Disconnected");
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

// This function parses data acquired from gps library and returns the Date as an integer (Call after a GPS obj has been instantiated)
// This was written since the GPS library does not parse very well
String rmcGetDate()
{
    // Data 4 contains the GPS RMC. Index one of data[9] contains the Date
    String data = gps.data[4];
    String field = "";
    int index = 0;
    int fieldCount = 0;
    int lastCommaIndex = -1;

    while (index != -1)
    {
        index = data.indexOf(',', lastCommaIndex + 1);

        if (index != -1)
        {
            field = data.substring(lastCommaIndex + 1, index);
        }
        else
        {
            field = data.substring(lastCommaIndex + 1);
        }

        // Extract Date (field 9)
        if (fieldCount == 9)
        {
            return field;
        }
        lastCommaIndex = index;
        fieldCount++;
    }

    return field;
}

// This function parses data acquired from gps library and returns the Time as an integer (Call after a GPS obj has been instantiated)
String rmcGetTime()
{
    // Data 4 contains the GPS RMC. Index one of data[4] contains the time
    String data = gps.data[4];
    String field = "";
    int index = 0;
    int fieldCount = 0;
    int lastCommaIndex = -1;

    while (index != -1)
    {
        index = data.indexOf(',', lastCommaIndex + 1);

        if (index != -1)
        {
            field = data.substring(lastCommaIndex + 1, index);
        }
        else
        {
            field = data.substring(lastCommaIndex + 1);
        }

        // Extract UTC time (field 1)
        if (fieldCount == 1)
        {
            return field;
        }
        lastCommaIndex = index;
        fieldCount++;
    }

    return field;
}

//Send Emergency flag to LoraWan device
void sendFlag(){
    digitalWrite(EMER_PIN, HIGH);
}

// function for patting the watchdog
void resetwatchdog() {
    pinMode(WATCHDOG_TRIGGER, OUTPUT);
    delay(500);
   // Serial.println("Watchdog Reset");
    pinMode(WATCHDOG_TRIGGER, INPUT);
}

void loop(){
        DateTime now = rtc.now();
        Rmc rmc = Rmc(gps);
        
        // Test GPS time
       /* 
        Serial.print("GPS Current date and time: ");
        Serial.print(gpsYY);
        Serial.print("-");
        Serial.print(gpsMM);
        Serial.print("-");
        Serial.print(gpsDD);
        Serial.print(" ");
        Serial.print(utcHour);
        Serial.print(":");
        Serial.print(utcMin);
        Serial.print(":");
        Serial.print(utcSec);
        Serial.println();
    */
    
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

        
        //Serial.println("GPS Time: " + rmcGetTime());
        //Serial.println("GPS Date: " + rmcGetDate());

        // Adjust RTC Time if innaccurate & GPS has signal
        if ((now.hour() != utcHour || now.minute() != utcMin || now.second() != utcSec) && rmc.northSouthIndicator[0] != '\0')
        {
            //Serial.println("RTC Time Inaccurate. Adjusting With GPS Time");
            rtc.adjust(DateTime(gpsYY, gpsMM, gpsDD, utcHour, utcMin, utcSec));
            
        }
        else
        {
            //Serial.println("RTC Time Accurate");
        }
    }
    else
    {
        //Serial.println("Week has not passed");
    }

    // Check if the user wants to connect to the cloud
  checkConnection();
   
     int test = now.hour(); // Replace  with now.hour()

    // If the Jetson is being manually turned on by user => power on the Jetson
    if (manualNanoON == TRUE)
    {
        jetsonCheck();
    }
    // Turn nano on at specified time between the hours of 8am and 6pm (or if testing nano)
    else if (((test >= 8) && (test <= 18) && ((test - 8) % checkHour == 0) && (now.minute() == 0) && (manualNanoON == false)) || Jetson_test == true)
    {
        // Turn on the Jetson
        jetsonCheck();
    }
    // Otherwise check if eink needs to be displayed
    else
    {
        // Check if the EINK needs to be displayed
        if ((optionCnt == 1) && (display == true) && (manualNanoON == false))
        {
            
            Serial.println("Option 1: 2 Hours");
           
            meink.einkReset();
            meink.begin();
            meink.twoHoursMesg();
            meink.einkSleep();
            
            checkHour = 2;
            ++optionCnt;
            delay(5000);
            // Serial.println("Clear EINK");
            
            meink.einkReset();
            meink.begin();
            meink.whiteScreen();
            meink.einkSleep();
            
            display = false;
            
        }
        else if ((optionCnt == 2) && (display == true) && (manualNanoON == false))
        {
            
            Serial.println("Option 2: 3 Hours");
            
            meink.einkReset();
            meink.begin();
            meink.threeHoursMesg();
            meink.einkSleep();
            
            checkHour = 3;
            ++optionCnt;
            delay(5000);
            // Serial.println("Clear EINK");
            
            meink.einkReset();
            meink.begin();
            meink.whiteScreen();
            meink.einkSleep();
            
            display = false;
            
        }
        else if ((optionCnt == 3) && (display == true) && (manualNanoON == false))
        {
            
            Serial.println("Option 3: 4 Hours");
            
            meink.einkReset();
            meink.begin();
            meink.fourHoursMesg();
            meink.einkSleep();
            
            checkHour = 4;
            optionCnt = 1;
            delay(5000);
            // Serial.println("Clear EINK");
            
            meink.einkReset();
            meink.begin();
            meink.whiteScreen();
            meink.einkSleep();
            
            display = false;
            
        }
    }
    
        
// ***
  // *** This will display the strings received from
  // *** the GPS unit. Some may be empty if the GPS
  // *** is not set to send all data.
  // ***
  //Serial.print("Data[0] = "); Serial.println(gps.data[0]);
  //Serial.print("Data[1] = "); Serial.println(gps.data[1]);
  //Serial.print("Data[2] = "); Serial.println(gps.data[2]);
  //Serial.print("Data[3] = "); Serial.println(gps.data[3]);
  //Serial.print("Data[4] = "); Serial.println(gps.data[4]);
  // Serial.print("Data[5] = "); Serial.println(gps.data[5]);
  // Serial.print("Data[6] = "); Serial.println(gps.data[6]);

// ***
  // *** Get the Antenna Status ($PGTOP).
  // ***
  Pgtop pgtop = Pgtop(gps);
  if (pgtop.parse())
  {
    Serial.println("1) Antenna Status ($PGTOP)");
    Serial.println("======================================================");
    Serial.print("Command ID: "); Serial.println(pgtop.commandId);
    Serial.print("Antenna Status: "); Serial.println(pgtop.reference);
    Serial.println("");
  }

  // ***
  // *** Get the Global Positioning System Fixed Data ($GPGGA).
  // ***
 /* Gga gga = Gga(gps);
  if (gga.parse())
  {
    Serial.println("2) Global Positioning System Fixed Data ($GPGGA)");
    Serial.println("======================================================");
    Serial.print("UTC Time: "); Serial.println(gga.utcTime);
    Serial.print("Latitude: "); Serial.println(gga.latitude);
    Serial.print("North/SouthIndicator: "); Serial.println(gga.northSouthIndicator);
    Serial.print("Longitude: "); Serial.println(gga.longitude);
    Serial.print("East/WestIndicator: "); Serial.println(gga.eastWestIndicator);
    Serial.print("Position Fix Indicator: "); Serial.println(gga.positionFixIndicator);
    Serial.print("Satellites Used: "); Serial.println(gga.satellitesUsed);
    Serial.print("Horizontal Dilution of Precision: "); Serial.println(gga.hdop);
    Serial.print("Altitude: "); Serial.print(gga.altitude); Serial.print(" "); Serial.println(gga.altitudeUnit);
    Serial.print("Geoidal Separation: "); Serial.print(gga.geoidalSeparation); Serial.print(" "); Serial.println(gga.geoidalSeparationUnit);
    Serial.print("Age of Diff. Corr.: "); Serial.println(gga.ageOfDiffCorr);
    Serial.println("");
  } */

  // ***
  // *** Get the Recommended Minimum Navigation Information ($GPRMC).
  // ***
  
  if (rmc.parse())
  {
   // if(rmc.northSouthIndicator[0] != '\0')
    {
        Serial.print("UTC Time: "); Serial.println(rmc.utcTime);
        Serial.print("Latitude: "); Serial.println(rmc.latitude);
        Serial.print("North/SouthIndicator: "); Serial.println(rmc.northSouthIndicator);
        Serial.print("Longitude: "); Serial.println(rmc.longitude);
        Serial.print("East/WestIndicator: "); Serial.println(rmc.eastWestIndicator);
        Serial.print("Date: "); Serial.println(rmc.date);
        
        String dataString = String(rmc.date);   
        dataString += ",";
        dataString += String(rmc.utcTime);
        dataString += ",";
        dataString += String(rmc.latitude);
        dataString += ",";
        dataString += String(rmc.northSouthIndicator);
        dataString += ",";
        dataString += String(rmc.longitude);
        dataString += ",";
        dataString += String(rmc.eastWestIndicator);
        
        //Serial.println(dataString);
    }
        
   /* else
    {
       Serial.println("GPS example data"); 
       String dataString = String("070424");   
        dataString += ",";
        dataString += String("213206.000");
        dataString += ",";
        dataString += String("2937.8663");
        dataString += ",";
        dataString += String("N");
        dataString += ",";
        dataString += String("08222.5768");
        dataString += ",";
        dataString += String("W");
        
        Serial.println(dataString);
    }
    */
    }
    
    
    /*
    Serial2.print("UTC Time: "); Serial2.println(rmc.utcTime);
    Serial2.print("Latitude: "); Serial2.println(rmc.latitude);
    Serial2.print("North/SouthIndicator: "); Serial2.println(rmc.northSouthIndicator);
    Serial2.print("Longitude: "); Serial2.println(rmc.longitude);
    Serial2.print("East/WestIndicator: "); Serial2.println(rmc.eastWestIndicator);
    Serial2.print("Date: "); Serial2.println(rmc.date);
    */

 //RTC test
    if (!rtc.begin()) {
        Serial.println("Couldn't find RTC");
        while (1);
    }
    
    if (!rtc.isrunning()) {
        Serial.println("RTC is NOT running!");
    }
    
    if (rtc.isrunning()){
        Serial.print("Current date and time: ");
        Serial.print(now.year());
        Serial.print("-");
        Serial.print(now.month());
        Serial.print("-");
        Serial.print(now.day());
        Serial.print(" ");
        Serial.print(now.hour());
        Serial.print(":");
        Serial.print(now.minute());
        Serial.print(":");
        Serial.print(now.second());
        Serial.println();
    }
    
    // Right now EMER_FLAG is manually set during first loop for testing,
    // Can be changed to GPIO to read input from Jetson 
    /* if (AI_detects_hardlock == 1{
        EMER_FLAG = 1;
    }
    else{
        EMER_FLAG = 0;
    }
    */
    if (EMER_FLAG == 1){
        sendFlag();
    }
    

    // Sleep the device majority of the time. There are deep sleep modes but unsure how to incorporte. Most of these modes
    // Either need 60 second sleeps or a wake up pin which isnt ideal for remote image taking
    // Only sleep if disconnected, if not then you wont be able to connect to the wifi. This can maybe be fixed if you time how long it
    // takes to connect to the internet.
    
    if(Particle.disconnected())
    {
        Serial.println("Sleep");
        System.sleep(20000);
    }
    resetwatchdog();
    
}
