#include <xc.h>
#include <sys/attribs.h>
#include "ES_Configure.h"
#include "ES_Framework.h"
#include "SPIMasterService.h"
#include "dbprintf.h"

/*----------------------------- Module Defines ----------------------------*/

/*---------------------------- Module Variables ---------------------------*/
static uint8_t MyPriority;
volatile static uint16_t ReceivedStatus;
volatile static uint16_t CurrentNavigatorStatus;
volatile static uint16_t PrevNavigatorStatus = NAV_STATUS_IDLE;
static uint32_t LastTransferTime;
static uint8_t LastSentCmd;
static uint16_t QueryFreq = 10000; // in msq
#define DEBUG_CMD NAV_CMD_QUERY_STATUS

/*---------------------------- Module Functions ---------------------------*/
void InitSPI(void);
bool SendSPICommand(uint8_t command);
void __ISR(_SPI_1_VECTOR, IPL6SOFT) SPIMasterISR(void);


const char* TranslateNavCmdToStr(uint8_t command) {
    switch(command) {
        case NAV_CMD_QUERY_STATUS:
            return "NAV_CMD_QUERY_STATUS";
        case NAV_CMD_MOVE_FORWARD:
            return "NAV_CMD_MOVE_FORWARD";
        case NAV_CMD_MOVE_BACKWARD:
            return "NAV_CMD_MOVE_BACKWARD";
        case NAV_CMD_TURN_LEFT:
            return "NAV_CMD_TURN_LEFT";
        case NAV_CMD_TURN_RIGHT:
            return "NAV_CMD_TURN_RIGHT";
        case NAV_CMD_STOP:
            return "NAV_CMD_STOP";
        default:
            DB_printf("Unknown command: %x\r\n", command);
            return "UNKNOWN_COMMAND";
    }
}

