#include <Arduino.h>
#include <SPI.h>

#include "GPS_Air530.h"
#include "GPS_Air530Z.h"
#include <Wire.h>  
#include "HT_SSD1306Wire.h"
#include "LoRaWan_APP.h"

#define EMERGENCY_PIN GPIO4

// A small helper
void error(const __FlashStringHelper*err) {
  Serial.println(err);
  while (1);
}


/*
 * set LoraWan_RGB to Active,the RGB active in loraWan
 * RGB red means sending;
 * RGB purple means joined done;
 * RGB blue means RxWindow1;
 * RGB yellow means RxWindow2;
 * RGB green means received done;
 */


int test_coord = 1;


uint8_t emergency_flag = 0;
uint8_t select_flag = 0;
uint8_t toggle_flag = 0;
uint32_t starting;

int32_t lng_int_part;
int32_t lng_tmp;
uint32_t lng_frac_part;


int32_t lat_int_part;
int32_t lat_tmp;
uint32_t lat_frac_part;


Air530ZClass GPS;


/* OTAA para*/
uint8_t devEui[] = { 0x70, 0xB3, 0xD5, 0x7E, 0xD0, 0x06, 0x67, 0xA2 };
uint8_t appEui[] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
uint8_t appKey[] = { 0xBF, 0x55, 0x5B, 0x25, 0x0D, 0x26, 0xA3, 0x62, 0x25, 0xEA, 0x41, 0xF4, 0x55, 0x0A, 0X0E, 0x84 };

/* ABP para*/
uint8_t nwkSKey[] = { 0x15, 0xb1, 0xd0, 0xef, 0xa4, 0x63, 0xdf, 0xbe, 0x3d, 0x11, 0x18, 0x1e, 0x1e, 0xc7, 0xda, 0x85 };
uint8_t appSKey[] = { 0xd7, 0x2c, 0x78, 0x75, 0x8c, 0xdc, 0xca, 0xbf, 0x55, 0xee, 0x4a, 0x77, 0x8d, 0x16, 0xef, 0x67 };
uint32_t devAddr =  ( uint32_t )0x007e6ae1;


/*LoraWan channelsmask, default channels 0-7*/
uint16_t userChannelsMask[6]={ 0xFF00,0x0000,0x0000,0x0000,0x0000,0x0000 };


/*LoraWan region, select in arduino IDE tools*/
LoRaMacRegion_t loraWanRegion = ACTIVE_REGION;


/*LoraWan Class, Class A and Class C are supported*/
DeviceClass_t  loraWanClass = LORAWAN_CLASS;


/*the application data transmission duty cycle.  value in [ms].*/
uint32_t appTxDutyCycle = 15000;


/*OTAA or ABP*/
bool overTheAirActivation = LORAWAN_NETMODE;


/*ADR enable*/
bool loraWanAdr = LORAWAN_ADR;


/* set LORAWAN_Net_Reserve ON, the node could save the network info to flash, when node reset not need to join again */
bool keepNet = LORAWAN_NET_RESERVE;


/* Indicates if the node is sending confirmed or unconfirmed messages */
bool isTxConfirmed = LORAWAN_UPLINKMODE;


/* Application port */
uint8_t appPort = 2;
/*!
* Note, that if NbTrials is set to 1 or 2, the MAC will not decrease
* the datarate, in case the LoRaMAC layer did not receive an acknowledgment
*/
uint8_t confirmedNbTrials = 4;


int32_t fracPart(double val, int n) // function used for converting GPS data
{
  return (uint32_t)abs((val - (int32_t)(val))*pow(10,n));
}


void VextON(void) // turns OLED display ON
{
  pinMode(Vext,OUTPUT);
  digitalWrite(Vext, LOW);
}


void VextOFF(void) // turns OLED display off
{
  pinMode(Vext,OUTPUT);
  digitalWrite(Vext, HIGH);
}


void EMERGENCY_PIN_CHECK() {
  delay(200);
  if (digitalRead(EMERGENCY_PIN) == 1) {
    emergency_flag = 0xFF;
}
  else  {
    emergency_flag = 0X00;
  }

}

/*
  if ((digitalRead(TOGGLE_BUTTON) == 1) && test_coord == 1) {
      lng_frac_part = lng_frac_part+10;
      lat_frac_part = lat_frac_part+10;
    }
    */


