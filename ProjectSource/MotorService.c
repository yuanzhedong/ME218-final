
/****************************************************************************
 Module
   MotorService.c

****************************************************************************/
/*----------------------------- Include Files -----------------------------*/
/* include header files for this state machine as well as any machines at the
   next lower level in the hierarchy that are sub-machines to this machine
*/
#include "ES_Configure.h"
#include "ES_Framework.h"
#include "ES_Events.h"
#include "MotorService.h" // header file for this module
#include "dbprintf.h"
#include "ES_Port.h"

/*----------------------------- Module Defines ----------------------------*/
 #define TEST // for debugging

/*---------------------------- Module Functions ---------------------------*/
/* prototypes for private functions for this service.They should be functions
   relevant to the behavior of this service
*/

/*---------------------------- Module Variables ---------------------------*/
// with the introduction of Gen2, we need a module level Priority variable
static uint8_t MyPriority;

// for writing number of ticks
static uint16_t TimerPeriod = 49999;

// array structure goes WhichMove [OC2RS, OC2 polarity, OC3RS, OC3 polarity, time]
// 2 is right, 3 is left

static uint16_t MotorSettings[10][5] = 
{
    {0, 0, 0, 0, 0}, // setting 0: stop
    {45000, 0, 49000, 0, 0}, // setting 1: forward full speed
    {22500, 0, 25000, 0, 0}, // setting 2: forward half speed
    {49000, 1, 47000, 1, 0}, // setting 3: reverse full speed
    {25000, 1, 23500, 1, 0}, // setting 4: reverse half speed
    {49000, 1, 49000, 0, 1600}, // setting 5 clockwise 90 deg
    {45000, 0, 47000, 1, 1600}, // setting 6 counter-clockwise 90 deg
    {49000, 1, 49000, 0, 800}, // setting 7 clockwise 45 deg
    {45000, 0, 47000, 1, 800}, // setting 8 counter-clockwise 45 deg
    {49000, 1, 49000, 0, 0} // setting 9 unbounded rotation
    
};

// direction tracking
static bool isFwd = true;


