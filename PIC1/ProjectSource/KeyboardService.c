/****************************************************************************
 Module
   KeyboardService.c

 Revision
   1.0.1

 Description
   This is a Keyboard file for implementing a simple service under the
   Gen2 Events and Services Framework.

 Notes

 History
 When           Who     What/Why
 -------------- ---     --------
 01/16/12 09:58 jec      began conversion from KeyboardFSM.c
****************************************************************************/
/*----------------------------- Include Files -----------------------------*/
/* include header files for this state machine as well as any machines at the
   next lower level in the hierarchy that are sub-machines to this machine
*/
#include "ES_Configure.h"
#include "ES_Framework.h"
#include "KeyboardService.h"
#include "SPIFollowerService.h"

/*----------------------------- Module Defines ----------------------------*/

/*---------------------------- Module Functions ---------------------------*/
/* prototypes for private functions for this service.They should be functions
   relevant to the behavior of this service
*/

/*---------------------------- Module Variables ---------------------------*/
// with the introduction of Gen2, we need a module level Priority variable
static uint8_t MyPriority;

/*------------------------------ Module Code ------------------------------*/
/****************************************************************************
 Function
     InitKeyboardService

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
bool InitKeyboardService(uint8_t Priority)
{
  ES_Event_t ThisEvent;

  MyPriority = Priority;
  /********************************************
   in here you write your initialization code
   *******************************************/
  // post the initial transition event
  ThisEvent.EventType = ES_INIT;
  DB_printf("KeyboardService initialized\r\n");
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
     PostKeyboardService

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
bool PostKeyboardService(ES_Event_t ThisEvent)
{
  return ES_PostToService(MyPriority, ThisEvent);
}

/****************************************************************************
 Function
    RunKeyboardService

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
ES_Event_t RunKeyboardService(ES_Event_t ThisEvent)
{
  ES_Event_t ReturnEvent;
  ReturnEvent.EventType = ES_NO_EVENT; // assume no errors
  /********************************************
   in here you write your service code
   *******************************************/
  if (ThisEvent.EventType == ES_NEW_KEY)
  {
    ES_Event_t Event2Post;
    switch (ThisEvent.EventParam)
    {
    case 'a':
        /* code */
        break;
    case 'b':
        /* code */
        break;
    case 'c':
        Event2Post.EventType = ES_TAPE_FOLLOW;
        Event2Post.EventParam = 1;
        PostTapeFSM(Event2Post);
        DB_printf("ES_TAPE_FOLLOW posted to TapeFSM\r\n");
        break;
    case 'd':
        Event2Post.EventType = ES_TAPE_FOLLOW;
        Event2Post.EventParam = 0;
        PostTapeFSM(Event2Post);
        DB_printf("ES_TAPE_FOLLOW posted to TapeFSM\r\n");
        break;
    case 'e':
        Event2Post.EventType = ES_TAPE_STOP;  
        PostTapeFSM(Event2Post);
        DB_printf("ES_TAPE_STOP posted to TapeFSM\r\n");
        break;
    case 'f':
        /* code */
        break;
    case 'g':
        /* code */
        break;
    case 'h':
        /* code */
        break;
    case 'i':
        /* code */
        break;
    case 'j':
        /* code */
        break;
    case 'k':
        /* code */
        break;
    case 'l':
        /* code */
        break;
    case 'm':
        /* code */
        break;
    case 'n':
        /* code */
        break;
    case 'o':
        /* code */
        break;
    case 'p':
        /* code */
        break;
    case 'q':
        /* code */
        break;
    case 'r':
        /* code */
        break;
    case 's':
        /* code */
        break;
    case 't':
        /* code */
        break;
    case 'u':
        /* code */
        break;
    case 'v':
        /* code */
        break;
    case 'w':
        /* code */
        break;
    case 'x':
        /* code */
        break;
    case 'y':
        /* code */
        break;
    case 'z':
        /* code */
        break;
    case '0':
        /* code */
        break;
    case '1':
        /* code */
        break;
    case '2':
        /* code */
        break;
    case 'A':
        /* code */
        break;
    case 'B':
        /* code */
        break;
    case 'C':
        /* code */
        break;
    case 'D':
        /* code */
        break;
    case 'E':
        /* code */
        break;
    case 'F':
        /* code */
        break;
    case 'G':
        /* code */
        break;
    case 'H':
        /* code */
        break;
    case 'I':
        /* code */
        break;
    case 'J':
        /* code */
        break;
    case 'K':
        /* code */
        break;
    case 'L':
        /* code */
        break;
    case 'M':
        /* code */
        break;
    case 'N':
        /* code */
        break;
    case 'O':
        /* code */
        break;
    case 'P':
        /* code */
        break;
    case 'Q':
        /* code */
        break;
    case 'R':
        /* code */
        break;
    case 'S':
        /* code */
        break;
    case 'T':
        /* code */
        break;
    case 'U':
        /* code */
        break;
    case 'V':
        /* code */
        break;
    case 'W':
        /* code */
        break;
    case 'X':
        /* code */
        break;
    case 'Y':
        /* code */
        break;
    case 'Z':
        /* code */
        break;

    case '3':
        DB_printf("3 pressed\r\n");
        Event2Post.EventType = ES_NEW_NAV_CMD;
        Event2Post.EventParam = NAV_CMD_MOVE_FORWARD;
        PostNavigatorHSM(Event2Post);
        break;

    case '4':
        DB_printf("4 pressed\r\n");
        Event2Post.EventType = ES_NEW_NAV_CMD;
        Event2Post.EventParam = NAV_CMD_MOVE_BACKWARD;
        PostNavigatorHSM(Event2Post);
        break;
    
    case '5':
        DB_printf("5 pressed\r\n");
        Event2Post.EventType = ES_NEW_NAV_CMD;
        Event2Post.EventParam = NAV_CMD_TURN_LEFT;
        PostNavigatorHSM(Event2Post);
        break;

    case '6':
        DB_printf("6 pressed\r\n");
        Event2Post.EventType = ES_NEW_NAV_CMD;
        Event2Post.EventParam = NAV_CMD_TURN_RIGHT;
        PostNavigatorHSM(Event2Post);
        break;
    case '7':
        DB_printf("7 pressed\r\n");
        Event2Post.EventType = ES_CROSS_DETECTED;
        Event2Post.EventParam = NAV_CMD_STOP;
        PostNavigatorHSM(Event2Post);
        break;
    default:
        break;
    }
  }
  
  return ReturnEvent;
}

/***************************************************************************
 private functions
 ***************************************************************************/

/*------------------------------- Footnotes -------------------------------*/
/*------------------------------ End of file ------------------------------*/

