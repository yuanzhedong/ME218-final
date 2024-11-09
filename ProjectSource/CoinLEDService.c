#include "../ProjectHeaders/CoinLEDService.h"

// Hardware
#include <xc.h>
// #include <proc/p32mx170f256b.h>

// Event & Services Framework
#include "ES_Configure.h"
#include "ES_Framework.h"
#include "ES_DeferRecall.h"
#include "ES_Port.h"
#include "terminal.h"
#include "dbprintf.h"

/*---------------------------- Module Variables ---------------------------*/
// with the introduction of Gen2, we need a module level Priority variable
static uint8_t MyPriority;

bool InitCoinLEDService(uint8_t Priority)
{
    ES_Event_t ThisEvent;

    MyPriority = Priority;

    // When doing testing, it is useful to announce just which program
    // is running.
    clrScrn();
    puts("\rStarting CoinLEDService\r");
    DB_printf("compiled at %s on %s\n", __TIME__, __DATE__);
    DB_printf("\n\r\n");

    /********************************************
     in here you write your initialization code
     *******************************************/
    SPISetup_BasicConfig(SPI_SPI1);
    SPISetup_SetLeader(SPI_SPI1, SPI_SMP_MID);
    SPISetup_MapSSOutput(SPI_SPI1, SPI_RPA0);
    SPISetup_MapSDOutput(SPI_SPI1, SPI_RPA1);
    SPISetup_MapSSOutput(SPI_SPI1, SPI_RPA3); //coin LED 1
    SPISetup_MapSDOutput(SPI_SPI1, SPI_RPA4); // coin LED 2
    SPI1BUF;
    SPISetEnhancedBuffer(SPI_SPI1, 1);
    SPISetup_SetBitTime(SPI_SPI1, 10000);
    SPISetup_SetXferWidth(SPI_SPI1, SPI_16BIT);
    SPISetup_SetActiveEdge(SPI_SPI1, SPI_SECOND_EDGE);
    SPISetup_SetClockIdleState(SPI_SPI1, SPI_CLK_HI);
    SPI1CONbits.FRMPOL = 0;
    SPISetup_EnableSPI(SPI_SPI1);

    // post the initial transition event
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

/****************************************************************************
 Function
     PostTestHarnessService0

 Parameters
     ES_Event ThisEvent ,the event to post to the queue

 Returns
     bool false if the Enqueue operation failed, true otherwise

 Description
     Posts an event to this state machine's queue
 Notes

 Author
     J. Edward Carryer, 10/23/11, 19:25
****************************************************************************/
bool PostTestHarnessService0(ES_Event_t ThisEvent)
{
    return ES_PostToService(MyPriority, ThisEvent);
}

/****************************************************************************
 Function
    RunTestHarnessService0

 Parameters
   ES_Event : the event to process

 Returns
   ES_Event, ES_NO_EVENT if no error ES_ERROR otherwise

 Description
   add your description here
 Notes

 Author
   J. Edward Carryer, 01/15/12, 15:23
****************************************************************************/
ES_Event_t RunTestHarnessService0(ES_Event_t ThisEvent)
{
    ES_Event_t ReturnEvent;
    ReturnEvent.EventType = ES_NO_EVENT; // assume no errors
    static char DeferredChar = '1';

#ifdef _INCLUDE_BYTE_DEBUG_
    _HW_ByteDebug_SetValueWithStrobe(ENTER_RUN);
#endif
    switch (ThisEvent.EventType)
    {
    case ES_INIT:
    {
        // ES_Timer_InitTimer(SERVICE0_TIMER, HALF_SEC);
        puts("Service 00:");
        DB_printf("\rES_INIT received in Service %d\r\n", MyPriority);
    }
    break;
    case ES_TIMEOUT: // re-start timer & announce
    {
        // ES_Timer_InitTimer(SERVICE0_TIMER, FIVE_SEC);
        DB_printf("ES_TIMEOUT received from Timer %d in Service %d\r\n",
                  ThisEvent.EventParam, MyPriority);
    }
    break;
    case ES_SHORT_TIMEOUT: // lower the line & announce
    {
        puts("\rES_SHORT_TIMEOUT received\r\n");
    }
    break;
    case ES_NEW_KEY: // announce
    {
        DB_printf("ES_NEW_KEY received with -> %c <- in Service 0\r\n",
                  (char)ThisEvent.EventParam);
        if ('d' == ThisEvent.EventParam)
        {
            ThisEvent.EventParam = DeferredChar++; //
            if (ES_DeferEvent(DeferralQueue, ThisEvent))
            {
                puts("ES_NEW_KEY deferred in Service 0\r");
            }
        }
        if ('r' == ThisEvent.EventParam)
        {
            ThisEvent.EventParam = 'Q'; // This one gets posted normally
            PostTestHarnessService0(ThisEvent);
            // but we slide the deferred events under it so it(they) should come out first
            if (true == ES_RecallEvents(MyPriority, DeferralQueue))
            {
                puts("ES_NEW_KEY(s) recalled in Service 0\r");
                DeferredChar = '1';
            }
        }
#ifdef TEST_INT_POST
        if ('p' == ThisEvent.EventParam)
        {
            StartTMR2();
        }
#endif
    }
    break;
    default:
    {
    }
    break;
    }

    return ReturnEvent;
}

/***************************************************************************
 private functions
 ***************************************************************************/
#ifdef BLINK_LED
#define LED LATBbits.LATB6
static void InitLED(void)
{
    LED = 0;              // start with it off
    TRISBbits.TRISB6 = 0; // set RB6 as an output
}

static void BlinkLED(void)
{
    // toggle state of LED
    LED = ~LED;
}
#endif

#ifdef TEST_INT_POST
#include <sys/attribs.h> // for ISR macors

// for testing posting from interrupts.
// Intializes TMR2 to gerenate an interrupt at 100ms
static void InitTMR2(void)
{
    // turn timer off
    T2CONbits.ON = 0;
    // Use internal peripheral clock
    T2CONbits.TCS = 0;
    // setup for 16 bit mode
    T2CONbits.T32 = 0;
    // set prescale to 1:1
    T2CONbits.TCKPS = 0;
    // load period value
    PR2 = 2000 - 1; // creates a 100ms period with a 20MHz peripheral clock
    // set priority
    IPC2bits.T2IP = 2;
    // clear interrupt flag
    IFS0bits.T2IF = 0;
    // enable the timer interrupt
    IEC0bits.T2IE = 1;
}

// Clears and Starts TMR2
static void StartTMR2(void)
{
    // clear timer
    TMR2 = 0;
    // start timer
    // LATBbits.LATB14 = 0;
    T2CONbits.ON = 1;
}

void __ISR(_TIMER_2_VECTOR, IPL2AUTO) Timer2ISR(void)
{
    // clear flag
    IFS0bits.T2IF = 0;
    // post event
    static ES_Event_t interruptEvent = {ES_SHORT_TIMEOUT, 0};
    PostTestHarnessService0(interruptEvent);

    // stop timer
    T2CONbits.ON = 0;
    return;
}
#endif
/*------------------------------- Footnotes -------------------------------*/
/*------------------------------ End of file ------------------------------*/
