/****************************************************************************
 Module
   TapeFSM.c

 Revision
   1.0.1

 Description
   This is a Tape file for implementing flat state machines under the
   Gen2 Events and Services Framework.

 Notes

 History
 When           Who     What/Why
 -------------- ---     --------
 01/15/12 11:12 jec      revisions for Gen2 framework
 11/07/11 11:26 jec      made the queue static
 10/30/11 17:59 jec      fixed references to CurrentEvent in RunTapeSM()
 10/23/11 18:20 jec      began conversion from SMTape.c (02/20/07 rev)
****************************************************************************/
/*----------------------------- Include Files -----------------------------*/
/* include header files for this state machine as well as any machines at the
   next lower level in the hierarchy that are sub-machines to this machine
*/
#include "ES_Configure.h"
#include "ES_Framework.h"
#include "TapeFSM.h"

/*----------------------------- Module Defines ----------------------------*/

/*---------------------------- Module Functions ---------------------------*/
/* prototypes for private functions for this machine.They should be functions
   relevant to the behavior of this state machine
*/
static void exitFollowing(); //the exit function for exiting the Following state back to Idle
/*---------------------------- Module Variables ---------------------------*/
// everybody needs a state variable, you may need others as well.
// type of state variable should match htat of enum in header file

#define PWM_freq  10000 //wheel motor PWM frquency in Hz
#define PIC_freq 20000000
#define PIC_freq_kHz 20000


#define H_bridge1A_TRIS TRISBbits.TRISB11//output
#define H_bridge1A_LAT LATBbits.LATB11 //latch
#define H_bridge3A_TRIS TRISBbits.TRISB9//output
#define H_bridge3A_LAT LATBbits.LATB9 //latch
#define IC_TIMER_period 50000

//prescalars and priorities
#define prescalar_OC_M1 2 //for PWM for motor1
#define prescalar_OC_M2 2 
#define prescalar_T4 4 //used for running control law
#define priority_control 5

//output compare stuff (PWM)
static bool Dir1 = 0; //the direction of the motor, 0 = positive dir
static bool Dir2 = 0; 


//control stuff
#define Control_interval 2 //in ms
#define Ki 0.3
#define Kp 0.1
static K_error = 0;
volatile int K_error_sum = 0;
volatile int K_effort = 200;//in the unit of ticks for PR2
// framework stuff
static uint8_t MyPriority;
static TapeState_t CurrentState;
/*------------------------------ Module Code ------------------------------*/
/****************************************************************************
 Function
     InitTapeFSM

 Parameters
     uint8_t : the priorty of this service

 Returns
     bool, false if error in initialization, true otherwise

 Description
     Saves away the priority, sets up the initial transition and does any
     other required initialization for this state machine
 Notes

 Author
     J. Edward Carryer, 10/23/11, 18:55
****************************************************************************/
bool InitTapeFSM(uint8_t Priority)
{
  
  H_bridge1A_TRIS = 0; //Outputs
  H_bridge1A_LAT = 0;
  H_bridge3A_TRIS = 0; //Outputs
  H_bridge3A_LAT = 0;
  ES_Event_t ThisEvent;
  MyPriority = Priority;
  // put us into the Initial PseudoState
  CurrentState = Idle;
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
     PostTapeFSM

 Parameters
     EF_Event_t ThisEvent , the event to post to the queue

 Returns
     boolean False if the Enqueue operation failed, True otherwise

 Description
     Posts an event to this state machine's queue
 Notes

 Author
     J. Edward Carryer, 10/23/11, 19:25
****************************************************************************/
bool PostTapeFSM(ES_Event_t ThisEvent)
{
  return ES_PostToService(MyPriority, ThisEvent);
}

/****************************************************************************
 Function
    RunTapeFSM

 Parameters
   ES_Event_t : the event to process

 Returns
   ES_Event_t, ES_NO_EVENT if no error ES_ERROR otherwise

 Description
   add your description here
 Notes
   uses nested switch/case to implement the machine.
 Author
   J. Edward Carryer, 01/15/12, 15:23
****************************************************************************/
ES_Event_t RunTapeFSM(ES_Event_t ThisEvent)
{
  ES_Event_t ReturnEvent;
  ReturnEvent.EventType = ES_NO_EVENT; // assume no errors

  switch (CurrentState)
  {
    case Idle:        
    {

    }
    break;

    case Following:        // If current state is state one
    {

    }
    break;
    default:
    break;
  }                                   // end switch on Current State
  return ReturnEvent;
}

/****************************************************************************
 Function
     QueryTapeSM

 Parameters
     None

 Returns
     TapeState_t The current state of the Tape state machine

 Description
     returns the current state of the Tape state machine
 Notes

 Author
     J. Edward Carryer, 10/23/11, 19:21
****************************************************************************/
TapeState_t QueryTapeFSM(void)
{
  return CurrentState;
}

/***************************************************************************
 private functions
 ***************************************************************************/
void exitFollowing(){
  return;
}
