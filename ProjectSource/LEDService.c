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
#include "../ProjectHeaders/LEDService.h"
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


/*----------------------------- Module Defines ----------------------------*/
// these times assume a 10.000mS/tick timing
#define ONE_SEC 1000
#define HALF_SEC (ONE_SEC / 2)
#define TWO_SEC (ONE_SEC * 2)
#define FIVE_SEC (ONE_SEC * 5)

#define ENTER_POST     ((MyPriority<<3)|0)
#define ENTER_RUN      ((MyPriority<<3)|1)
#define ENTER_TIMEOUT  ((MyPriority<<3)|2)

//#define TEST_INT_POST
//#define BLINK LED
/*---------------------------- Module Functions ---------------------------*/
/* prototypes for private functions for this service.They should be functions
   relevant to the behavior of this service
*/
#ifdef BLINK_LED
static void InitLED(void);
static void BlinkLED(void);
#endif

#ifdef TEST_INT_POST
static void InitTMR2(void);
static void StartTMR2(void);
#endif
/*---------------------------- Module Variables ---------------------------*/
// with the introduction of Gen2, we need a module level Priority variable
static uint8_t MyPriority;
// add a deferral queue for up to 3 pending deferrals +1 to allow for overhead
static uint8_t UpdatingLED = 0;

static ES_Event_t DeferralQueue[3 + 1];

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



