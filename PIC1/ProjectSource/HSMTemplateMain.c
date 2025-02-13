/****************************************************************************
 Module
   d:\me218bTNG\Lectures\Lecture31\SMTemplateMain.c

 Revision
   1.0.1

 Description
   This is a template file for a main() implementing Hierarchical 
   State Machines within the Events and Services Framework.

 Notes

 History
 When           Who     What/Why
 -------------- ---     --------
 02/11/21 14:21 jec     updated to match the PIC32 compiler requirements
 11/26/17 14:09 jec     updated with changes from main.c for headers & port 
                        initialization
 02/27/17 12:01 jec     set framework timer rate to 1mS
 02/06/15 13:21 jec     minor tweaks to include header files & clock init for 
                        Tiva
 02/08/12 10:32 jec     major re-work for the Events and Services Framework
                        Gen2
 03/03/10 00:36 jec     now that StartTemplateSM takes an event as input
                        you should pass it something.
 03/17/09 10:20 jec     added cast to return from RunTemplateSM() to quiet
                        warnings because now that function returns Event_t
 02/11/05 16:56 jec     Began coding
****************************************************************************/
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>

#include "ES_Configure.h"
#include "ES_Framework.h"
#include "ES_Port.h"

// stripped down printf()
#include "dbprintf.h"


#define clrScrn() printf("\x1b[2J")
#define goHome() printf("\x1b[1,1H")
#define clrLine() printf("\x1b[K")

void main(void)
{
  ES_Return_t ErrorType;

  _HW_PIC32Init();
  clrScrn();

  // When doing testing, it is useful to announce just which program
  // is running.
// When doing testing, it is useful to announce just which program
// is running.
	puts("\rStarting Test Harness for \r");
	printf("the 2nd Generation Events & Services Framework V2.4\r\n");
	printf("Template for HSM implementation\r\n");
	printf("%s %s\n",__TIME__, __DATE__);
	printf("\n\r\n");


  // Your hardware initialization function calls go here


  // now initialize the Events and Services Framework and start it running
  ErrorType = ES_Initialize(ES_Timer_RATE_1mS);
  if ( ErrorType == Success ) {
    ErrorType = ES_Run();
  }
//if we got to here, there was an error
  switch (ErrorType){
    case FailedPointer:
      puts("Failed on NULL pointer");
      break;
    case FailedInit:
      puts("Failed Initialization");
      break;
    default:
      puts("Other Failure");
      break;
  }
  for(;;)   // hang after reporting error
    ;
}
