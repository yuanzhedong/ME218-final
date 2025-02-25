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
void TriggerSPIFollowerISR(void); // Function prototype for triggering ISR

/*------------------------------ Module Code ------------------------------*/
bool InitSPIFollowerService(uint8_t Priority)
{
    MyPriority = Priority;
    InitSPI_v2(); // Initialize SPI as slave

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

    // if (ThisEvent.EventType == ES_NEW_KEY) {
    //     DB_printf("SPIFollowerService received new key event\r\n");
    //     TriggerSPIFollowerISR();
    // }

    // Handle events here

    return ReturnEvent;
}


void InitSPI(void)
{
    // Step 0: Disable analog function on all SPI pins
    
    // Step 1: Map SPI Inputs/Outputs to all desired pins
    TRISBbits.TRISB5 = 1;  // Set SDI1 (RB5) as input
    SDI1R = 0b0001;        // Map SDI1 to RB5
    TRISBbits.TRISB8 = 0;  // Set SDO1 (RB8) as output
    RPB8R = 0b0011;        // Map SDO1 to RB8
    TRISBbits.TRISB14 = 1; // Set SCK1 (RB14) as input
    ANSELBbits.ANSB14 = 0; // Disable analog function on SCK1
    TRISBbits.TRISB4 = 1;  // Set SS1 (RB4) as input
    
    
    // Step 2: Disable SPI Module
    SPI1CONbits.ON = 0;

    // Step 3: Clear the receive buffer
    uint8_t dummy = SPI1BUF;

    // Step 4: Enable Enhanced Buffer
    SPI1CONbits.ENHBUF = 0;

    // Step 5: Set Baudrate (not applicable for slave mode)

    // Step 6: Clear the SPIROV Bit
    SPI1STATbits.SPIROV = 0;

    // Step 7: Write desired settings to SPIxCON
    SPI1CONbits.MSTEN = 0; // Slave mode
    SPI1CONbits.CKE = 0;   // Reads on 2nd edge
    SPI1CONbits.CKP = 1;   // SCK idles high
    //SPI1CONbits.SSEN = 1;  // Enable SS pin

    // Configure SPI for 8-bit communication
    SPI1CONbits.MODE16 = 0; // 8-bit mode
    SPI1CONbits.MODE32 = 0; // 8-bit mode
    // Step 8: Initialize Interrupts
    SPI1CONbits.SRXISEL = 0b01; // Interrupt when buffer is full
    IFS0CLR = _IFS1_SPI1RXIF_MASK;
    IPC7bits.SPI1IP = 6;
    IEC0SET = _IFS1_SPI1RXIF_MASK;

    // Step 9: Enable SPI
    SPI1CONbits.ON = 1;

    __builtin_enable_interrupts();
    lastReceiveTime = ES_Timer_GetTime();
}
void InitSPI_v2(void)
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


// void __ISR(_SPI_1_VECTOR, IPL6SOFT) SPIFollowerISR(void) {
//     uint8_t receivedByte = SPI1BUF;
//     IFS0CLR = _IFS1_SPI1RXIF_MASK; // Clear the interrupt flag

//     DB_printf("Received byte: %d\r\n", receivedByte); // Add debug print

//     // Process command directly
//     if(receivedByte >= NAV_CMD_MOVE && receivedByte <= NAV_CMD_TURN_360) {
//         ES_Event_t CmdEvent;
//         CmdEvent.EventType = ES_NEW_PLANNER_CMD;
//         CmdEvent.EventParam = receivedByte;
//         DB_printf("Received command: %d\r\n", receivedByte);
//         PostNavigatorHSM(CmdEvent);
//     } else if (receivedByte == NAV_CMD_QUERY_STATUS) {
//         // Update status based on Navigator state
//         DB_printf("Received status query\r\n");
//         NavigatorState_t currentState = QueryNavigatorHSM();
//         switch (currentState) {
//             case Init:
//                 CurrentStatus = NAV_STATUS_INIT;
//                 break;
//             case Idle:
//                 CurrentStatus = NAV_STATUS_IDLE;
//                 break;
//             case LineFollow:
//                 CurrentStatus = NAV_STATUS_LINE_FOLLOW;
//                 break;
//             case AlignBeacon:
//                 CurrentStatus = NAV_STATUS_ALIGN_BEACON;
//                 break;
//             case CheckIntersection:
//                 CurrentStatus = NAV_STATUS_CHECK_INTERSECTION;
//                 break;
//             case TurnLeft:
//                 CurrentStatus = NAV_STATUS_TURN_LEFT;
//                 break;
//             case TurnRight:
//                 CurrentStatus = NAV_STATUS_TURN_RIGHT;
//                 break;
//             case LineDiscover:
//                 CurrentStatus = NAV_STATUS_LINE_DISCOVER;
//                 break;
//             case CheckCrate:
//                 CurrentStatus = NAV_STATUS_CHECK_CRATE;
//                 break;
//             default:
//                 CurrentStatus = NAV_STATUS_ERROR;
//                 break;
//         }
//     }

//     // Always update status for next transfer
//     SPI1BUF = CurrentStatus;
// }
void __ISR(_SPI_2_VECTOR, IPL6SOFT) SPIFollowerISR(void) {
    uint8_t receivedByte = SPI2BUF;
    IFS1CLR = _IFS1_SPI2RXIF_MASK; // Clear the interrupt flag

    DB_printf("Received byte: %d\r\n", receivedByte); // Add debug print

    // Process command directly
    if(receivedByte >= NAV_CMD_MOVE_FORWARD && receivedByte <= NAV_CMD_TURN_360) {
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
    SPI2BUF = 0x02;
}

void TriggerSPIFollowerISR(void) {
    IFS1SET = _IFS1_SPI2RXIF_MASK; // Set the interrupt flag to trigger the ISR
    //IFS1SET = _IFS1_SPI1RXIF_MASK;
}