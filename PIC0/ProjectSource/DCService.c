#include "ES_Configure.h"
#include "ES_Framework.h"
#include "DCService.h"
#include <xc.h>
//#include "PWM_PIC32.h"
//#include "PWMSetup.h"

/*----------------------------- Module Defines ----------------------------*/
#define ONE_SEC 1000 // 1000 ms
static uint16_t steps_per_second = 400;
static uint16_t interval = 5;

/*---------------------------- Module Variables ---------------------------*/
static uint8_t MyPriority;
static uint16_t currentADC; 
uint16_t currentDutyCycle = 90; 
static bool forward = true;    // Default duty cycle (50%)
static uint16_t max_steps = 1000;
static uint16_t currentStep = 0;
static uint16_t TotalStep = 0;
//static bool MotorDirection = 1;
#define MotorDirection1 PORTAbits.RA2
#define MotorDirection2 PORTAbits.RA3
typedef enum{
    CW = 0,
    CCW
}MotorDirection_t;
static MotorDirection_t Direction;
#define DC_1 LATAbits.LATA1
#define DC_2 LATAbits.LATA2
//#define DIR_VAL PORTAbits.RA3
#define DIR_VAL 0


#define PWM_freq  10000 //wheel motor PWM frquency in Hz
#define PIC_freq 20000000
#define PIC_freq_kHz 20000
#define ns_per_tick 50 //nano-sec per tick for the PBC = 1/PIC_freq
#define prescalar_T2 2 //for PWM for the 2 motors
#define DUTY_CYCLE 60

/*---------------------------- Module Functions ---------------------------*/
//static void SetMotorDirection(bool direction);

static void ConfigPWM_OC2() {

    // map OC2 to RA1
    RPA1R = 0b0101;

    //Clear OC2CON register: 
    OC2CON = 0;

    // Configure the Output Compare module for one of two PWM operation modes
    OC2CONbits.ON = 0;         // Turn off Output Compare module
    OC2CONbits.OCM = 0b110;    // PWM mode without fault pin
    OC2CONbits.OCTSEL = 0;     // Use Timer2 as the time base

    // Set the PWM duty cycle by writing to the OCxRS register
    OC2RS = PR2 * 0;       // Secondary Compare Register (for duty cycle)
    OC2R = PR2 * 0;        // Primary Compare Register (initial value)
    //OC2R = (PR2 + 1) * 60 / 100;
    //OC2RS = (PR2 + 1) * 60 / 100;

    // Turn on Timer 2 after OC2 setup
//    T2CONbits.ON = 1;
    // Turn ON the Output Compare module
    OC2CONbits.ON = 1;         // Enable Output Compare module

    return;
}

/*------------------------------ Module Code ------------------------------*/

/****************************************************************************
 Function
     InitDCMotorService

 Parameters
     uint8_t : the priority of this service

 Returns
     bool, false if error in initialization, true otherwise

 Description
     Initializes the DC motor service, sets up PWM, and configures the motor.
****************************************************************************/
bool InitDCMotorService(uint8_t Priority)
{
    MyPriority = Priority;

    // Initialize motor control pins as digital outputs
    TRISAbits.TRISA0 = 0; // RA0 as output
    TRISAbits.TRISA2 = 0; // RA1 as output
    TRISAbits.TRISA3 = 1; // RA3 as input
    
    ANSELAbits.ANSA0 = 0; // Disable analog on RA0

    // Post the initial ES_INIT event
    ES_Event_t ThisEvent;
    ThisEvent.EventType = ES_INIT;
    
    //ConfigTimer2();
    ConfigPWM_OC2();
    //OC2RS = (PR2 + 1) * 90 / 100;
    //OC2RS = PR2/2;
//    DB_printf("curent PR2 %d\n", PR2);
    //SetPWMDutyCycle(80); // Set to 50% duty cycle
    Direction = (DIR_VAL == 1) ? CW : CCW;
//    OC2RS = (PR2 + 1) * DUTY_CYCLE / 100;
//    LATAbits.LATA0 = 0;
//    
    if (DIR_VAL == 1){
//      OC1RS = 0;
      OC2RS = (PR2 + 1) * DUTY_CYCLE / 100;
      LATAbits.LATA0 = 0;
      
    } else {
      LATAbits.LATA0 = 1;
      OC2RS = (PR2 + 1) * (100-DUTY_CYCLE) / 100;
    } 
    return ES_PostToService(MyPriority, ThisEvent);
}

/****************************************************************************
 Function
     PostDCMotorService

 Parameters
     ES_Event ThisEvent : the event to post

 Returns
     bool, true if post succeeds, false otherwise
****************************************************************************/
bool PostDCMotorService(ES_Event_t ThisEvent)
{
    return ES_PostToService(MyPriority, ThisEvent);
}

/****************************************************************************
 Function
     RunDCMotorService

 Parameters
     ES_Event : the event to process

 Returns
     ES_Event, ES_NO_EVENT if no error
****************************************************************************/
ES_Event_t RunDCMotorService(ES_Event_t ThisEvent)
{
    ES_Event_t ReturnEvent;
    ReturnEvent.EventType = ES_NO_EVENT; // Assume no errors

    switch (ThisEvent.EventType)
    {
    case ES_INIT:
        // Initialize step timer for motor control
        
        ES_Timer_InitTimer(DC_MOTOR_TIMER, interval);
        DB_printf("curent OC1RS %d\n", OC1RS);
        
        break;

    // case ES_TIMEOUT:
    //     // Print the current OC1RS value for debugging
    //     //DB_printf("Current OC1RS: %d\n", OC1RS);
    //     // Restart the timer to keep checking periodically
    //     //DB_printf("curent OC1RS %d\n", OC1RS);
    //     if (Direction == CW && DIR_VAL == 0) {
    //         DC_1 = 0;
    //         DC_2 = 1;
    //         Direction = CCW;
    //     } else if (Direction == CCW && DIR_VAL == 1) {
    //         DC_1 = 1;
    //         DC_2 = 0;
    //         Direction = CW;
    //     }

    //     ES_Timer_InitTimer(DC_MOTOR_TIMER, interval);
    //     break;
    
    case ES_NEW_KEY:
        DB_printf('direction reversed!!!');
        if (ThisEvent.EventParam == 'r'){
            DB_printf('direction reversed!!!');
            MotorDirection1 ^= 1;
            MotorDirection2 ^= 1;
        }
    break;

//    case ES_ADC_UPDATE:
//        // Update motor speed based on event parameter (0-100%)
//        currentADC = ThisEvent.EventParam;
//        currentDutyCycle = currentADC*100/1024;
//        SetPWMDutyCycle(currentDutyCycle);
//        DB_printf("Current DutyCycle: %d\n", currentDutyCycle);
//        DB_printf("curent OC1RS %d\n", OC1RS);
//        
//        break;
//
//    case ES_CHANGE_DIRECTION:
//        // Change motor direction
//        forward = !forward;
//        SetMotorDirection(forward);
//        break;
//
    default:
        break;
    }

    return ReturnEvent;
}