const char* TranslateNavStatusToStr(uint8_t status) {
    switch(status) {
        case NAV_STATUS_MOVE_FORWARD_COMPLETE:
            return "NAV_STATUS_MOVE_FORWARD_COMPLETE";
        case NAV_STATUS_MOVE_BACKWARD_COMPLETE:
            return "NAV_STATUS_MOVE_BACKWARD_COMPLETE";
        case NAV_STATUS_TURN_LEFT_COMPLETE:
            return "NAV_STATUS_TURN_LEFT_COMPLETE";
        case NAV_STATUS_TURN_RIGHT_COMPLETE:
            return "NAV_STATUS_TURN_RIGHT_COMPLETE";
        case NAV_STATUS_STOP_COMPLETE:
            return "NAV_STATUS_STOP_COMPLETE";
        case NAV_STATUS_TURN_CW_COMPLETE:
            return "NAV_STATUS_TURN_CW_COMPLETE";
        case NAV_STATUS_TURN_CCW_COMPLETE:
            return "NAV_STATUS_TURN_CCW_COMPLETE";
        case NAV_STATUS_QUERY_COMPLETE:
            return "NAV_STATUS_QUERY_COMPLETE";
        case NAV_STATUS_ALIGN_COMPLETE:
            return "NAV_STATUS_ALIGN_COMPLETE";
        case NAV_STATUS_OK:
            return "NAV_STATUS_OK";
        case NAV_STATUS_ERROR:
            return "NAV_STATUS_ERROR";
        case NAV_STATUS_TURN_LEFT:
            return "NAV_STATUS_TURN_LEFT";
        case NAV_STATUS_TURN_RIGHT:
            return "NAV_STATUS_TURN_RIGHT";
        case NAV_STATUS_TURN_360:
            return "NAV_STATUS_TURN_360";
        case NAV_STATUS_IDLE:
            return "NAV_STATUS_IDLE";
        case NAV_STATUS_LINE_FOLLOW:
            return "NAV_STATUS_LINE_FOLLOW";
        case NAV_STATUS_ALIGN_TAPE:
            return "NAV_STATUS_ALIGN_TAPE";
        case NAV_STATUS_CHECK_INTERSECTION:
            return "NAV_STATUS_CHECK_INTERSECTION";
        case NAV_STATUS_LINE_DISCOVER:
            return "NAV_STATUS_LINE_DISCOVER";
        case NAV_STATUS_CHECK_CRATE:
            return "NAV_STATUS_CHECK_CRATE";
        case NAV_STATUS_INIT:
            return "NAV_STATUS_INIT";
        default:
            return "UNKNOWN_STATUS";
    }
}
/*------------------------------ Module Code ------------------------------*/
bool InitSPIMasterService(uint8_t Priority)
{
    MyPriority = Priority;
    InitSPI(); // Initialize SPI as master

    // Initialize Commands
    SPI1BUF = DEBUG_CMD;
    PrevNavigatorStatus = NAV_STATUS_IDLE;
    CurrentNavigatorStatus = NAV_STATUS_IDLE;

    // Start a timer to query the slave periodically
    ES_Timer_InitTimer(SPI_QUERY_TIMER, QueryFreq);
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

    ES_Timer_InitTimer(SPI_QUERY_TIMER, QueryFreq); // Restart the timer
    
    // Handle events here
    if (ThisEvent.EventType == ES_TIMEOUT && ThisEvent.EventParam == SPI_QUERY_TIMER)
    {
        // Query the slave for its status
        SendSPICommand(NAV_CMD_QUERY_STATUS);
    } else {
        if (ThisEvent.EventType == ES_NEW_NAV_CMD && ThisEvent.EventParam != 0) {
            SendSPICommand(ThisEvent.EventParam);
        }
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
    //LATBbits.LATB4 = 1; // Pull SS high
    TRISBbits.TRISB8 = 0;
    RPB8R = 0b0011;        // Map SDO to RB8
    TRISBbits.TRISB14 = 0; // Set SCK1 (RB14) as output
    // Step 2: Map SDI
    TRISBbits.TRISB5 = 1; // Input
    SDI1R = 0b0001;       // Map SDI1 to RB5
    // Step 3: Disable SPI Module
    SPI1CONbits.ON = 0;
    // Step 4: Clear the receive buffer
    uint8_t dummpy = SPI1BUF;
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
    SPI1CON2bits.AUDEN = 0;
    SPI1CONbits.MODE16 = 0; // Enable 8 bit transfers
    SPI1CONbits.MODE32 = 0; 
    // Step 9: Initialize Interrupts
    SPI1CONbits.SRXISEL = 0b01; // Interrupt when buffer is full
    IFS1CLR = _IFS1_SPI1RXIF_MASK;
    IPC7bits.SPI1IP = 6;
    IEC1SET = _IEC1_SPI1RXIE_MASK;
    // Step 10: Enable SPI
    SPI1CONbits.ON = 1;
    __builtin_enable_interrupts();
}

bool SendSPICommand(uint8_t command) {
    if(SPI1STATbits.SPIBUSY) {
        DB_printf("SPI is busy\r\n");
        return false;
    }
    
    // Send command directly
    while(SPI1STATbits.SPITBF);
    //DB_printf("[SPI] Sending command: %x\r\n", command);
    SPI1BUF = command;
    LastSentCmd = command;
    LastTransferTime = ES_Timer_GetTime();
    return true;
}

void __ISR(_SPI_1_VECTOR, IPL6SOFT) SPIMasterISR(void) {
    
    uint8_t receivedByte = SPI1BUF;
    IFS1CLR = _IFS1_SPI1RXIF_MASK;

    // Check for timeout
    if((ES_Timer_GetTime() - LastTransferTime) > SPI_TIMEOUT_MS) {
        // Handle timeout
        DB_printf("SPI Timeout\r\n");
        return;
    } else {
        ReceivedStatus = receivedByte;
        ES_Event_t CmdEvent;
        if (ReceivedStatus == NAV_STATUS_OK || ReceivedStatus == NAV_STATUS_ERROR || ReceivedStatus == NAV_STATUS_IDLE) {
            // Received status
            DB_printf("[SPI] Received status: %s\r\n", TranslateNavStatusToStr(ReceivedStatus));
            CmdEvent.EventType = ES_NAVIGATOR_HEALTH_CHECK;
            CmdEvent.EventParam = ReceivedStatus;
            PostPlannerHSM(CmdEvent);
        } else if (PrevNavigatorStatus != ReceivedStatus) {
            DB_printf("[SPI] Previous status: %s\r\n", TranslateNavStatusToStr(PrevNavigatorStatus));
            DB_printf("[SPI] Received status: %s\r\n", TranslateNavStatusToStr(ReceivedStatus));
            CmdEvent.EventType = ES_NAVIGATOR_STATUS_CHANGE;
            CmdEvent.EventParam = ReceivedStatus;
            PostPlannerHSM(CmdEvent);
            PrevNavigatorStatus = ReceivedStatus;
        }
    }
    LastTransferTime = ES_Timer_GetTime();
}

void UpdateNavigatorStatus(uint8_t newStatus) {
    CurrentNavigatorStatus = newStatus;
}