#include "eink_213.h"
#include "messages.h"

EINK_213::EINK_213(uint16_t ss, int spiclkspeed, uint16_t einkbusy, uint16_t einkdc, uint16_t einkrst) : _ss(ss),
                                                                                    _spiclkspeed(spiclkspeed),
                                                                                    _einkbusy(einkbusy),
                                                                                    _einkdc(einkdc),
                                                                                    _einkrst(einkrst)
{
    pinMode(_einkbusy, INPUT);
    pinMode(_einkdc, OUTPUT);
    pinMode(_einkrst, OUTPUT);
}

// Initialize INK
void EINK_213::begin()
{
    MYSPI_begin(_ss);
    MYSPI.setBitOrder(MSBFIRST);
    MYSPI.setClockSpeed(_spiclkspeed);
    MYSPI.setDataMode(SPI_MODE0);
    Serial.begin(400000); //115200
    waitInitialize();
}

void EINK_213::einkReset()
{
    digitalWrite(_einkrst, 1);
    delay(20);
    digitalWrite(_einkrst, 0);
    delay(2);
    digitalWrite(_einkrst, 1);
    delay(20);
}

void EINK_213::waitInitialize()
{
    // HW Reset
    einkReset(); 
    
    // SW Reset
    waitEINKbusy();
    sendCommand(0x12);
    waitEINKbusy();

    // Drive Output Control
    sendCommand(0x01);
    sendData(0xF9);
    sendData(0x00);
    sendData(0x00);

    // Data Entry Mode
    sendCommand(0x11);
    sendData(0x03);

    setWindow(0, 0, EINK_WIDTH - 1, EINK_HEIGHT - 1);
    setCursor(0 , 0);

    // Border Waveform
    sendCommand(0x3C);
    sendData(0x05);

    // Display Update Control
    sendCommand(0x21);
    sendData(0x00);
    sendData(0x80);

    // Read built-in Temp Sensor
    sendCommand(0x18);
    sendData(0x80);
    waitEINKbusy();

    Serial.println("EINK Initialized");

}
void EINK_213::waitEINKbusy()
{
    Serial.println("EINK Busy");
    while(1)
    {
        if(digitalRead(_einkbusy) == 0)
        {
            break;
        }
        delay(10);
    }
    delay(10);
    Serial.println("EINK Busy Release");
}

void EINK_213::turnOnDisplay(void)
{
    sendCommand(0x22);
    sendData(0xf7);
    sendCommand(0x20);
    waitEINKbusy();
}

void EINK_213::sendCommand(uint8_t byte)
{
    digitalWrite(_einkdc, LOW);
    digitalWrite(_ss, LOW);
    MYSPI.transfer(byte);
    digitalWrite(_ss, HIGH);
}

void EINK_213::sendData(uint8_t byte)
{
    digitalWrite(_einkdc, HIGH);
    digitalWrite(_ss, LOW);
    MYSPI.transfer(byte);
    digitalWrite(_ss, HIGH);
}

void EINK_213::setWindow(uint16_t XSTART, uint16_t YSTART, uint16_t XEND, uint16_t YEND)
{
    sendCommand(0x44);
    sendData((XSTART >> 3) & 0xFF);
    sendData((XEND >> 3) & 0xFF);

    sendCommand(0x45);
    sendData(YSTART & 0xFF);
    sendData((YSTART >> 8) & 0xFF);
    sendData(YEND & 0xFF);
    sendData((YEND >> 8) & 0xFF);
}

void EINK_213::setCursor(uint16_t XSTART, uint16_t YSTART)
{
    sendCommand(0x4E);
    sendData(XSTART & 0xFF);

    sendCommand(0x4F);
    sendData(YSTART & 0xFF);
    sendData((YSTART >> 8) & 0xFF);
}

void EINK_213::blackScreen()
{

    delay(5000);
    uint16_t width;
    uint16_t height;

    width = (EINK_WIDTH % 8 == 0) ? (EINK_WIDTH / 8) : (EINK_WIDTH / 8 + 1);
    height = EINK_HEIGHT;

    sendCommand(0x24);
    for (uint16_t j = 0; j < height; j++)
    {
        for (uint16_t i = 0; i < width; i++)
        {
            sendData(0x00);
        }
    }
    Serial.println("EINK Blackscreen");

    turnOnDisplay();
}

void EINK_213::whiteScreen()
{
    delay(5000);
    uint16_t width;
    uint16_t height;

    width = (EINK_WIDTH % 8 == 0) ? (EINK_WIDTH / 8) : (EINK_WIDTH / 8 + 1);
    height = EINK_HEIGHT;

    sendCommand(0x24);
    for (uint16_t j = 0; j < height; j++)
    {
        for (uint16_t i = 0; i < width; i++)
        {
            sendData(0xFF);
        }
    }

    turnOnDisplay();
}

void EINK_213::pleaseWaitMsg()
{
    delay(5000);

    uint16_t width;
    uint16_t height;

    width = (EINK_WIDTH % 8 == 0) ? (EINK_WIDTH / 8) : (EINK_WIDTH / 8 + 1);
    height = EINK_HEIGHT;

    sendCommand(0x24);
    for (uint16_t j = 0; j < height; j++)
    {
        for (uint16_t i = 0; i < width; i++)
        {
            sendData(pleaseWait[i + j*width]);
        }
    }

    turnOnDisplay();
}

void EINK_213::twoHoursMesg()
{
    delay(5000);
    uint16_t width;
    uint16_t height;

    width = (EINK_WIDTH % 8 == 0) ? (EINK_WIDTH / 8) : (EINK_WIDTH / 8 + 1);
    height = EINK_HEIGHT;

    sendCommand(0x24);
    for (uint16_t j = 0; j < height; j++)
    {
        for (uint16_t i = 0; i < width; i++)
        {
            sendData(twoHours[i + j * width]);
        }
    }

    turnOnDisplay();
}

void EINK_213::threeHoursMesg()
{
    delay(5000);
    uint16_t width;
    uint16_t height;

    width = (EINK_WIDTH % 8 == 0) ? (EINK_WIDTH / 8) : (EINK_WIDTH / 8 + 1);
    height = EINK_HEIGHT;

    sendCommand(0x24);
    for (uint16_t j = 0; j < height; j++)
    {
        for (uint16_t i = 0; i < width; i++)
        {
            sendData(threeHours[i + j * width]);
        }
    }

    turnOnDisplay();
}

void EINK_213::fourHoursMesg()
{
    delay(5000);
    uint16_t width;
    uint16_t height;

    width = (EINK_WIDTH % 8 == 0) ? (EINK_WIDTH / 8) : (EINK_WIDTH / 8 + 1);
    height = EINK_HEIGHT;

    sendCommand(0x24);
    for (uint16_t j = 0; j < height; j++)
    {
        for (uint16_t i = 0; i < width; i++)
        {
            sendData(fourHours[i + j * width]);
        }
    }

    turnOnDisplay();
}

void EINK_213::einkSleep()
{
    sendCommand(0x10);
    sendData(0x01);
    delay(100);
}

void EINK_213::newTwoHoursMesg()
{
    delay(5000);
    uint16_t width;
    uint16_t height;

    width = (EINK_WIDTH % 8 == 0) ? (EINK_WIDTH / 8) : (EINK_WIDTH / 8 + 1);
    height = EINK_HEIGHT;
    
    setCursor(0, 0);
    sendCommand(0x24);
    for (uint16_t j = 0; j < height; j++)
    {
        for (uint16_t i = 0; i < width; i++)
        {
            sendData(newTwoHours[i + j * width]);
        }
    }

    turnOnDisplay();
}
