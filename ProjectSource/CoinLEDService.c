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
#include "../ProjectHeaders/PIC32_SPI_HAL.h"

/*---------------------------- Module Variables ---------------------------*/
// with the introduction of Gen2, we need a module level Priority variable
static uint8_t MyPriority;

#define LED1 LATAbits.LATA3
#define LED2 LATAbits.LATA4

static uint8_t total_coins = 0;
static CoinLEDServiceState_t currentState = InitPState;

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
    TRISAbits.TRISA3 = 0;   //output for LED1
    TRISAbits.TRISA4 = 0;   //output for LED2

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

bool PostCoinLEDService(ES_Event_t ThisEvent)
{
    return ES_PostToService(MyPriority, ThisEvent);
}

ES_Event_t RunCoinLEDService(ES_Event_t ThisEvent)
{
    ES_Event_t ReturnEvent;
    ReturnEvent.EventType = ES_NO_EVENT; // assume no errors

    switch (currentState)
    {
    case InitPState: // If current state is initial Psedudo State
    {
        if (ThisEvent.EventType == ES_INIT) // only respond to ES_Init
        {
            LED1 = 0;
            LED2 = 0;
            currentState = WaitForCoin;
        }
    }
    break;

    case WaitForCoin: // If current state is state one
    {
        puts("###dfsd##");

        switch (ThisEvent.EventType)
        {
        case ES_NEW_COIN:
        {
            LED1 = 1;
            total_coins += 1;
            if (total_coins == 2)
            {
                LED2 = 1;
                currentState = GameStart;
                ES_Event_t StartGameEvent;
                StartGameEvent.EventType = ES_START_GAME;
                ES_PostAll(StartGameEvent);
            }
        }
        break;
        case ES_NEW_KEY:
        {
            if ('a' == ThisEvent.EventParam)
            {
                puts("Insert new coin...");
                ES_Event_t EventNewCoin = {ES_NEW_COIN, ThisEvent.EventParam};
                PostCoinLEDService(EventNewCoin);
            }
        }
        break;
        // repeat cases as required for relevant events
        default:;
        } // end switch on CurrentEvent
    }

    case GameStart:
    {
        switch (ThisEvent.EventType)
        {
        case ES_END_GAME:
        {
            currentState = WaitForCoin;
            total_coins = 0;
            LED1 = 0;
            LED2 = 0;
        }

        break;

        default:
            break;
        }
    }
    break;
    // repeat state pattern as required for other states
    default:;
    } // end switch on Current State
    return ReturnEvent;
}

CoinLEDServiceState_t QueryCoinLEDService(void)
{
    return currentState;
}
