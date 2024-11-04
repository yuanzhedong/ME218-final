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

void CharacterizePulse(void);
void CharacterizeSpace(void);
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
                    //puts("\rR");  // Indicate rising edge
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
                    //puts("\rF");  // Indicate falling edge
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
                case ES_MORSE_RISE:
                    TimeOfLastRise = ThisEvent.EventParam;
                    CharacterizeSpace();
                    CurrentState = EOC_WaitFall;
                 break;
            }
            if (ThisEvent.EventType == ES_INIT_MORSE) { // only respond to ES_Init
                DB_printf("MorseElementsService running.\n\r");
                CurrentState = CalWaitForRise;
            }
        break;
        
        case EOC_WaitFall:
            switch (ThisEvent.EventType) {
            case ES_MORSE_FALL:
                puts("\rF");  // Indicate falling edge
                TimeOfLastFall = ThisEvent.EventParam;
            
                // Call CharacterizeSpace to determine if it's a character, word, or bad space
                CharacterizeSpace();
            
                // Transition to the next state based on what was characterized
                CurrentState = EOC_WaitRise;  // Transition to EOC_WaitRise to wait for the next rising edge
            break;
        
            case ES_BUTTON_PRESSED:  // Handle re-calibration if necessary
                DB_printf("Re-calibrate initiated!");
                FirstDelta = 0;
                CurrentState = CalWaitForRise;
            break;

            case EOCDetected:
                puts("eoc detected");
                    CurrentState = DecodeWaitFall;
            break;

            case EOWDetected:
                puts("eow detected");
                    CurrentState = DecodeWaitFall;
            break;

            default:
                // Handle any unexpected events if necessary
            break;
            }
        break;

        case DecodeWaitFall:
            switch (ThisEvent.EventType) {
                case ES_MORSE_FALL:
                    // Set TimeOfLastFall to the time from the event parameter
                    TimeOfLastFall = ThisEvent.EventParam;

                    // Transition to DecodeWaitRise
                    CurrentState = DecodeWaitRise;

                    // Call CharacterizePulse to process the pulse length
                    CharacterizePulse();
                break;
                case ES_BUTTON_PRESSED:
                    // If the double button (DBButtonDown) event is detected
                    // Transition to CalWaitForRise for recalibration
                    CurrentState = CalWaitForRise;

                    // Reset FirstDelta to 0 for calibration
                     FirstDelta = 0;
                break;
            }
        break;

        case DecodeWaitRise:
            if (ThisEvent.EventType == ES_MORSE_RISE) {
                // Set TimeOfLastRise to the time from the event parameter
                TimeOfLastRise = ThisEvent.EventParam;

                // Transition to DecodeWaitFall
                CurrentState = DecodeWaitFall;

                // Call CharacterizeSpace function to process the space length
                CharacterizeSpace();
            }
            else if (ThisEvent.EventType == ES_BUTTON_PRESSED) {
                // If the double button (DBButtonDown) event is detected
                // Transition to CalWaitForRise for recalibration
             CurrentState = CalWaitForRise;

                // Reset FirstDelta to 0 for calibration
             FirstDelta = 0;
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


#define TOLERANCE 1  // Define tolerance for space length comparison

// Check if the given interval matches the dot space criteria
bool IsDotSpace(uint16_t interval) {
    uint16_t lowerBound = LengthOfDot - TOLERANCE;
    uint16_t upperBound = LengthOfDot + TOLERANCE;
    return (interval >= lowerBound) && (interval <= upperBound);
}

// Check if the given interval matches the character space criteria
bool IsCharacterSpace(uint16_t interval) {
    uint16_t characterSpaceLength = 3 * LengthOfDot;
    //DB_printf("characterSpaceLength: %d \n\r",characterSpaceLength);

    uint16_t lowerBound = characterSpaceLength - TOLERANCE * 3;
    uint16_t upperBound = characterSpaceLength + TOLERANCE * 3;
    return (interval >= lowerBound) && (interval <= upperBound);
}

// Check if the given interval matches the word space criteria
bool IsWordSpace(uint16_t interval) {
    uint16_t wordSpaceLength = 7 * LengthOfDot;
    uint16_t lowerBound = wordSpaceLength - TOLERANCE * 7;
    uint16_t upperBound = wordSpaceLength + TOLERANCE * 7;
    return (interval >= lowerBound) && (interval <= upperBound);
}


void CharacterizeSpace(void) {
    uint16_t LastInterval = TimeOfLastRise - TimeOfLastFall;  // Calculate interval between last rise and fall
    
    // Determine the type of space based on the LastInterval
    if (!IsDotSpace(LastInterval)) {  // If not a Dot space
        //DB_printf("TimeOfLastRise: %d \n\r",TimeOfLastRise);
        //DB_printf("TimeOfLastFall: %d \n\r",TimeOfLastFall);

        //DB_printf("LastInterval: %d \n\r",LastInterval);
        if (IsCharacterSpace(LastInterval)) {  // Check if it’s a Character space
            // Post an EOCDetected event for End of Character
            //puts("*****");
            ES_Event_t Event2Post;
            Event2Post.EventType = EOCDetected;
            DB_printf(" ");

            PostMorseElementsService(Event2Post);  // Post event to Morse Elements Service

        } else if (IsWordSpace(LastInterval)) {  // Check if it’s a Word space
            // Post an EOWDetected event for End of Word
            ES_Event_t Event2Post;
            Event2Post.EventType = EOWDetected;
            DB_printf("   ");

            PostMorseElementsService(Event2Post);  // Post event to Morse Elements Service
        } else {  
            // If neither Character nor Word space, post a BadSpace event
            ES_Event_t Event2Post;
            Event2Post.EventType = BadSpace;
            PostMorseElementsService(Event2Post);  // Post event to Morse Elements Service
        }
    }
    // If it's a Dot space, do nothing per the provided instructions
}

void CharacterizePulse(void) {
    uint16_t LastPulseWidth = TimeOfLastFall - TimeOfLastRise;
    ES_Event_t Event2Post;

    // Determine if the pulse is a dot
    if (LastPulseWidth >= LengthOfDot - TOLERANCE && LastPulseWidth <= LengthOfDot + TOLERANCE) {
        DB_printf(".");
        Event2Post.EventType = DotDetectedEvent;
    }
    // Determine if the pulse is a dash
    else if (LastPulseWidth >= 3 * LengthOfDot - TOLERANCE && LastPulseWidth <= 3 * LengthOfDot + TOLERANCE) {
        DB_printf("-");

        Event2Post.EventType = DashDetectedEvent;
    }
    // Otherwise, it's an invalid pulse
    else {
        Event2Post.EventType = BadPulseEvent;
    }

    // Post the event to the Decode Morse Service
    PostMorseElementsService(Event2Post);
}




/*------------------------------- Footnotes -------------------------------*/
/*------------------------------ End of file ------------------------------*/

