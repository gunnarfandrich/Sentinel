/*
 * Code prototypes to Drive the 2.13 Waveshare Display Hat on the Particle Argon.
 * Initial code taken from babean at https://github.com/babean/Photon-E-ink.git.
 * Resulting code was taken to work with the 2.13 inch Waveshare Display
 * Modified by: Chloe Bellas
 * Notes: If anyone reads this and wants to work on this code, the first thing that could be improved is
 *        writing the messages to the E-Ink. There are actual functions to write instead of just saving the entire image data.
 *        If more messages want to be added, this was the link used to create the hexmaps: https://www.digole.com/tools/PicturetoC_Hex_converter.php
 */

#ifndef _EINK_213_
#define _EINK_213_
#define EINK_WIDTH 122
#define EINK_HEIGHT 250

#if defined(PARTICLE)
#include "Particle.h"
#endif

// Initializing SPI on Argon as Master
#define MYSPI SPI
#define MYSPI_begin(ss) MYSPI.begin(SPI_MODE_MASTER, ss)


class EINK_213
{
    public:

        // Constructor and prototypes
        EINK_213(uint16_t ss, int spiclkspeed, uint16_t einkbusy, uint16_t einkdc, uint16_t einkrst);
        void begin(void);
        void sendCommand(uint8_t byte);
        void sendData(uint8_t byte);
        void turnOnDisplay(void);
        void blackScreen(void);
        void whiteScreen(void);
        void setWindow(uint16_t XSTART, uint16_t YSTART, uint16_t XEND, uint16_t YEND);
        void setCursor(uint16_t XSTART, uint16_t YSTART);
        void einkReset(void);
        void einkSleep(void);
        void pleaseWaitMsg(void);
        void twoHoursMesg(void);
        void threeHoursMesg(void);
        void fourHoursMesg(void);
        void newTwoHoursMesg(void);

    private: 
    
        uint16_t _ss;
        int _spiclkspeed;
        uint16_t _einkbusy;
        uint16_t _einkdc;
        uint16_t _einkrst;
        void waitEINKbusy(void);
        void waitInitialize(void);
       
};

#endif