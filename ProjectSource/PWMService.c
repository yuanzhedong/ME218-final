/****************************************************************************

  Source file for PWM Service
  based on the Gen 2 Events and Services Framework

 ****************************************************************************/

#include "../ProjectHeaders/PWMService.h"
#include "ES_Configure.h"
#include "ES_Framework.h"
#include "ES_Timers.h"
#include <xc.h> // Include processor files - each processor file is guarded.

// Module-level variables
static uint8_t MyPriority;
static uint16_t DutyCycle = 70;
static uint16_t PRx = 2399;
bool Forward = true;

void changeDutyCycle(uint16_t newDutyCycle)
{
    if (newDutyCycle < 0 || newDutyCycle > 100)
    {
        return;
    }
    DB_printf("Changing duty cycle to %d%%\r\n", newDutyCycle);
    DutyCycle = newDutyCycle;
    if (Forward)
    {
        OC4R = (PR3 + 1) * DutyCycle / 100;
        OC4RS = (PR3 + 1) * DutyCycle / 100;
    }
    else
    {
        OC4R = (PR3 + 1) * (100 - DutyCycle) / 100;
        OC4RS = (PR3 + 1) * (100 - DutyCycle) / 100;
    }
}

// Function to initialize the PWM Service
bool InitPWMService(uint8_t Priority)
{
    MyPriority = Priority;
    // Initialization code for PWM hardware
    // ...
    // Example initialization code for PWM hardware using TMR and OC
    // Configure Timer and Output Compare for PWM

    // prescale 8
    // 20 MHz / 8 = 2.5 MHz instruction clock
    // 2.5 MHz / 200 Hz = 12500
    // PRx = 12500 - 1 = 12499

    // 2.5 MHz / 250 Hz = 10000
    // PRx = 10000 - 1 = 9999

    // 2.5 MHz / 500 Hz = 5000
    // PRx = 5000 - 1 = 4999

    // 2.5 MHz / 1000 Hz = 2500
    //  PRx = 2500 - 1 = 2499

    // 2.5 MHz / 2000 Hz = 1250
    //  PRx = 1250 - 1 = 1249

    // 2.5 MHz / 10000 Hz = 250
    // PRx = 250 - 1 = 249

    // prescale 64
    // 20 MHz / 64 = 312.5 kHz instruction clock
    // 312.5 kHz / 200 Hz = 1562.5
    // PRx = 1562 - 1 = 1561

    // Assuming Timer 2 and Output Compare 4 for PWM
    T3CONbits.TCKPS = 0b000; // Set prescaler to 1:1
    PR3 = PRx;               // Set period register for 1 kHz PWM frequency
    TMR3 = 0;                // Clear timer register

    OC4CON = 0x0000;                            // Clear OC4CON register
    OC4CONbits.OCTSEL = 1;                      // Select Timer 2 as clock source
    OC4CONbits.OCM = 0b110;                     // Set Output Compare mode to PWM
    OC4R = (PRx + 1) * (float)DutyCycle / 100;  // Set initial duty cycle to 50%
    OC4RS = (PRx + 1) * (float)DutyCycle / 100; // Set secondary compare register

    TRISAbits.TRISA0 = 0;
    TRISAbits.TRISA1 = 0;
    LATAbits.LATA0 = 1;
    LATAbits.LATA1 = 0;

    // Configure RA2 as output for OC1
    TRISAbits.TRISA2 = 0; // Set RA2 as output
    RPA2R = 0b0101;       // Map OC4 to RA2

    // Start Timer 2
    T3CONbits.ON = 1;
    // Start Output Compare 1
    OC4CONbits.ON = 1;

    ANSELAbits.ANSA1 = 0;
    ANSELAbits.ANSA0 = 0;

    puts("PWM Service initialized.\r\n");
    return true;
}

// Function to post an event to the PWM Service
bool PostPWMService(ES_Event_t ThisEvent)
{
    return ES_PostToService(MyPriority, ThisEvent);
}

// Function to run the PWM Service
ES_Event_t RunPWMService(ES_Event_t ThisEvent)
{
    ES_Event_t ReturnEvent;
    ReturnEvent.EventType = ES_NO_EVENT; // Assume no errors

    switch (ThisEvent.EventType)
    {
    case ES_INIT:
        // Initialization code for the service
        // ...
        break;

    case ES_NEW_KEY:
        Forward = !Forward;
        if (Forward)
        {
            puts("Forward.\r\n");
            changeDutyCycle(DutyCycle);
            LATAbits.LATA1 = 0;
        }
        else
        {
            changeDutyCycle(DutyCycle);
            LATAbits.LATA1 = 1;
            puts("Back.\r\n");
        }

        // Handle timeout events
        // ...
        break;

        // Add other event types here
    case ES_POTENTIOMETER_CHANGED:
        // Handle potentiometer changed events
        // Rescale the potentiometer reading from 0-1024 to 0-100
        // uint16_t scaledValue = ((float)ThisEvent.EventParam * 100) / 1024;

        // uint16_t scaledValue = ThisEvent.EventParam * 100 / 1024;

        changeDutyCycle(ThisEvent.EventParam * 100 / 1024);
        break;

    case ES_NEW_DUTY_CYCLE:
        // changeDutyCycle(ThisEvent.EventParam);
        break;

    default:
        break;
    }

    return ReturnEvent;
}