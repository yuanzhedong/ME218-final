/****************************************************************************
 Module
   TemplateService.c

 Revision
   1.0.1

 Description
   This is a template file for implementing a simple service under the
   Gen2 Events and Services Framework.

 Notes

 History
 When           Who     What/Why
 -------------- ---     --------
 01/16/12 09:58 jec      began conversion from TemplateFSM.c
****************************************************************************/
/*----------------------------- Include Files -----------------------------*/
/* include header files for this state machine as well as any machines at the
   next lower level in the hierarchy that are sub-machines to this machine
*/
#include "ES_Configure.h"
#include "ES_Framework.h"
#include "Lab8_SM.h"
#include "MotorService.h"
#include "dbprintf.h"

/*----------------------------- Module Defines ----------------------------*/

/*---------------------------- Module Functions ---------------------------*/
/* prototypes for private functions for this service.They should be functions
   relevant to the behavior of this service
*/

/*---------------------------- Module Variables ---------------------------*/
// with the introduction of Gen2, we need a module level Priority variable
static uint8_t MyPriority;

static Lab8State_t state;

/*------------------------------ Module Code ------------------------------*/
/****************************************************************************
 Function
     InitTemplateService

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
bool InitLab8_SM(uint8_t Priority)
{
  ES_Event_t ThisEvent;

  MyPriority = Priority;
  /********************************************
   in here you write your initialization code
   *******************************************/
  clrScrn();
  InitOpto();
  state = Generating;
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
     PostTemplateService

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
bool PostLab8_SM(ES_Event_t ThisEvent)
{
  return ES_PostToService(MyPriority, ThisEvent);
}

/****************************************************************************
 Function
    RunTemplateService

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
ES_Event_t RunLab8_SM(ES_Event_t ThisEvent)
{
  ES_Event_t ReturnEvent;
  ReturnEvent.EventType = ES_NO_EVENT; // assume no errors
  /********************************************
   in here you write your service code
   *******************************************/
  switch(state)
  {
      case Generating:
          if(ES_GEN == ThisEvent.EventType){
//              DB_printf("Inside ES_GEN\n");
              switch(ThisEvent.EventParam)
              {
                  case STOP_CMD:
                      MotorCommand(M_STOP);
                      DB_printf("Command: 0x%x\n",ThisEvent.EventParam);
                      DB_printf("Stopping motors\n");
                      DB_printf("\n");
                      break;
                  case CW_90_CMD:
                      MotorCommand(M_CW_90);
                      DB_printf("Command: 0x%x\n",ThisEvent.EventParam);
                      DB_printf("Turning CW 90 degrees\n");
                      DB_printf("\n");
                      break;
                  case CW_45_CMD:
                      MotorCommand(M_CW_45);
                      DB_printf("Command: 0x%x\n",ThisEvent.EventParam);
                      DB_printf("Turning CW 45 degrees\n");
                      DB_printf("\n");
                      break;
                  case CCW_90_CMD:
                      MotorCommand(M_CCW_90);
                      DB_printf("Command: 0x%x\n",ThisEvent.EventParam);
                      DB_printf("Turning CCW 90 degrees\n");
                      DB_printf("\n");
                      break;
                  case CCW_45_CMD:
                      MotorCommand(M_CCW_45);
                      DB_printf("Command: 0x%x\n",ThisEvent.EventParam);
                      DB_printf("Turning CCW 45 degrees\n");
                      DB_printf("\n");
                      break;
                  case FWD_FULL_CMD:
                      MotorCommand(M_FWD_FS);
                      DB_printf("Command: 0x%x\n",ThisEvent.EventParam);
                      DB_printf("Driving forwards at full speed\n");
                      DB_printf("\n");
                      break;
                  case FWD_HALF_CMD:
                      MotorCommand(M_FWD_HS);
                      DB_printf("Command: 0x%x\n",ThisEvent.EventParam);
                      DB_printf("Driving forwards at half speed\n");
                      DB_printf("\n");
                      break;
                  case BWD_FULL_CMD:
                      MotorCommand(M_BWD_FS);
                      DB_printf("Command: 0x%x\n",ThisEvent.EventParam);
                      DB_printf("Driving backwards at full speed\n");
                      DB_printf("\n");
                      break;
                  case BWD_HALF_CMD:
                      MotorCommand(M_BWD_HS);
                      DB_printf("Command: 0x%x\n",ThisEvent.EventParam);
                      DB_printf("Driving backwards at half speed\n");
                      DB_printf("\n");
                      break;
                  case ALIGN_CMD:
                      MotorCommand(M_SPIN);
                      state = Aligning;
                      ES_Event_t BeginLooking4Beacon;
                      BeginLooking4Beacon.EventType = ES_LOOKING_4_BEACON;
                      PostBeaconFSM(BeginLooking4Beacon);
                      DB_printf("Command: 0x%x\n",ThisEvent.EventParam);
                      DB_printf("Aligning with beacon\n");
                      DB_printf("\n");
                      break;
                  default:
                      break;
              }
          }
          break;
      case Aligning:
          if(ES_BEACON == ThisEvent.EventType){
              MotorCommand(M_STOP);
              state = Aligned;
              DB_printf("Beacon found.  Stopping motors\n");
              DB_printf("\n");
          }
          break;
      case Aligned:
          // Note: FIND_TAPE_CMD occurs before ES_BEACON, we will be stuck in the Aligned state!!!!!!!!!!!!!!!!!!!!
          // One possible fix is to have FIND_TAPE_CMD be the only command that is sent repeatedly
          if(ES_GEN == ThisEvent.EventType){
              if(FIND_TAPE_CMD == ThisEvent.EventParam){
                  MotorCommand(M_FWD_FS); // do we want full speed or half speed???????????????????????????????????
                  state = Approaching;
                  DB_printf("Command: 0x%x\n",ThisEvent.EventParam);
                  DB_printf("Driving towards tape\n");
                  DB_printf("\n");
                  
                  ES_Event_t DummyEvent4EventChecker;
                  DummyEvent4EventChecker.EventType = ES_BEACON;
                  PostLab8_SM(DummyEvent4EventChecker);
              }
             else{
                  state = Generating;
                  // I make an event here because I want to go back to the Generating state
                  // and pass in the command so that way the motors know what to do
                  ES_Event_t GenEvent;
                  GenEvent.EventType = ES_GEN;
                  GenEvent.EventParam = ThisEvent.EventParam;
                  PostLab8_SM(GenEvent);
              }
          }
          break;
      case Approaching:
          if(ES_TAPE == ThisEvent.EventType){
              MotorCommand(M_STOP);
              state = Done;
              DB_printf("Tape detected.  Motors stopping\n");
              DB_printf("\n");
          }
          break;
      case Done:
          break;
      default:
          break;
  }
//  DB_printf("ThisEvent.EventType = %u\n", ThisEvent.EventType);
  return ReturnEvent;
}

Lab8State_t QueryLab8_SM(void)
{
  return state;
}

/***************************************************************************
 private functions
 ***************************************************************************/

/*------------------------------- Footnotes -------------------------------*/
/*------------------------------ End of file ------------------------------*/

