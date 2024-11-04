/****************************************************************************
 Module
   TestHarnessService1.c

 Revision
   1.0.1

 Description
   This is the first service for the Test Harness under the
   Gen2 Events and Services Framework.

 Notes

 History
 When           Who     What/Why
 -------------- ---     --------
 10/26/17 18:26 jec     moves definition of ALL_BITS to ES_Port.h
 10/19/17 21:28 jec     meaningless change to test updating
 10/19/17 18:42 jec     removed referennces to driverlib and programmed the
                        ports directly
 08/21/17 21:44 jec     modified LED blink routine to only modify bit 3 so that
                        I can test the new new framework debugging lines on PF1-2
 08/16/17 14:13 jec      corrected ONE_SEC constant to match Tiva tick rate
 11/02/13 17:21 jec      added exercise of the event deferral/recall module
 08/05/13 20:33 jec      converted to test harness service
 01/16/12 09:58 jec      began conversion from TemplateFSM.c
****************************************************************************/
/*----------------------------- Include Files -----------------------------*/
// This module
#include "../ProjectHeaders/MorseElementsService.h"
#include "../ProjectHeaders/DM_Display.h"

// debugging printf()

// Hardware
#include <xc.h>
#include <xc.h>
#include "PIC32_SPI_HAL.h"
//#include <proc/p32mx170f256b.h>

// Event & Services Framework
#include "ES_Configure.h"
#include "ES_Framework.h"
#include "ES_DeferRecall.h"
#include "ES_Port.h"
#include "terminal.h"
#include "dbprintf.h"


/*---------------------------- Module Variables ---------------------------*/
// with the introduction of Gen2, we need a module level Priority variable
static uint8_t MyPriority;
static uint16_t TimeOfLastRise;
static uint16_t TimeOfLastFall;
static uint16_t LengthOfDot;
static uint16_t FirstDelta;

static MorseElementState_t CurrentState;

/*------------------------------ Module Code ------------------------------*/
/****************************************************************************
 Function
     InitTestHarnessService0

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



bool InitMorseElementsService(uint8_t Priority)
{
    TRISAbits.TRISA2 = 1;
//    SPISetup_BasicConfig(SPI_SPI1);
//    SPISetup_SetLeader(SPI_SPI1, SPI_SMP_MID);
//    SPISetup_MapSSOutput(SPI_SPI1, SPI_RPA0);
//    SPISetup_MapSDOutput(SPI_SPI1, SPI_RPA1);
//    SPISetup_MapSDInput(SPI_SPI1, SPI_RPA1);
//
//    SPI1BUF;
//    SPISetEnhancedBuffer(SPI_SPI1, 1);
//    SPISetup_SetBitTime(SPI_SPI1, 10000);
//    SPISetup_SetXferWidth(SPI_SPI1, SPI_16BIT);
//    SPISetup_SetActiveEdge(SPI_SPI1, SPI_SECOND_EDGE);
//    SPISetup_SetClockIdleState(SPI_SPI1, SPI_CLK_HI);
//    SPI1CONbits.FRMPOL = 0;
//    SPISetup_EnableSPI(SPI_SPI1);
  ES_Event_t ThisEvent;

  MyPriority = Priority;

  // When doing testing, it is useful to announce just which program
  // is running.
  clrScrn();


  InitButtonStatus();
  puts("\rStarting Morse Elements SERVICE for \r");
  DB_printf( "compiled at %s on %s\n", __TIME__, __DATE__);  
  //while (false == DM_TakeInitDisplayStep()) {
        // Continue calling to fully initialize the display

  //}
  DB_printf( "Finish init Morse Elements SERVICE\n");

  ThisEvent.EventType = ES_INIT_MORSE;
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
     PostLEDService

 Parameters
     ES_Event ThisEvent ,the event to post to the queue

 Returns
     bool false if the Enqueue operation failed, true otherwise

 Description
     Posts an event to this state machine's queue
 Notes

 Author
     J. Edward Carryer, 10/23/11, 19:25
****************************************************************************/
bool PostMorseElementsService(ES_Event_t ThisEvent)
{
  return ES_PostToService(MyPriority, ThisEvent);
}