/*------------------------------ Module Code ------------------------------*/
/****************************************************************************
 Function
     InitMotorService

****************************************************************************/
bool InitMotorService(uint8_t Priority)
{
    #ifdef TEST
        DB_printf(" init motor service \r\n"); // debug printing
    #endif
    
  ES_Event_t ThisEvent; // variable event to return
  MyPriority = Priority; // save priority variable
    
    // PWM pins configure as outputs
    TRISBbits.TRISB8 = 0; // PWM OC2
    TRISBbits.TRISB9 = 0; // PWM OC3
    TRISBbits.TRISB10 = 0; // polarity OC2
    TRISBbits.TRISB12 = 0; // polarity OC3
    
    // no analog on capable pins
    ANSELBbits.ANSB12 = 0;
    
    // configure timer 2 (use for both PWM channels))
    T2CONbits.ON = 0; // turn timer off
    T2CONbits.TCS = 0; // internal clock source
    T2CONbits.TCKPS = 1; // 1 for prescale of 2, 0 for 1
    T2CONbits.TGATE = 0; // no external gate
    PR2 = TimerPeriod; // period setting (starting freq 200Hz)
    TMR2 = 0; // start timer at zero
    IFS0CLR = _IFS0_T2IF_MASK; // clear any flag
    T2CONbits.ON = 1; // turn timer on

    // configure output channel (channel 3, RPB9)
    OC3CONbits.ON = 0; // turn off output compare
    OC3CONbits.OCM = 0b110; // PWM without fault mode
    OC3CONbits.OCTSEL = 0; // connect timer 2
    OC3CONbits.SIDL = 0; // continue in idle
    OC3CONbits.OC32 = 0; //32-bit mode off
    RPB9R = 0b0101; // map channel to the correct pin
    OC3CONbits.ON = 1; // turn on output compare
    OC3RS = 0; // preliminary duty cycle zero
    OC3R = 0; // preliminary duty cycle zero
    
    // configure output channel (channel 2, RPB8)
    OC2CONbits.ON = 0; // turn off output compare
    OC2CONbits.OCM = 0b110; // PWM without fault mode
    OC2CONbits.OCTSEL = 0; // connect timer 2
    OC2CONbits.SIDL = 0; // continue in idle
    OC2CONbits.OC32 = 0; //32-bit mode off
    RPB8R = 0b0101; // map channel to the correct pin
    OC2CONbits.ON = 1; // turn on output compare
    OC2RS = 0; // preliminary duty cycle zero
    OC2R = 0; // preliminary duty cycle zero
    
    // timer to trigger the run function
    // ES_Timer_InitTimer(TEST_TIMER, 100);

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
     PostMotorService

 Parameters
     EF_Event_t ThisEvent ,the event to post to the queue

 Returns
     bool false if the Enqueue operation failed, true otherwise

 Description
     Posts an event to this state machine's queue

****************************************************************************/
bool PostMotorService(ES_Event_t ThisEvent)
{
  return ES_PostToService(MyPriority, ThisEvent);
}

/****************************************************************************
 Function
    RunMotorService

 Parameters
   ES_Event_t : the event to process

 Returns
   ES_Event, ES_NO_EVENT if no error ES_ERROR otherwise

 Description
   add your description here

****************************************************************************/
ES_Event_t RunMotorService(ES_Event_t ThisEvent)
{
  ES_Event_t ReturnEvent; // prepare event to return to the framework
  ReturnEvent.EventType = ES_NO_EVENT; // assume no errors

   if (ThisEvent.EventType == ES_TIMEOUT) // if it's a timeout
  {
//    if (ThisEvent.EventParam == TEST_TIMER) // if it's the encoder timer's timeout
//    {
//        MotorCommand(M_CW_45); // send the motor command
//    } // endif test timer
    
    if (ThisEvent.EventParam == MOTOR_TIMER) // if motor timer done
    {
//        if (isFwd)
//        {
//            MotorCommand(M_CCW_45); // send the motor command
//            isFwd = false;
//        }
//        else
//        {
//            MotorCommand(M_CW_45); // send the motor command
//            isFwd = true;
//        }
          
          MotorCommand(M_STOP); // send the motor command
        
    } // endif motor timer
    
   } // endif timeout event
    
  return ReturnEvent; // return event for the framework
}

/***************************************************************************
 private functions
 ***************************************************************************/
// helper function for sending commands to the motor
void MotorCommand(uint16_t WhichMove)
{
    // array structure goes WhichMove [OC2RS, OC2 polarity, OC3RS, OC3 polarity, time]
    
    // start at no speed
    OC2RS = 0;
    OC3RS = 0;
    
    // send commands to left side motor
    if (MotorSettings[WhichMove][3] == 0) // if going clockwise
    {
        OC3RS = MotorSettings[WhichMove][2]; // set pwm normally
        LATBbits.LATB12 = 0; // set polarity pin
    }
    else
    {
        OC3RS = (TimerPeriod - MotorSettings[WhichMove][2]); // inverse pwm
        LATBbits.LATB12 = 1; // set polarity pin
    }
    
    
    // send PWM commands to right side motor
    if (MotorSettings[WhichMove][1] == 0) // if going clockwise
    {
        OC2RS = (TimerPeriod - MotorSettings[WhichMove][0]); // inverse pwm
        LATBbits.LATB10 = 1; // set polarity pin
    }
    else
    {
        OC2RS = MotorSettings[WhichMove][0]; // set pwm normally
        LATBbits.LATB10 = 0; // set polarity pin
    }
    
    // if we need to start a timer
    if (MotorSettings[WhichMove][4] != 0)
    {
        uint16_t time = MotorSettings[WhichMove][4];
        ES_Timer_InitTimer(MOTOR_TIMER, time);
    }
    
}


/*------------------------------- Footnotes -------------------------------*/


/*------------------------------ End of file ------------------------------*/