bool InitLEDService(uint8_t Priority)
{
    SPISetup_BasicConfig(SPI_SPI1);
    SPISetup_SetLeader(SPI_SPI1, SPI_SMP_MID);
    SPISetup_MapSSOutput(SPI_SPI1, SPI_RPA0);
    SPISetup_MapSDOutput(SPI_SPI1, SPI_RPA1);
    SPI1BUF;
    SPISetEnhancedBuffer(SPI_SPI1, 1);
    SPISetup_SetBitTime(SPI_SPI1, 10000);
    SPISetup_SetXferWidth(SPI_SPI1, SPI_16BIT);
    SPISetup_SetActiveEdge(SPI_SPI1, SPI_SECOND_EDGE);
    SPISetup_SetClockIdleState(SPI_SPI1, SPI_CLK_HI);
    SPI1CONbits.FRMPOL = 0;
    SPISetup_EnableSPI(SPI_SPI1);
  ES_Event_t ThisEvent;
  
   ES_InitDeferralQueueWith(DeferralQueue, ARRAY_SIZE(DeferralQueue));

  MyPriority = Priority;

  // When doing testing, it is useful to announce just which program
  // is running.
  clrScrn();
  puts("\rStarting LED SERVICE for \r");
  DB_printf( "compiled at %s on %s\n", __TIME__, __DATE__);  
  while (false == DM_TakeInitDisplayStep()) {
        // Continue calling to fully initialize the display

  }
  DB_printf( "Finish init LED\n");

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
bool PostLEDService(ES_Event_t ThisEvent)
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
ES_Event_t RunLEDService(ES_Event_t ThisEvent)
{
  ES_Event_t ReturnEvent;
  ReturnEvent.EventType = ES_NO_EVENT; // assume no errors
  //return ReturnEvent;
  
#ifdef _INCLUDE_BYTE_DEBUG_
  _HW_ByteDebug_SetValueWithStrobe( ENTER_RUN );
#endif  
  switch (ThisEvent.EventType)
  {
    case ES_INIT:
    {
//      ES_Timer_InitTimer(SERVICE0_TIMER, HALF_SEC);
      puts("Service LED:");
      DB_printf("\rES_INIT received in LED Service %d\r\n", MyPriority);
    }
    break;
//    case ES_TIMEOUT:   // re-start timer & announce
//    {
//      ES_Timer_InitTimer(SERVICE0_TIMER, FIVE_SEC);
//      DB_printf("ES_TIMEOUT received from Timer %d in Service %d\r\n",
//          ThisEvent.EventParam, MyPriority);
//    }
//    break;

   case ES_NEW_KEY:   // announce
   {
    // DB_printf("ES_NEW_KEY received with -> %c <- in Service 0\r\n",
    //     (char)ThisEvent.EventParam);
     if (UpdatingLED) {
         DB_printf("Ignore input due to updating LED buffer....");
         break;
     }
     ES_Event_t START_LED_WRITE = {ES_START_LED_WRITE, ThisEvent.EventParam};

     PostLEDService(START_LED_WRITE);
   }
    break;

    case ES_UPDATE_LIVE: {
      uint8_t currentLives = ThisEvent.EventParam;
      //map range 100 to range 32
      DB_printf("Current Lives level: %d\n", currentLives);
      uint8_t level = currentLives * 1.0 / 100 * 32;
      DB_printf("Current LED level: %d\n", level);
    }
    
    case ES_START_LED_WRITE: {
        //DB_printf("ES_START_LED_WRITE received in Service %d\r\n", MyPriority);
        if (UpdatingLED == 1) {
            //puts("xxxxxx\r");

            if (ES_DeferEvent(DeferralQueue, ThisEvent)==true)
            {
              //puts("ES_START_LED_WRITE deferred in LED Service\r");
                ;
            }
            //DB_printf("Still updating LED. SKIP ES_START_LED_WRITE received in Service %d\r\n", MyPriority);
            break;
        }
        UpdatingLED = 1;
        DM_ScrollDisplayBuffer(4);
        DM_AddChar2DisplayBuffer((char)ThisEvent.EventParam);
        ES_Event_t LED_WRITE_ROW = {ES_LED_WRITE_ROW, ThisEvent.EventParam};
        PostLEDService(LED_WRITE_ROW);
    }
    break;
    case ES_LED_WRITE_ROW: {
        //DB_printf("ES_LED_WRITE_ROW received in Service %d\r\n", MyPriority);
        if (false == DM_TakeDisplayUpdateStep()) {
            PostLEDService(ThisEvent);
        }
        else {
           UpdatingLED = 0;
           //dequeue if needed
           if (true == ES_RecallEvents(MyPriority, DeferralQueue)) {
            //puts("writing recalled in LEDService \r");
               ;
           }
        }
    }
    break;
    default:
    {}
     break;
  }

  return ReturnEvent;
}

/***************************************************************************
 private functions
 ***************************************************************************/
#ifdef BLINK_LED
#define LED LATBbits.LATB6
static void InitLED(void)
{
  LED = 0; //start with it off
  TRISBbits.TRISB6 = 0; // set RB6 as an output
}

static void BlinkLED(void)
{
  // toggle state of LED
  LED = ~LED;
}
#endif

#ifdef TEST_INT_POST
#include <sys/attribs.h> // for ISR macors

// for testing posting from interrupts.
// Intializes TMR2 to gerenate an interrupt at 100ms
static void InitTMR2(void)
{
  // turn timer off
  T2CONbits.ON = 0;
  // Use internal peripheral clock
  T2CONbits.TCS = 0;
  // setup for 16 bit mode
  T2CONbits.T32 = 0;
  // set prescale to 1:1
  T2CONbits.TCKPS = 0;
  // load period value
  PR2 = 2000-1; // creates a 100ms period with a 20MHz peripheral clock
  // set priority
  IPC2bits.T2IP = 2;
  // clear interrupt flag
  IFS0bits.T2IF = 0;
  // enable the timer interrupt
  IEC0bits.T2IE = 1;
}

// Clears and Starts TMR2
static void StartTMR2(void)
{
  // clear timer
  TMR2 = 0;
  // start timer
  //LATBbits.LATB14 = 0;
  T2CONbits.ON = 1;
}

void __ISR(_TIMER_2_VECTOR, IPL2AUTO) Timer2ISR(void)
{
  // clear flag
  IFS0bits.T2IF = 0;
  // post event
  static ES_Event_t interruptEvent = {ES_SHORT_TIMEOUT, 0};
  PostTestHarnessService1(interruptEvent);
  
  // stop timer
  T2CONbits.ON = 0;
  return;
}
#endif
/*------------------------------- Footnotes -------------------------------*/
/*------------------------------ End of file ------------------------------*/

