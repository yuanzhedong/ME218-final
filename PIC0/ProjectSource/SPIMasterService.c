#include <xc.h>
#include <sys/attribs.h>
#include "ES_Configure.h"
#include "ES_Framework.h"
#include "SPIMasterService.h"
#include "dbprintf.h"

/*----------------------------- Module Defines ----------------------------*/
#define SPI_MASTER_CMD 0xAA
#define QUERY_TIMER 1

/*---------------------------- Module Variables ---------------------------*/
static uint8_t MyPriority;
volatile static uint16_t ReceivedCmd;
volatile static uint16_t CurrentNavigatorStatus;
volatile static uint16_t PrevNavigatorStatus;

/*---------------------------- Module Functions ---------------------------*/
void InitSPI(void);
void SendSPICommand(uint16_t command);
void __ISR(_SPI_1_VECTOR, IPL6SOFT) SPIMasterISR(void);

/*------------------------------ Module Code ------------------------------*/
bool InitSPIMasterService(uint8_t Priority)
{
    MyPriority = Priority;
    InitSPI(); // Initialize SPI as master

    // Initialize Commands
    SPI1BUF = SPI_MASTER_CMD;
    PrevNavigatorStatus = 0xFF;
    CurrentNavigatorStatus = 0xFF;

    // Start a timer to query the slave periodically
    ES_Timer_InitTimer(QUERY_TIMER, 200);

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

bool PostSPIMasterService(ES_Event_t ThisEvent)
{
    return ES_PostToService(MyPriority, ThisEvent);
}

ES_Event_t RunSPIMasterService(ES_Event_t ThisEvent)
{
    ES_Event_t ReturnEvent;
    ReturnEvent.EventType = ES_NO_EVENT; // assume no errors

    // Handle events here
    if (ThisEvent.EventType == ES_TIMEOUT && ThisEvent.EventParam == QUERY_TIMER)
    {
        // Query the slave for its status
        SendSPICommand(SPI_MASTER_CMD);
        ES_Timer_InitTimer(QUERY_TIMER, 200); // Restart the timer
    }

    return ReturnEvent;
}

void InitSPI(void)
{
    // Step 0: Disable analog function on all SPI pins
    ANSELBbits.ANSB14 = 0;
    // Step 1: Map SPI Outputs to all desired pins
    TRISBbits.TRISB4 = 0;
    RPB4R = 0b0011; // Map SS1 to RB4
    TRISBbits.TRISB8 = 0;
    RPB8R = 0b0011;        // Map SDO to RB8
    TRISBbits.TRISB14 = 0; // set SCK1 (RB14) as output
    // Step 2: Map SDI
    TRISBbits.TRISB5 = 1; // input
    SDI1R = 0b0001;       // Map SDI1 to RB5
    // Step 3: Disable SPI Module
    SPI1CONbits.ON = 0;
    // Step 4: Clear the receive buffer
    SPI1BUF;
    // Step 5: Enable Enhanced Buffer
    SPI1CONbits.ENHBUF = 0;
    // Step 6: Set Baudrate
    SPI1BRG = 10; // Corresponds to 10 kHz
    // Step 7: Clear the SPIROV Bit
    SPI1STATbits.SPIROV = 0;
    // Step 8: Write desired settings to SPIxCON
    SPI1CONbits.MSTEN = 1;  // Places in Leader Mode
    SPI1CONbits.MSSEN = 1;  // Drives the CS automatically
    SPI1CONbits.CKE = 0;    // Reads on 2nd edge
    SPI1CONbits.CKP = 1;    // SCK idles high
    SPI1CONbits.FRMPOL = 0; // CS is active low
    SPI1CON2bits.AUDEN = 0; // |
    SPI1CONbits.MODE16 = 0; // Enable 8 bit transfers
    SPI1CONbits.MODE32 = 0; // |
    // Step 9: Initialize Interrupts
    IFS1CLR = _IFS1_SPI1RXIF_MASK;
    IPC7bits.SPI1IP = 6;
    IEC1SET = _IEC1_SPI1RXIE_MASK;
    // Step 10: Enable SPI
    SPI1CONbits.ON = 1;
}

void SendSPICommand(uint16_t command)
{
    while (SPI1STATbits.SPITBF)
        ; // Wait until the buffer is not full
    SPI1BUF = command;
}

void __ISR(_SPI_1_VECTOR, IPL6SOFT) SPIMasterISR(void)
{
    CurrentNavigatorStatus = (uint16_t)SPI1BUF;
    IFS1CLR = _IFS1_SPI1RXIF_MASK;

    if ((PrevNavigatorStatus != CurrentNavigatorStatus) && (CurrentNavigatorStatus != 0xFF))
    {
        ES_Event_t CMD_Event;
        CMD_Event.EventType = ES_NAVIGATOR_STATUS;
        CMD_Event.EventParam = CurrentNavigatorStatus;
        PostSPIMasterService(CMD_Event);
        PrevNavigatorStatus = CurrentNavigatorStatus;
    }
}