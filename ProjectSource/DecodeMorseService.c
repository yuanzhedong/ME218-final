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
#include "DecodeMorseService.h"
#include "LEDService.h"
#include <string.h>
#include "dbprintf.h"


/*----------------------------- Module Defines ----------------------------*/

/*---------------------------- Module Functions ---------------------------*/
/* prototypes for private functions for this service.They should be functions
   relevant to the behavior of this service
*/

/*---------------------------- Module Variables ---------------------------*/
// with the introduction of Gen2, we need a module level Priority variable
static uint8_t MyPriority;
static char morseChars[8];
static char ASCIIChars[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890?.,:'-/()\"=!$&+;@_";
static char morseCode[][8] = { ".-","-...","-.-.","-..",".","..-.","--.",
                            "....","..",".---","-.-",".-..","--","-.","---",
                            ".--.","--.-",".-.","...","-","..-","...-",
                            ".--","-..-","-.--","--..",".----","..---",
                            "...--","....-",".....","-....","--...","---..",
                            "----.","-----","..--..",".-.-.-","--..--",
                            "---...",".----.","-....-","-..-.","-.--.-",
                            "-.--.-",".-..-.","-...-","-.-.--","...-..-",
                            ".-...",".-.-.","-.-.-.",".--.-.","..--.-"
                            };


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
bool InitDecodeMorseService(uint8_t Priority)
{
  ES_Event_t ThisEvent;

  MyPriority = Priority;
  
  DB_printf( "Init DecodeMorseService\r\n");
  DB_printf( "Compiled at %s on %s\n", __TIME__, __DATE__);
  DB_printf( "----------------------------------------------------------\n\r");

  memset(morseChars, '\0', sizeof(morseChars));
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
bool PostDecodeMorseService(ES_Event_t ThisEvent)
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
ES_Event_t RunDecodeMorseService(ES_Event_t ThisEvent)
{
  ES_Event_t ReturnEvent;
  ReturnEvent.EventType = ES_NO_EVENT; 

  switch (ThisEvent.EventType){
              
    case DotDetectedEvent:
    {
      if (strlen(morseChars)<= 8){
        strcat(morseChars,".");
      } 
      else {
        ReturnEvent.EventType = ES_ERROR;
      }
    }
    break;

    case DashDetectedEvent:
    {
      if (strlen(morseChars)<= 8){
        strcat(morseChars,"-");
      } else {
        ReturnEvent.EventType = ES_ERROR;
      }
    }
    break;

    case EOCDetected:
    {
      // DB_printf("ES_EOCDetected\r\n");
      char newChar = ConvertMorsetoChar();
      ES_Event_t START_LED_WRITE = {ES_START_LED_WRITE,  newChar};
      PostLEDService(START_LED_WRITE);
      DB_printf("%c", newChar);
      memset(morseChars, '\0', sizeof(morseChars));
    }
    break;

    case EOWDetected:
    {
      // DB_printf("ES_EOWDetected\r\n");
      char newChar = ConvertMorsetoChar();

      //printing char to display
      ES_Event_t START_LED_WRITE = {ES_START_LED_WRITE,  newChar};
      PostLEDService(START_LED_WRITE);
      DB_printf("%c", newChar);
      
      ES_Event_t LED_WRITE_SPACE = {ES_START_LED_WRITE,  ' '};
      PostLEDService(LED_WRITE_SPACE);
      DB_printf("%c", ' ');
      memset(morseChars, '\0', sizeof(morseChars));
    }
    break;
   default:
    {}
    break;
  }

  
  return ReturnEvent;
}

char ConvertMorsetoChar (void){
  for (uint8_t i = 0; i < sizeof(morseCode)/sizeof(morseCode[0]); i++){
    if (strcmp(morseChars, morseCode[i]) == 0){
      return ASCIIChars[i];
    }
  }
  return '~';
}
/***************************************************************************
 private functions
 ***************************************************************************/

/*------------------------------- Footnotes -------------------------------*/
/*------------------------------ End of file ------------------------------*/