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

static uint8_t MAX_LIVES = 100;
static uint8_t currentLives = 100;
static uint8_t justTouched = 0;

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

    TRISBbits.TRISB5 = 0;  // Vibration motor output
    TRISBbits.TRISB10 = 0; // Live LED1 output
    TRISBbits.TRISB11 = 0; // Live LED2 output
    TRISBbits.TRISB8 = 0;  // Buzzer output

    LATBbits.LATB5 = 0;  // Vibration motor off
    LATBbits.LATB10 = 0; // live LED 1 off
    LATBbits.LATB11 = 0; // live LED 2 off
    LATBbits.LATB8 = 0;  // Buzzer off

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
    ;
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
        // if (ES_Timer_GetTime() - justTouched < 1000) { // ignore another touch within 1 sec
        //     break;
        // }

        ES_Event_t ThisEvent;
        ThisEvent.EventType = ES_START_VIBRATION;
        ThisEvent.EventParam = QUATER_SEC;
        PostLiveService(ThisEvent);
        ES_Event_t liveEvent;
        liveEvent.EventType = ES_MINUS_LIVE;
        liveEvent.EventParam = 1; // minus live when touch boundary
        PostLiveService(liveEvent);
        justTouched = ES_Timer_GetTime();
    }
    break;
    case ES_START_GAME:
    {
        LATBbits.LATB10 = 1;
        LATBbits.LATB11 = 1;
        currentLives = MAX_LIVES;
    }
    break;

    case ES_START_VIBRATION:
    {
        uint16_t vibration_time = ThisEvent.EventParam;
        ES_Timer_InitTimer(LIVE_SERVICE_TIMER, vibration_time);
        LATBbits.LATB5 = 1;
        LATBbits.LATB8 = 1;
    }
    break;

    case ES_TIMEOUT:
    {
        LATBbits.LATB5 = 0; // stop vibration
        LATBbits.LATB8 = 0; // stop buzz
    }

    case ES_MINUS_LIVE:
    {
        //DB_printf("Current live: %d\n", currentLives);
        
        if (ThisEvent.EventParam == 14) {
            ThisEvent.EventParam = 0;
        }
        if (ThisEvent.EventParam != 0) {
        DB_printf("minus live: %d\n", ThisEvent.EventParam);
        }
        if (currentLives <= ThisEvent.EventParam) {
            ES_Event_t endGame;
            endGame.EventType = ES_END_GAME;
            ES_PostAll(endGame);
            puts("game ends\n");
            break;
        }
        currentLives -= ThisEvent.EventParam;
        
        ES_Event_t event;
        event.EventType = ES_UPDATE_LIVE;
        event.EventParam = currentLives;
        PostDisplayService(event);
    }

    break;
    case ES_NEW_KEY:
        {
            if ('t' == ThisEvent.EventParam)
            {
                puts("Touch...");
                ES_Event_t EventTouch = {ES_TOUCH_BOUNDARY, 0};
                PostLiveService(EventTouch);
            }
        }
    break;
    default:;
    }
}

