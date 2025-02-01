#include "../ProjectHeaders/PotentiometerService.h"

#include "ES_Configure.h"
#include "ES_Framework.h"
#include "ES_Timers.h"
#include "PIC32_AD_Lib.h"

#define POTENTIOMETER_THRESHOLD 10

static uint8_t MyPriority;
static uint16_t LastPotentiometerValue = 0;

uint32_t adcResults[1]; // Array to hold the ADC result
bool SignalDetected = false;

uint8_t InitPotentiometerService(uint8_t Priority)
{
    MyPriority = Priority;
    //ES_Timer_InitTimer(POTENTIOMETER_SERVICE_TIMER, 100);

    // Configure RB12 as an analog input
    TRISBbits.TRISB12 = 1; // Set RB12 as input
    ANSELBbits.ANSB12 = 1; // Set RB12 as analog

    // Initialize the ADC to read from RB12 (AN12)
    if (!ADC_ConfigAutoScan(BIT12HI))
       return false;
    PostPotentiometerService((ES_Event_t){.EventType = ES_INIT});
    return true;
}

uint8_t PostPotentiometerService(ES_Event_t ThisEvent)
{
    return ES_PostToService(MyPriority, ThisEvent);
}

ES_Event_t RunPotentiometerService(ES_Event_t ThisEvent)
{
    ES_Event_t ReturnEvent;
    ReturnEvent.EventType = ES_NO_EVENT;

    switch (ThisEvent.EventType)
    {
    case ES_INIT:
        ES_Timer_InitTimer(POTENTIOMETER_SERVICE_TIMER, 1000);
        break;

    case ES_TIMEOUT:
        ES_Timer_InitTimer(POTENTIOMETER_SERVICE_TIMER, 1000);
        DB_printf("Potentiometer Value: %d\n", LastPotentiometerValue);
        ADC_MultiRead(adcResults); // Read the ADC value
        uint16_t CurrentPotentiometerValue = (uint16_t)adcResults[0];
        DB_printf("Potentiometer Value: %d\n", CurrentPotentiometerValue);
        if (abs(CurrentPotentiometerValue - LastPotentiometerValue) > POTENTIOMETER_THRESHOLD)
        {
            LastPotentiometerValue = CurrentPotentiometerValue;
            ES_Event_t NewEvent;
            NewEvent.EventType = ES_POTENTIOMETER_CHANGED;
            NewEvent.EventParam = CurrentPotentiometerValue;
            DB_printf("Potentiometer Value: %d\n", CurrentPotentiometerValue);
            ES_PostAll(NewEvent);
        }
        break;

    default:
        break;
    }

    return ReturnEvent;
}