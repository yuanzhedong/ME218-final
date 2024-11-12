#include "../ProjectHeaders/ServoService.h"

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
#include "../ProjectHeaders/PIC32_AD_Lib.h"

/*---------------------------- Module Variables ---------------------------*/
// with the introduction of Gen2, we need a module level Priority variable
static uint8_t MyPriority;
static uint32_t adcResults[1];

static uint16_t LastWeightValue = 0;
uint16_t CurrentWeightValue = 0;
uint16_t WeightValueThreshold = 10; // need to tune

#define ONE_SEC 1000
#define HALF_SEC (ONE_SEC / 2)
#define TWO_SEC (ONE_SEC * 2)
#define FIVE_SEC (ONE_SEC * 5)

bool InitWeightSensor(uint8_t Priority)
{
    ES_Event_t ThisEvent;

    MyPriority = Priority;

    // When doing testing, it is useful to announce just which program
    // is running.
    clrScrn();
    puts("\rStarting WeightSensor\r");
    DB_printf("compiled at %s on %s\n", __TIME__, __DATE__);
    DB_printf("\n\r\n");

    if (!ADC_ConfigAutoScan(BIT12HI))
    {
        return false; // Return false if ADC configuration fails
    }

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

bool PostWeightSensor(ES_Event_t ThisEvent)
{
    return ES_PostToService(MyPriority, ThisEvent);
}

ES_Event_t RunWeightSensor(ES_Event_t ThisEvent)
{
    ES_Event_t ReturnEvent;
    ReturnEvent.EventType = ES_NO_EVENT; // assume no errors

    switch (ThisEvent.EventType)
    {
    case ES_INIT:
    {
        // We don't need to sample the weight that frequently
        ES_Timer_InitTimer(WEIGHT_SENSOR_TIMER, ONE_SEC);
        puts("Start sampling weight...");
    }
    break;

    case ES_TIMEOUT:
    {
        ES_Timer_InitTimer(WEIGHT_SENSOR_TIMER, ONE_SEC);
        ADC_MultiRead(adcResults);
        CurrentWeightValue = (uint16_t)adcResults[0];
        if (abs(LastWeightValue - CurrentWeightValue) > WeightValueThreshold)
        {
            LastWeightValue = CurrentWeightValue;
            DB_printf("Current Weight: %d\n", CurrentWeightValue);
            // TODO: post weight value to LED
        }
    }
    break;
    default:;
    }
}
