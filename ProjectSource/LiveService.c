#include "../ProjectHeaders/LiveService.h"

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
#include "../ProjectHeaders/PIC32_SPI_HAL.h"

/*---------------------------- Module Variables ---------------------------*/
// with the introduction of Gen2, we need a module level Priority variable
static uint8_t MyPriority;

static uint8_t MAX_LIVES = 100000000;
static uint8_t currentLives = 1000000;

#define ONE_SEC 1000
#define HALF_SEC (ONE_SEC / 2)
#define QUATER_SEC (HALF_SEC / 2)

bool InitLiveService(uint8_t Priority)
{
    ES_Event_t ThisEvent;

    MyPriority = Priority;

    // When doing testing, it is useful to announce just which program
    // is running.
    clrScrn();
    puts("\rStarting LiveService\r");
    DB_printf("compiled at %s on %s\n", __TIME__, __DATE__);
    DB_printf("\n\r\n");

    TRISBbits.TRISB5 = 0; // Vibration motor output
    TRISBbits.TRISB6 = 0; // Live LED1 output
    TRISBbits.TRISB7 = 0; // Live LED2 output
    TRISBbits.TRISB8 = 0; // Buzzer output

    LATBbits.LATB5 = 0; // Vibration motor off
    LATBbits.LATB6 = 0; // live LED 1 off
    LATBbits.LATB7 = 0; // live LED 2 off
    LATBbits.LATB8 = 0; // Buzzer off

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

bool PostLiveService(ES_Event_t ThisEvent)
{
    return ES_PostToService(MyPriority, ThisEvent);
}

ES_Event_t RunLiveService(ES_Event_t ThisEvent)
{
    ES_Event_t ReturnEvent;
    ReturnEvent.EventType = ES_NO_EVENT; // assume no errors

    switch (ThisEvent.EventType)
    {
    case ES_INIT:
    {
        ;
    }
    break;

    case ES_TOUCH_BOUNDARY:
    {

            ES_Event_t ThisEvent;
            ThisEvent.EventType = ES_START_VIBRATION;
            PostLiveService(ThisEvent);
    }
    break;
    case ES_START_GAME:
    {
        LATBbits.LATB6 = 1;
        LATBbits.LATB7 = 1;
        currentLives = MAX_LIVES;
    }
    break;

    case ES_START_VIBRATION:
    {
        ES_Timer_InitTimer(LIVE_SERVICE_TIMER, QUATER_SEC);
        LATBbits.LATB5 = 1;
        LATBbits.LATB8 = 1;
    }
    break;

    case ES_TIMEOUT:
    {
        LATBbits.LATB5 = 0; // stop vibration
        LATBbits.LATB8 = 0;
    }
    break;
    default:;
    }
}