void ButtonInits(void) {

  pinMode(EMERGENCY_PIN,INPUT_PULLDOWN);
  attachInterrupt(EMERGENCY_PIN,EMERGENCY_PIN_CHECK,RISING);

}

/* Prepares the payload of the frame */
static void prepareTxFrame( uint8_t port )
{
 
  if (test_coord == 0) {
    lng_int_part = (int32_t)GPS.location.lng();
    lng_tmp=lng_int_part;
    lng_int_part=abs(lng_int_part);
    lng_frac_part = fracPart(GPS.location.lng(),6);
   
    lat_int_part = (int32_t)GPS.location.lat();
    lat_tmp=lat_int_part;
    lat_int_part = abs(lat_int_part);
    lat_frac_part = fracPart(GPS.location.lat(),6);
  }
 
    appDataSize = 11;
    appData[0] = (uint8_t)(lng_frac_part & 0xFF);
    appData[1] = (uint8_t)((lng_frac_part>>8) & 0xFF);
    appData[2] = (uint8_t)((lng_frac_part>>16) & 0xFF);
    appData[3] = (uint8_t)(lng_int_part & 0xFF);
    if (lng_tmp < 0) {
      appData[4] = 0xFF;
    }
    else {
      appData[4] = 0x00;
    }
   
    appData[5] = (uint8_t)(lat_frac_part & 0xFF);
    appData[6] = (uint8_t)((lat_frac_part>>8) & 0xFF);
    appData[7] = (uint8_t)((lat_frac_part>>16) & 0xFF);
    appData[8] = (uint8_t)(lat_int_part & 0xFF);
    if (lat_tmp < 0) {
      appData[9] = 0xFF;
    }
    else {
      appData[9] = 0x00;
    }
    appData[10] = emergency_flag;

  int i;
  Serial.print("APP DATA ARRAY: ");
  for(i = 0; i < 11; i++){
    Serial.print(appData[i], HEX);
    Serial.print(",");
  
  }
  Serial.println("");
}

void setup() {


  VextON();


  while (!Serial);  // required for Flora & Micro
  delay(500);

  Serial.begin(115200);

  ButtonInits();


  if (test_coord == 1) {
    lng_int_part = -82;
    lng_tmp=lng_int_part;
    lng_int_part=abs(lng_int_part);
    lng_frac_part=348025;
   
    lat_int_part = 29;
    lat_tmp=lat_int_part;
    lat_int_part = abs(lat_int_part);
    lat_frac_part=643979;
  }


  GPS.begin();
  EMERGENCY_PIN_CHECK();


#if(AT_SUPPORT)
  enableAt();
#endif
  deviceState = DEVICE_STATE_INIT;
  LoRaWAN.ifskipjoin();
}

void loop()
{

  switch( deviceState )
  {
    case DEVICE_STATE_INIT:
    {
#if(LORAWAN_DEVEUI_AUTO)
      LoRaWAN.generateDeveuiByChipID();
#endif
#if(AT_SUPPORT)
      getDevParam();
#endif
      printDevParam();
      LoRaWAN.init(loraWanClass,loraWanRegion);
      deviceState = DEVICE_STATE_JOIN;
      break;
    }
    case DEVICE_STATE_JOIN:
    {
      LoRaWAN.join();
      break;
    }
    case DEVICE_STATE_SEND:
    {
      starting = millis();
      while( (millis()-starting) < 1000 ) {
         while (GPS.available() > 0) {
          GPS.encode(GPS.read());
          }
          }


      prepareTxFrame( appPort );
      LoRaWAN.send();
      deviceState = DEVICE_STATE_CYCLE;
      break;
    }
    case DEVICE_STATE_CYCLE:
    {
      // Schedule next packet transmission
      txDutyCycleTime = appTxDutyCycle + randr( 0, APP_TX_DUTYCYCLE_RND );
      LoRaWAN.cycle(txDutyCycleTime);
      deviceState = DEVICE_STATE_SLEEP;
      break;
    }
    case DEVICE_STATE_SLEEP:
    {
      LoRaWAN.sleep();
      break;
    }
    default:
    {
      deviceState = DEVICE_STATE_INIT;
      break;
    }
  }
}
