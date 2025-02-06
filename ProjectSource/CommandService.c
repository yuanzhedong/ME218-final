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
#include "CommandService.h"
#include "dbprintf.h"
#include <xc.h>
#include <sys/attribs.h>

/*----------------------------- Module Defines ----------------------------*/
#define QUERY 0xAA
/*---------------------------- Module Functions ---------------------------*/
/* prototypes for private functions for this service.They should be functions
   relevant to the behavior of this service
*/

/*---------------------------- Module Variables ---------------------------*/
// with the introduction of Gen2, we need a module level Priority variable
static uint8_t MyPriority;

volatile static uint16_t PrevCmd;
volatile static uint16_t CurrentCmd;

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
bool InitCommandService(uint8_t Priority)
{
  ES_Event_t ThisEvent;

  MyPriority = Priority;
  /********************************************
   in here you write your initialization code
   *******************************************/
  puts("Start Initializing Command Service\n");

  // Step 0: Disable analog function on all SPI pins
  ANSELBbits.ANSB14 = 0;
  // Step 1: Map SPI Outputs to all desired pins
  TRISBbits.TRISB4 = 0;
  RPB4R = 0b0011; // Map SS1 to RB4
  TRISBbits.TRISB8 = 0;
  RPB8R = 0b0011;        // Map SDO to RB8
  TRISBbits.TRISB14 = 0; // set SCK1 (RB14) as output
  // Step 2: Map SDI
  TRISBbits.TRISB5 = 1; // input
  SDI1R = 0b0001;       // Map SDI1 to RB5
  // Step 3: Disable SPI Module
  SPI1CONbits.ON = 0;
  //  SPI1CONbits.MCLKSEL = 0;
  // Step 4: Clear the receive buffer
  SPI1BUF;
  // Step 5: Enable Enhanced Buffer
  SPI1CONbits.ENHBUF = 0;
  // Step 6: Set Baudrate
  SPI1BRG = 10; // Corresponds to 10 kHz
  // Step 7: Clear the SPIROV Bit
  SPI1STATbits.SPIROV = 0;
  // Step 8: Write desired settings to SPIxCON
  SPI1CONbits.MSTEN = 1; // Places in Leader Mode
  SPI1CONbits.MSSEN = 1; // Drives the CS automatically
  SPI1CONbits.CKE = 0;   // Reads on 2nd edge
  SPI1CONbits.CKP = 1;   // SCK idles high
  // SPI1CONbits.FRMEN = 0; // FRMEN
  SPI1CONbits.FRMPOL = 0; // CS is active low
  SPI1CON2bits.AUDEN = 0; // |
  SPI1CONbits.MODE16 = 0; // Enable 8 bit transfers
  SPI1CONbits.MODE32 = 0; // |
  // Step 9: Initialize Interrupts
  // IEC1CLR = _IEC1_SPI1RXIE_MASK;
  SPI1CONbits.SRXISEL = 0b01;
  //  IFS0CLR = _IFS0_INT4IF_MASK;
  IFS1CLR = _IFS1_SPI1RXIF_MASK;
  IPC7bits.SPI1IP = 6;
  IEC1SET = _IEC1_SPI1RXIE_MASK;
  // INTCONbits.MVEC = 1;
  //  Step 10: Enable SPI
  SPI1CONbits.ON = 1;
  __builtin_enable_interrupts();

  // Initialize Commands
  SPI1BUF = QUERY;
  PrevCmd = 0xFF;
  CurrentCmd = 0xFF;

  ES_Timer_InitTimer(QUERY_TIMER, 200);

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
bool PostCommandService(ES_Event_t ThisEvent)
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
ES_Event_t RunCommandService(ES_Event_t ThisEvent)
{
  ES_Event_t ReturnEvent;
  ReturnEvent.EventType = ES_NO_EVENT; // assume no errors
  /********************************************
   in here you write your service code
   *******************************************/
  if (ES_TIMEOUT == ThisEvent.EventType)
  {
    // Query the Command generator in a timely manner, interval set to 200ms
    if (QUERY_TIMER == ThisEvent.EventParam)
    {
      SPI1BUF = QUERY;
      //DB_printf("Querying\n");
      ES_Timer_InitTimer(QUERY_TIMER, 200);
    }
  }
  else if (ES_GEN == ThisEvent.EventType)
  {
    DB_printf("Command sent: 0x%x\n", ThisEvent.EventParam);
  }
  return ReturnEvent;
}

/***************************************************************************
 private functions
 ***************************************************************************/
void __ISR(_SPI_1_VECTOR, IPL6SOFT) CmdISR(void)
{
  //__builtin_disable_interrupts();
  CurrentCmd = (uint16_t)SPI1BUF;
  IFS1CLR = _IFS1_SPI1RXIF_MASK;
  //DB_printf("In ISR");
  // if((PrevCmd != CurrentCmd) && (CurrentCmd != 0xFF)){
  if ((PrevCmd != CurrentCmd) && (CurrentCmd != 0xFF))
  {
    //DB_printf("sth");
    ES_Event_t CMD_Event;
    CMD_Event.EventType = ES_GEN;
    CMD_Event.EventParam = CurrentCmd;
    PostCommandService(CMD_Event);
    PostRobotFSM(CMD_Event); // CHANGE THIS WHEN IMPLEMENTING EVERYTHING SO MOTORS GET THIS EVENT!!!!!!!!
    PrevCmd = CurrentCmd;
  }
  //__builtin_enable_interrupts();
}
/*------------------------------- Footnotes -------------------------------*/
/*------------------------------ End of file ------------------------------*/
