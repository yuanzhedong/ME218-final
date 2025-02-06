/****************************************************************************
 Module
   lab8_mainFSM.c

 Revision
   1.0.1

 Description
   This is a lab8_main file for implementing flat state machines under the
   Gen2 Events and Services Framework.

 Notes

 History
 When           Who     What/Why
 -------------- ---     --------
 01/15/12 11:12 jec      revisions for Gen2 framework
 11/07/11 11:26 jec      made the queue static
 10/30/11 17:59 jec      fixed references to CurrentEvent in Runlab8_mainSM()
 10/23/11 18:20 jec      began conversion from SMlab8_main.c (02/20/07 rev)
****************************************************************************/
/*----------------------------- Include Files -----------------------------*/
/* include header files for this state machine as well as any machines at the
   next lower level in the hierarchy that are sub-machines to this machine
*/
#include "ES_Configure.h"
#include "ES_Framework.h"
#include "lab8_mainFSM.h"
#include "dbprintf.h"
/*----------------------------- Module Defines ----------------------------*/

/*---------------------------- Module Functions ---------------------------*/
/* prototypes for private functions for this machine.They should be functions
   relevant to the behavior of this state machine
*/

/*---------------------------- Module Variables ---------------------------*/
// everybody needs a state variable, you may need others as well.
// type of state variable should match htat of enum in header file
#define TapeSenorReading PORTAbits.RA2
#define TapeSensorTris TRISAbits.TRISA2 
static uint16_t ScanInterval = 30; //time between each AD read
static uint32_t CurrADVal[1];
static uint32_t PrevADVal[1];
static uint8_t numOfIncr = 0; //number of successive increases between AD readings
static uint8_t numOfDecr = 0; //number of successive decreases between AD readings
static bool increasingOrNo = false;
static lab8_mainState_t CurrentState;
static lab8_mainState_t  NextState;
// with the introduction of Gen2, we need a module level Priority var as well
static uint8_t MyPriority;

/*------------------------------ Module Code ------------------------------*/
/****************************************************************************
 Function
     Initlab8_mainFSM

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
bool Initlab8_mainFSM(uint8_t Priority)
{
  ES_Event_t ThisEvent;

  MyPriority = Priority;
  // put us into the Initial PseudoState
  CurrentState = StopMotion;
  BeaconSensorTris = 1;
  ADC_ConfigAutoScan(BIT12HI);//bit5 corresponds to AN12/PMD0/RB12
  TRISBbits.TRISB12 = 1; // configure the pin as input
  ANSELBbits.ANSB12 = 1; // Configure RB15 as analog IO

  
  DB_printf("AD service initialized \n");

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
     Postlab8_mainFSM

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
bool Postlab8_mainFSM(ES_Event_t ThisEvent)
{
  return ES_PostToService(MyPriority, ThisEvent);
}

/****************************************************************************
 Function
    Runlab8_mainFSM

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
ES_Event_t Runlab8_mainFSM(ES_Event_t ThisEvent)
{
  ES_Event_t ReturnEvent;
  ReturnEvent.EventType = ES_NO_EVENT; // assume no errors
  NextState = CurrentState;
  ES_Event_t Event2Post;
  switch (CurrentState)
  {
    case StopMotion:        // If current state is initial Psedudo State
      Event2Post.EventType = ES_StopMotor;
      PostPWMService(Event2Post);
      switch (ThisEvent.EventType)
      {
        case ES_ALIGN:  
        NextState = BeaconAlign;
        ES_Timer_InitTimer(ADTimer,ScanInterval);
        break;

        // repeat cases as required for relevant events
        default:
        break;
      }  // end switch on CurrentEvent
      
    break;

    case NormalMove:        // If current state is state one

    break;
    case BeaconAlign:
        if (ThisEvent.EventType==ES_TIMEOUT &&ThisEvent.EventParam == ADTimer)
        {
          ES_Timer_InitTimer(ADTimer,ScanInterval);
          //read the potentiometer
          ADC_MultiRead(CurrADVal);
          if (CurrADVal[0]>PrevADVal[0])
          {
            numOfIncr++;
          }
          PrevADVal[0] = CurrADVal[0];
        }
        

        
    break;
   
    default:
    break;
  }   // end switch on Current State  
  CurrentState = NextState;     
  return ReturnEvent;
}

/****************************************************************************
 Function
     Querylab8_mainSM

 Parameters
     None

 Returns
     lab8_mainState_t The current state of the lab8_main state machine

 Description
     returns the current state of the lab8_main state machine
 Notes

 Author
     J. Edward Carryer, 10/23/11, 19:21
****************************************************************************/
lab8_mainState_t Querylab8_mainFSM(void)
{
  return CurrentState;
}

/***************************************************************************
 private functions
 ***************************************************************************/

