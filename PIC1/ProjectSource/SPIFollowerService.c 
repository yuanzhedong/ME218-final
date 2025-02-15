#include <xc.h>
#include <sys/attribs.h>
#include "ES_Configure.h"
#include "ES_Framework.h"
#include "SPIFollowerService.h"
#include "NavigatorHSM.h"
#include "dbprintf.h"

/*----------------------------- Module Defines ----------------------------*/
#define SPI_SLAVE_CMD 0xAA

/*---------------------------- Module Variables ---------------------------*/
static uint8_t MyPriority;
volatile static uint16_t ReceivedCmd;

/*---------------------------- Module Functions ---------------------------*/
void InitSPI(void);
void __ISR(_SPI_2_VECTOR, IPL6SOFT) SPISlaveISR(void);

/*------------------------------ Module Code ------------------------------*/
bool InitSPIFollowerService(uint8_t Priority)
{
    MyPriority = Priority;
    InitSPI(); // Initialize SPI as slave

    // Post the initial transition event
    ES_Event_t ThisEvent;
    ThisEvent.EventType = ES_INIT;
    if (ES_PostToService(MyPriority, ThisEvent) == true)
    {
        return true;
    }
    else
    {
        return false;
    }
}

bool PostSPIFollowerService(ES_Event_t ThisEvent)
{
    return ES_PostToService(MyPriority, ThisEvent);
}

ES_Event_t RunSPIFollowerService(ES_Event_t ThisEvent)
{
    ES_Event_t ReturnEvent;
    ReturnEvent.EventType = ES_NO_EVENT; // assume no errors

    // Handle events here

    return ReturnEvent;
}

void InitSPI(void)
{
    // Step 0: Disable analog function on all SPI pins
    ANSELBbits.ANSB5 = 0;
    // Step 1: Map SPI Inputs/Outputs to all desired pins
    TRISBbits.TRISB5 = 1;  // Set SDI2 (RB5) as input
    SDI2R = 0b0100;        // Map SDI2 to RB5
    TRISBbits.TRISB8 = 0;  // Set SDO2 (RB8) as output
    RPB8R = 0b0100;        // Map SDO2 to RB8
    TRISBbits.TRISB14 = 1; // Set SCK2 (RB14) as input
    TRISBbits.TRISB4 = 1;  // Set SS2 (RB4) as input

    // Step 2: Disable SPI Module
    SPI2CONbits.ON = 0;

    // Step 3: Clear the receive buffer
    SPI2BUF;

    // Step 4: Enable Enhanced Buffer
    SPI2CONbits.ENHBUF = 0;

    // Step 5: Set Baudrate (not applicable for slave mode)

    // Step 6: Clear the SPIROV Bit
    SPI2STATbits.SPIROV = 0;

    // Step 7: Write desired settings to SPIxCON
    SPI2CONbits.MSTEN = 0; // Slave mode
    SPI2CONbits.CKE = 0;   // Reads on 2nd edge
    SPI2CONbits.CKP = 1;   // SCK idles high
    SPI2CONbits.SSEN = 1;  // Enable SS pin

    // Step 8: Initialize Interrupts
    IFS1CLR = _IFS1_SPI2RXIF_MASK;
    IPC8bits.SPI2IP = 6;
    IEC1SET = _IEC1_SPI2RXIE_MASK;

    // Step 9: Enable SPI
    SPI2CONbits.ON = 1;
}

void __ISR(_SPI_2_VECTOR, IPL6SOFT) SPISlaveISR(void)
{
    ReceivedCmd = (uint16_t)SPI2BUF;
    IFS1CLR = _IFS1_SPI2RXIF_MASK;

    // Post event to Navigator state machine
    ES_Event_t CMD_Event;
    CMD_Event.EventType = ES_GEN;
    CMD_Event.EventParam = ReceivedCmd;
    PostNavigatorHSM(CMD_Event);
}