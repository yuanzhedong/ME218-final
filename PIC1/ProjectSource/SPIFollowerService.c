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
static uint8_t CurrentNavStatus = NAV_STATUS_IDLE;

/*---------------------------- Module Functions ---------------------------*/
void InitSPI(void);
void __ISR(_SPI_2_VECTOR, IPL6SOFT) SPIFollowerISR(void);
void TriggerSPIFollowerISR(void); // Function prototype for triggering ISR

/*------------------------------ Module Code ------------------------------*/
bool InitSPIFollowerService(uint8_t Priority)
{
    MyPriority = Priority;
    InitSPI(); // Initialize SPI as slave
    ES_Timer_InitTimer(NAV_CMD_UPDATE_TIMER, 1000);
    
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

    if (ThisEvent.EventType == ES_NEW_NAV_STATUS) {
        CurrentNavStatus = ThisEvent.EventParam;
        DB_printf("[SPI] SPIFollowerService received new nav status: %d\r\n", CurrentNavStatus);
    }

    // Start a timer to query the slave periodically
    
    //puts("[SPI] Started SPI Query Timer\n");
    // ES_Event_t CmdEvent;
    // CmdEvent.EventType = ES_NEW_NAV_CMD;
    // CmdEvent.EventParam = 0x02;
    // PostNavigatorHSM(CmdEvent);
    return ReturnEvent;
}

void InitSPI(void)
{
    // Step 0: Disable analog function on all SPI pins
    
    // Step 1: Map SPI Inputs/Outputs to all desired pins
    //TRISBbits.TRISB13 = 1;  // Set SDI2 (RB13) as input
    //ANSELBbits.ANSB13 = 0;  // Disable analog function on SDI2
    //SDI2R = 0b0011;        // Map SDI2 to RB13
    TRISAbits.TRISA2 = 1;  // Set SDI2 (RA2) as input
    SDI2R = 0b0000;        // Map SDI2 to RA2
    TRISBbits.TRISB8 = 0;  // Set SDO2 (RB8) as output
    RPB8R = 0b0100;        // Map SDO2 to RB8
    TRISBbits.TRISB15 = 1; // Set SCK2 (RB15) as input
    TRISBbits.TRISB4 = 1;  // Set SS2 (RB4) as input
    ANSELBbits.ANSB15 = 0; // Disable analog function on SCK2
    //ANSELBbits.ANSB4 = 0;  // Disable analog function on SS2

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
    //SPI2CONbits.SSEN = 1;  // Enable SS pin

    // Step 8: Initialize Interrupts
    SPI2CONbits.SRXISEL = 0b01; // Interrupt when buffer is full
    IFS1CLR = _IFS1_SPI2RXIF_MASK;
    IPC9bits.SPI2IP = 6;
    IEC1SET = _IEC1_SPI2RXIE_MASK;

    // Step 9: Enable SPI
    SPI2CONbits.ON = 1;


    SPI1CONbits.MODE16 = 0; // Enable 8 bit transfers
    SPI1CONbits.MODE32 = 0; 
    lastReceiveTime = ES_Timer_GetTime();
}

void __ISR(_SPI_2_VECTOR, IPL6SOFT) SPIFollowerISR(void) {
    uint8_t receivedByte = SPI2BUF;
    IFS1CLR = _IFS1_SPI2RXIF_MASK; // Clear the interrupt flag

    DB_printf("[SPI] Received byte: %d\r\n", receivedByte); // Add debug print

    // Process command directly
    if(receivedByte >= NAV_CMD_MOVE_FORWARD && receivedByte <= NAV_CMD_TURN_360) {
        ReceivedCmd = receivedByte;
        DB_printf("Received command: %d\r\n", ReceivedCmd);
        ES_Event_t CmdEvent;
        CmdEvent.EventType = ES_NEW_NAV_CMD;
        CmdEvent.EventParam = ReceivedCmd;
        PostNavigatorHSM(CmdEvent);
    } else if (receivedByte == NAV_CMD_QUERY_STATUS) {
        // Update status based on Navigator state
        DB_printf("Received status query\r\n");
        NavigatorState_t currentState = QueryNavigatorHSM();
        switch (currentState) {
            case Init:
                CurrentNavStatus = NAV_STATUS_INIT;
                break;
            case Idle:
                CurrentNavStatus = NAV_STATUS_IDLE;
                break;
            case LineFollow:
                CurrentNavStatus = NAV_STATUS_LINE_FOLLOW;
                break;
            case AlignBeacon:
                CurrentNavStatus = NAV_STATUS_ALIGN_BEACON;
                break;
            case CheckIntersection:
                CurrentNavStatus = NAV_STATUS_CHECK_INTERSECTION;
                break;
            case TurnLeft:
                CurrentNavStatus = NAV_STATUS_TURN_LEFT;
                break;
            case TurnRight:
                CurrentNavStatus = NAV_STATUS_TURN_RIGHT;
                break;
            case LineDiscover:
                CurrentNavStatus = NAV_STATUS_LINE_DISCOVER;
                break;
            case CheckCrate:
                CurrentNavStatus = NAV_STATUS_CHECK_CRATE;
                break;
            default:
                CurrentNavStatus = NAV_STATUS_ERROR;
                break;
        }
    }

    // Always update status for next transfer
    SPI2BUF = CurrentNavStatus;
}

void TriggerSPIFollowerISR(void) {
    IFS1SET = _IFS1_SPI2RXIF_MASK; // Set the interrupt flag to trigger the ISR
    //IFS1SET = _IFS1_SPI1RXIF_MASK;
}