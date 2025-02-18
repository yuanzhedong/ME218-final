#include <xc.h>
#include <sys/attribs.h>
#include "ES_Configure.h"
#include "ES_Framework.h"
#include "SPIMasterService.h"
#include "dbprintf.h"

/*----------------------------- Module Defines ----------------------------*/

/*---------------------------- Module Variables ---------------------------*/
static uint8_t MyPriority;
volatile static uint16_t ReceivedCmd;
volatile static uint16_t CurrentNavigatorStatus;
volatile static uint16_t PrevNavigatorStatus;
static uint32_t LastTransferTime;
static uint8_t LastSentCmd;

#define DEBUG_CMD NAV_CMD_QUERY_STATUS

/*---------------------------- Module Functions ---------------------------*/
void InitSPI(void);
bool SendSPICommand(uint8_t command);
void __ISR(_SPI_1_VECTOR, IPL6SOFT) SPIMasterISR(void);

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
    ES_Timer_InitTimer(SPI_QUERY_TIMER, 1000);
    puts("Started SPI Query Timer");
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
    if (ThisEvent.EventType == ES_TIMEOUT && ThisEvent.EventParam == SPI_QUERY_TIMER)
    {
        puts("Querying slave for status");
        // Query the slave for its status
        SendSPICommand(DEBUG_CMD);
        ES_Timer_InitTimer(SPI_QUERY_TIMER, 1000); // Restart the timer
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
    // Check if previous transfer is complete
    //LATBbits.LATB4 = 0; // Pull SS low

    for (volatile uint32_t i = 0; i < 1000000; i++);
    for (volatile uint32_t i = 0; i < 1000000; i++);
    for (volatile uint32_t i = 0; i < 1000000; i++);

    for (volatile uint32_t i = 0; i < 1000000; i++);
    if(SPI1STATbits.SPIBUSY) {
        DB_printf("SPI is busy\r\n");
        return false;
    }
    
    // Add a delay of 1 second
    

    // Send command directly
    while(SPI1STATbits.SPITBF);
    DB_printf("Sending command: %d\r\n", command);
    SPI1BUF = command;
    LastSentCmd = command;
    LastTransferTime = ES_Timer_GetTime();
    uint8_t receivedByte = SPI1BUF;

        for (volatile uint32_t i = 0; i < 1000000; i++);
    for (volatile uint32_t i = 0; i < 1000000; i++);
    for (volatile uint32_t i = 0; i < 1000000; i++);

    for (volatile uint32_t i = 0; i < 1000000; i++);
    //LATBbits.LATB4 = 1; // Pull SS high    
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
        ReceivedCmd = receivedByte;
        ES_Event_t CmdEvent;
        DB_printf("Received status: %d\r\n", ReceivedCmd);
    }
    LastTransferTime = ES_Timer_GetTime();
}

void UpdateNavigatorStatus(uint8_t newStatus) {
    CurrentNavigatorStatus = newStatus;
}


// bool SendSPICommand(NavCommand_t command) {
//     // Check if previous transfer is complete
//     if(SPI2STATbits.SPIBUSY) {
//         return false;
//     }
    
//     // Send header first
//     while(SPI2STATbits.SPITBF);
//     SPI2BUF = SPI_HEADER_BYTE;
//     LastSentCmd = command;
//     LastTransferTime = ES_GetTime();
    
//     return true;
// }



// void __ISR(_SPI_1_VECTOR, IPL6SOFT) SPIMasterISR(void) {
//     CurrentNavigatorStatus = SPI1BUF;
//     IFS1CLR = _IFS1_SPI1RXIF_MASK;

//     // Only post if status changed
//     if (PrevNavigatorStatus != CurrentNavigatorStatus) {
//         ES_Event_t StatusEvent;
//         StatusEvent.EventType = ES_NAVIGATOR_STATUS;
//         StatusEvent.EventParam = CurrentNavigatorStatus;
//         DB_printf("Received status: %d\r\n", CurrentNavigatorStatus);
//         PostSPIMasterService(StatusEvent);
//         PrevNavigatorStatus = CurrentNavigatorStatus;
//     }
// }