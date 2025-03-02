/****************************************************************************
 Module
   StepperService.c

 Revision
   1.0.1

 Description
   This is a Stepper file for implementing a simple service under the
   Gen2 Events and Services Framework.

 Notes

 History
 When           Who     What/Why
 -------------- ---     --------
 01/16/12 09:58 jec      began conversion from StepperFSM.c
****************************************************************************/
/*----------------------------- Include Files -----------------------------*/
/* include header files for this state machine as well as any machines at the
   next lower level in the hierarchy that are sub-machines to this machine
*/
#include "ES_Configure.h"
#include "ES_Framework.h"
#include "StepperService.h"
#include "dbprintf.h"
#include <sys/attribs.h>
#include "terminal.h"

/*----------------------------- Module Defines ----------------------------*/

/*---------------------------- Module Functions ---------------------------*/
/* prototypes for private functions for this service.They should be functions
   relevant to the behavior of this service
*/

/*---------------------------- Module Variables ---------------------------*/
// with the introduction of Gen2, we need a module level Priority variable
#define ticks_per_us 2.5
#define H_bridge1A_LAT  LATBbits.LATB9
#define H_bridge2A_LAT  LATBbits.LATB10
#define H_bridge3A_LAT  LATBbits.LATB11
#define H_bridge4A_LAT  LATBbits.LATB12
static int stepInd = 0;
static int RowNum;//number of rows in the table chosen
static bool Dir = 0; //0 means forward and 1 means backwards
static uint16_t speedInHz = 50;//set motor speed in Hz
static uint16_t maxSpeed = 150; //max motor speed in Hz
static uint16_t minSpeed = 3;
static uint16_t stepsCommanded;
static uint16_t stepsCompleted;
static uint16_t StepInterval = 1; //steps per sec = 1000 / StepInterval
static bool (*tableChosen)[4];
static bool table_FullStep[4][4] = {
    {1, 0, 1, 0},
    {1, 0, 0, 1},
    {0, 1, 0, 1},
    {0, 1, 1, 0}
};
static bool table_Wave[4][4] = {
    {1, 0, 0, 0},
    {0, 0, 0, 1},
    {0, 1, 0, 0},
    {0, 0, 1, 0}
};
static bool table_HalfStep[8][4] = {
    {1, 0, 1, 0},
    {1, 0, 0, 0},
    {1, 0, 0, 1},
    {0, 0, 0, 1},
    {0, 1, 0, 1},
    {0, 1, 0, 0},
    {0, 1, 1, 0},
    {0, 0, 1, 0}
};
static uint8_t MyPriority;

/*------------------------------ Module Code ------------------------------*/
/****************************************************************************
 Function
     InitStepperService

 Parameters
     uint8_t : the priorty of this service

 Returns
     bool, false if error in initialization, true otherwise

 Description
     Saves away the priority, and does any
     other required initialization for this service
 Notes

 Author
     J. Edward Carryer, 01/16/12, 10:00
****************************************************************************/
bool InitStepperService(uint8_t Priority)
{
  ES_Event_t ThisEvent;

  MyPriority = Priority;
  /********************************************
   in here you write your initialization code
   *******************************************/
  //Configure the pins for the H bridge
  TRISBbits.TRISB9 = 0;
  TRISBbits.TRISB10 = 0;
  TRISBbits.TRISB11 = 0;
  TRISBbits.TRISB12 = 0;
  ANSELBbits.ANSB12 = 0;
  H_bridge1A_LAT = 0;//stepper phase A+ RB9
  H_bridge2A_LAT = 0;
  H_bridge3A_LAT = 0;
  H_bridge4A_LAT = 0;
  tableChosen = table_FullStep;
  stepInd = 0;
  RowNum = 4;
  DB_printf("[STEP MOTER] half step mode chosen\r\n");
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
     PostStepperService

 Parameters
     EF_Event_t ThisEvent ,the event to post to the queue

 Returns
     bool false if the Enqueue operation failed, true otherwise

 Description
     Posts an event to this state machine's queue
 Notes

 Author
     J. Edward Carryer, 10/23/11, 19:25
****************************************************************************/
bool PostStepperService(ES_Event_t ThisEvent)
{
  return ES_PostToService(MyPriority, ThisEvent);
}

/****************************************************************************
 Function
    RunStepperService

 Parameters
   ES_Event_t : the event to process

 Returns
   ES_Event, ES_NO_EVENT if no error ES_ERROR otherwise

 Description
   add your description here
 Notes

 Author
   J. Edward Carryer, 01/15/12, 15:23
****************************************************************************/
ES_Event_t RunStepperService(ES_Event_t ThisEvent)
{
  ES_Event_t ReturnEvent;
  ReturnEvent.EventType = ES_NO_EVENT; // assume no errors
  switch (ThisEvent.EventType)
  {
  case ES_TIMEOUT:
    if (stepsCompleted >= stepsCommanded)
    {
      H_bridge1A_LAT = 0;
      H_bridge2A_LAT = 0;
      H_bridge3A_LAT = 0;
      H_bridge4A_LAT = 0;
      ES_Event_t Event2Post;
      Event2Post.EventType = ES_STEPPER_COMPLETE;
      PostPlannerHSM(Event2Post);
      DB_printf("steps completed is %d\n", stepsCompleted);
    }else{
      //step1: step a step
      DB_printf("%d ",tableChosen[stepInd][0]);
      DB_printf("%d ",tableChosen[stepInd][1]);
      DB_printf("%d ",tableChosen[stepInd][2]);
      DB_printf("%d \n",tableChosen[stepInd][3]);
      H_bridge1A_LAT = tableChosen[stepInd][0];
      H_bridge2A_LAT = tableChosen[stepInd][1];
      H_bridge3A_LAT = tableChosen[stepInd][2];
      H_bridge4A_LAT = tableChosen[stepInd][3];
      if (Dir == 0)
      {
        if (stepInd >= RowNum - 1)
        {
          stepInd = 0;
        }else
        {
          stepInd++;
        }
      }else
      {
        if (stepInd <= 0)
        {
          stepInd = RowNum - 1;
        }else
        {
          stepInd--;
        }      
      }
      stepsCompleted++;
      //step2: reset the timer for the next step
      ES_Timer_InitTimer(Stepper_TIMER, StepInterval);
    }
  break;

  case ES_STEPPER_FWD:
    //stepInd = 0;
    Dir = 0;
    stepsCommanded = ThisEvent.EventParam;
    stepsCompleted = 0;
    ES_Timer_InitTimer(Stepper_TIMER, StepInterval);
    DB_printf("steps commanded is %d\n", stepsCommanded);
    break;
  case ES_STEPPER_BWD:
    //stepInd = 0;
    Dir = 1;
    stepsCommanded = ThisEvent.EventParam;
    stepsCompleted = 0;
    ES_Timer_InitTimer(Stepper_TIMER, StepInterval);
    DB_printf("steps commanded is %d\n", stepsCommanded);
    break;
  default:
    break;
  }
  return ReturnEvent;
}

/***************************************************************************
 private functions
 ***************************************************************************/

/*------------------------------- Footnotes -------------------------------*/
/*------------------------------ End of file ------------------------------*/

