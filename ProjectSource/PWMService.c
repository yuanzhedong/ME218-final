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

// Function to initialize the PWM Service
bool InitPWMService(uint8_t Priority) {
    MyPriority = Priority;
    // Initialization code for PWM hardware
    // ...
    // Example initialization code for PWM hardware using TMR and OC
    // Configure Timer and Output Compare for PWM

    // Assuming Timer 2 and Output Compare 1 for PWM
    T2CONbits.TCKPS = 0b011; // Set prescaler to 1:8
    PR2 = 1999;              // Set period register for 1 kHz PWM frequency
    TMR2 = 0;                // Clear timer register

    OC4CON = 0x0000;         // Clear OC4CON register
    OC4CONbits.OCTSEL = 0;   // Select Timer 2 as clock source
    OC4CONbits.OCM = 0b110;  // Set Output Compare mode to PWM
    OC4R = 1000;             // Set initial duty cycle to 50%
    OC4RS = 1000;            // Set secondary compare register

    // Configure RA2 as output for OC1
    TRISAbits.TRISA2 = 0;    // Set RA2 as output
    RPA2R = 0b0101;          // Map OC4 to RA2

    // Start Timer 2
    T2CONbits.ON = 1;
    // Start Output Compare 1
    OC4CONbits.ON = 1;
    puts("PWM Service initialized.\r\n");
    return true;
    
}

// Function to post an event to the PWM Service
bool PostPWMService(ES_Event_t ThisEvent) {
    return ES_PostToService(MyPriority, ThisEvent);
}

// Function to run the PWM Service
ES_Event_t RunPWMService(ES_Event_t ThisEvent) {
    ES_Event_t ReturnEvent;
    ReturnEvent.EventType = ES_NO_EVENT; // Assume no errors

    switch (ThisEvent.EventType) {
        case ES_INIT:
            // Initialization code for the service
            // ...
            break;

        case ES_TIMEOUT:
            // Handle timeout events
            // ...
            break;

        // Add other event types here

        default:
            break;
    }

    return ReturnEvent;
}