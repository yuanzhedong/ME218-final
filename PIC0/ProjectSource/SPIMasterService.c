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

bool SendSPICommand(NavCommand_t command) {
    // Check if previous transfer is complete
    if(SPI1STATbits.SPIBUSY) {
        return false;
    }
    
    // Send header first
    while(SPI1STATbits.SPITBF);
    SPI1BUF = SPI_HEADER_BYTE;
    LastSentCmd = command;
    LastTransferTime = ES_GetTime();
    
    return true;
}

void __ISR(_SPI_1_VECTOR, IPL6SOFT) SPIMasterISR(void) {
    static uint8_t byteCount = 0;
    static uint8_t receivedStatus = 0;
    
    uint8_t receivedByte = SPI1BUF;
    IFS1CLR = _IFS1_SPI1RXIF_MASK;

    // Check for timeout
    if((ES_GetTime() - LastTransferTime) > SPI_TIMEOUT_MS) {
        byteCount = 0;
        // Handle timeout
        ES_Event_t TimeoutEvent;
        TimeoutEvent.EventType = ES_SPI_TIMEOUT;
        PostSPIMasterService(TimeoutEvent);
        return;
    }

    switch(byteCount) {
        case 0:
            // Send command after header acknowledged
            while(SPI1STATbits.SPITBF);
            SPI1BUF = LastSentCmd;
            byteCount++;
            break;
            
        case 1:
            // Store received status
            receivedStatus = receivedByte;
            byteCount = 0;
            
            // Only post if status is valid and changed
            if(receivedStatus >= NAV_STATUS_OK && 
               receivedStatus <= NAV_STATUS_ERROR &&
               receivedStatus != PrevNavigatorStatus) {
                   
                ES_Event_t StatusEvent;
                StatusEvent.EventType = ES_NAVIGATOR_STATUS;
                StatusEvent.EventParam = receivedStatus;
                PostSPIMasterService(StatusEvent);
                PrevNavigatorStatus = receivedStatus;
            }
            break;
    }
}
Updated Follower Service:
c

Copy
// SPIFollowerService.c
#include "SPICommon.h"

static uint8_t CurrentStatus = NAV_STATUS_OK;
static uint32_t LastReceiveTime = 0;

void InitSPI(void) {
    // Previous initialization code...
    
    // Clear receive buffer properly
    uint8_t dummy = SPI2BUF;
    
    // Initialize last receive time
    LastReceiveTime = ES_GetTime();
}

void UpdateNavigatorStatus(NavStatus_t newStatus) {
    CurrentStatus = newStatus;
}

void __ISR(_SPI_2_VECTOR, IPL6SOFT) SPISlaveISR(void) {
    static uint8_t byteCount = 0;
    
    uint8_t receivedByte = SPI2BUF;
    IFS1CLR = _IFS1_SPI2RXIF_MASK;

    // Check for timeout
    if((ES_GetTime() - LastReceiveTime) > SPI_TIMEOUT_MS) {
        byteCount = 0;
    }
    LastReceiveTime = ES_GetTime();

    switch(byteCount) {
        case 0:
            if(receivedByte == SPI_HEADER_BYTE) {
                byteCount++;
                // Prepare to send status
                SPI2BUF = CurrentStatus;
            }
            break;
            
        case 1:
            // Process received command
            if(receivedByte >= NAV_CMD_MOVE && 
               receivedByte <= NAV_CMD_TURN_360) {
                ES_Event_t CmdEvent;
                CmdEvent.EventType = ES_NEW_PLANNER_CMD;
                CmdEvent.EventParam = receivedByte;
                PostNavigatorHSM(CmdEvent);
            }
            byteCount = 0;
            break;
    }
}

ES_Event_t RunSPIFollowerService(ES_Event_t ThisEvent) {
    ES_Event_t ReturnEvent;
    ReturnEvent.EventType = ES_NO_EVENT;
    
    switch(ThisEvent.EventType) {
        case ES_TIMEOUT:
            if(ThisEvent.EventParam == WATCHDOG_TIMER) {
                // Check for communication timeout
                if((ES_GetTime() - LastReceiveTime) > SPI_TIMEOUT_MS) {
                    // Handle communication loss
                    ES_Event_t TimeoutEvent;
                    TimeoutEvent.EventType = ES_SPI_TIMEOUT;
                    PostNavigatorHSM(TimeoutEvent);
                }
            }
            break;
    }
    
    return ReturnEvent;
}


bool SendSPICommand(NavCommand_t command) {
    // Check if previous transfer is complete
    if(SPI2STATbits.SPIBUSY) {
        return false;
    }
    
    // Send header first
    while(SPI2STATbits.SPITBF);
    SPI2BUF = SPI_HEADER_BYTE;
    LastSentCmd = command;
    LastTransferTime = ES_GetTime();
    
    return true;
}



void __ISR(_SPI_1_VECTOR, IPL6SOFT) SPIMasterISR(void) {
    CurrentNavigatorStatus = SPI1BUF;
    IFS1CLR = _IFS1_SPI1RXIF_MASK;

    // Only post if status changed
    if (PrevNavigatorStatus != CurrentNavigatorStatus) {
        ES_Event_t StatusEvent;
        StatusEvent.EventType = ES_NAVIGATOR_STATUS;
        StatusEvent.EventParam = CurrentNavigatorStatus;
        DB_printf("Received status: %d\r\n", CurrentNavigatorStatus);
        PostSPIMasterService(StatusEvent);
        PrevNavigatorStatus = CurrentNavigatorStatus;
    }
}