/****************************************************************************
 Function
    RunTestHarnessService0

 Parameters
   ES_Event : the event to process

 Returns
   ES_Event, ES_NO_EVENT if no error ES_ERROR otherwise

 Description
   add your description here
 Notes

 Author
   J. Edward Carryer, 01/15/12, 15:23
****************************************************************************/
ES_Event_t RunMorseElementsService(ES_Event_t ThisEvent)
{
    ES_Event_t ReturnEvent;
    ReturnEvent.EventType = ES_NO_EVENT;
    
    switch(CurrentState) {
        case InitPState:  // If current state is initial Pseudo State
            if (ThisEvent.EventType == ES_INIT_MORSE) { // only respond to ES_Init
                // Actions for transitioning from initial pseudo-state to actual initial state
                DB_printf("MorseElementsService running.\n\r");
                CurrentState = CalWaitForRise;
            }
        break;

        case CalWaitForRise:
            switch (ThisEvent.EventType) {
                case ES_MORSE_RISE:
                    puts("\rR");  // Indicate rising edge
                    TimeOfLastRise = ThisEvent.EventParam;
                    CurrentState = CalWaitForFall;
                break;
                
                case ES_CALIBRATION_COMPLETE:
                    CurrentState = EOC_WaitRise;
                    DB_printf("Length of dot: %d \n\r",LengthOfDot);
                break;
            }
        break;

        case CalWaitForFall:
            switch (ThisEvent.EventType) {
                case ES_MORSE_FALL:
                    puts("\rF");  // Indicate falling edge
                    TimeOfLastFall = ThisEvent.EventParam;
                    CurrentState = CalWaitForRise;
                    TestCalibration();
                break;
            }
        break;

        case EOC_WaitRise:
            //DB_printf("we are in the eoc state");
            switch (ThisEvent.EventType) {
                case ES_BUTTON_PRESSED:
                    DB_printf("Re-calibrate initiated!");
                    FirstDelta == 0;
                    CurrentState = CalWaitForRise;
                break;       
            }
            if (ThisEvent.EventType == ES_INIT_MORSE) { // only respond to ES_Init
                DB_printf("MorseElementsService running.\n\r");
                CurrentState = CalWaitForRise;
            }
        break;

        default:
        break;
    }
    return ReturnEvent;
}

void TestCalibration(){
    //DB_printf("First Delta: %d \n\r",FirstDelta);
    if (FirstDelta == 0){
        FirstDelta = TimeOfLastFall - TimeOfLastRise;
    }else{
        uint16_t SecondDelta = TimeOfLastFall - TimeOfLastRise;
        //DB_printf("Second Delta: %d \n\r",SecondDelta);
        if ((100.0 * (double) FirstDelta / (double)SecondDelta) <= 33.3333) {
            LengthOfDot = FirstDelta;
            ES_Event_t NewEvent;
            NewEvent.EventType = ES_CALIBRATION_COMPLETE;
            PostMorseElementsService(NewEvent);
            DB_printf("First Delta: %d \n\r",FirstDelta);
            DB_printf("First Delta: %d \n\r",SecondDelta);
        }else if ((100.0 * (double)FirstDelta / (double)SecondDelta) > 300.0) {
          LengthOfDot = SecondDelta;
          ES_Event_t ThisEvent;
          ThisEvent.EventType = ES_CALIBRATION_COMPLETE;
          PostMorseElementsService(ThisEvent);
          DB_printf("First Delta: %d \n\r",FirstDelta);
          DB_printf("First Delta: %d \n\r",SecondDelta);
        }
        else {
          FirstDelta = SecondDelta;
        }
    }
}

void ButtonPressd(void){
    //DB_printf("First Delta: %d \n\r",FirstDelta);
    if (FirstDelta == 0){
        FirstDelta = TimeOfLastFall - TimeOfLastRise;
    }else{
        uint16_t SecondDelta = TimeOfLastFall - TimeOfLastRise;
        DB_printf("Second Delta: %d \n\r",SecondDelta);
        if ((100.0 * (double) FirstDelta / (double)SecondDelta) <= 33.3333 && 
             (100.0 * (double) FirstDelta / (double)SecondDelta) > 30.000) {
            LengthOfDot = FirstDelta;
            ES_Event_t NewEvent;
            NewEvent.EventType = ES_CALIBRATION_COMPLETE;
            PostMorseElementsService(NewEvent);
        }else if ((100.0 * (double)FirstDelta / (double)SecondDelta) > 300.0
                && (100.0 * (double)FirstDelta / (double)SecondDelta) < 316.0) {
          LengthOfDot = SecondDelta;
          ES_Event_t ThisEvent;
          ThisEvent.EventType = ES_CALIBRATION_COMPLETE;
          PostMorseElementsService(ThisEvent);
        }
        else {
          FirstDelta = SecondDelta;
        }
    }
}

/*------------------------------- Footnotes -------------------------------*/
/*------------------------------ End of file ------------------------------*/

