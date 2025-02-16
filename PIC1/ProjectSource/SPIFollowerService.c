#include <xc.h>
#include <sys/attribs.h>
#include "ES_Configure.h"
#include "ES_Framework.h"
#include "SPIFollowerService.h"
#include "NavigatorHSM.h"
#include "dbprintf.h"

/*---------------------------- Module Variables ---------------------------*/
static uint8_t MyPriority;
volatile static uint8_t ReceivedCmd;
volatile static uint32_t lastReceiveTime;
static uint8_t CurrentStatus = NAV_STATUS_OK;

/*---------------------------- Module Functions ---------------------------*/
void InitSPI(void);
void __ISR(_SPI_2_VECTOR, IPL6SOFT) SPIFollowerISR(void);

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
    uint8_t dummpy = SPI2BUF;

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
    SPI2CONbits.SRXISEL = 0b01; // Interrupt when buffer is full
    IFS1CLR = _IFS1_SPI2RXIF_MASK;
    IPC9bits.SPI2IP = 6;
    IEC1SET = _IEC1_SPI2RXIE_MASK;

    // Step 9: Enable SPI
    SPI2CONbits.ON = 1;

    lastReceiveTime = ES_Timer_GetTime();
}

void __ISR(_SPI_2_VECTOR, IPL6SOFT) SPIFollowerISR(void) {
    uint8_t receivedByte = SPI2BUF;
    DB_printf("Received byte: %d\r\n", receivedByte); // Add debug print

    // Process command directly
    if(receivedByte >= NAV_CMD_MOVE && receivedByte <= NAV_CMD_TURN_360) {
        ES_Event_t CmdEvent;
        CmdEvent.EventType = ES_NEW_PLANNER_CMD;
        CmdEvent.EventParam = receivedByte;
        DB_printf("Received command: %d\r\n", receivedByte);
        PostNavigatorHSM(CmdEvent);
    } else if (receivedByte == NAV_CMD_QUERY_STATUS) {
        // Update status based on Navigator state
        DB_printf("Received status query\r\n");
        NavigatorState_t currentState = QueryNavigatorHSM();
        switch (currentState) {
            case Init:
                CurrentStatus = NAV_STATUS_INIT;
                break;
            case Idle:
                CurrentStatus = NAV_STATUS_IDLE;
                break;
            case LineFollow:
                CurrentStatus = NAV_STATUS_LINE_FOLLOW;
                break;
            case AlignBeacon:
                CurrentStatus = NAV_STATUS_ALIGN_BEACON;
                break;
            case CheckIntersection:
                CurrentStatus = NAV_STATUS_CHECK_INTERSECTION;
                break;
            case TurnLeft:
                CurrentStatus = NAV_STATUS_TURN_LEFT;
                break;
            case TurnRight:
                CurrentStatus = NAV_STATUS_TURN_RIGHT;
                break;
            case LineDiscover:
                CurrentStatus = NAV_STATUS_LINE_DISCOVER;
                break;
            case CheckCrate:
                CurrentStatus = NAV_STATUS_CHECK_CRATE;
                break;
            default:
                CurrentStatus = NAV_STATUS_ERROR;
                break;
        }
    }

    // Always update status for next transfer
    SPI2BUF = CurrentStatus;
}