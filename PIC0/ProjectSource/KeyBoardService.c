#include "ES_Configure.h"
#include "ES_Framework.h"
#include "ES_Events.h"
#include "KeyboardService.h"
#include "dbprintf.h"
#include "ES_Port.h"
#include "PlannerHSM.h"
#include "SPIMasterService.h"

/*----------------------------- Module Defines ----------------------------*/
#define INIT_COMPLETE 'i'
#define HAS_CRATE 'p'
#define SIDE_DETECTED 'l'

#define PROCESS_COLUMN 'c'
#define AT_STACK 'g'
#define DROPPED 'u'
#define CHECK_ROBO_STATUS 'v'
#define AT_CRATE 't'
#define PICKUP_CRATE 'k'
#define COLUMN_COMPLETE 'o'
#define STEPPER_FORWARD 'f'
#define STEPPER_BACKWARD 'b'

#define NAV_MOVE_FORWARD 'w'
#define NAV_MOVE_BACKWARD 's'
#define NAV_TURN_LEFT 'a'
#define NAV_TURN_RIGHT 'd'
#define NAV_STOP 'x'
#define NAV_TURN_CW 'e'
#define NAV_TURN_CCW 'q'

#define QUERY_STATUS 'z'

#define START_PLANNER '0'
#define TAPE_ALIGNED '1'
#define AT_COLUMN_INTERSECTION '2'
/*---------------------------- Module Variables ---------------------------*/
static uint8_t MyPriority;

/*------------------------------ Module Code ------------------------------*/
bool InitKeyboardService(uint8_t Priority)
{
    MyPriority = Priority;
    ES_Event_t ThisEvent;
    ThisEvent.EventType = ES_INIT;
    return ES_PostToService(MyPriority, ThisEvent);
}

bool PostKeyboardService(ES_Event_t ThisEvent)
{
    return ES_PostToService(MyPriority, ThisEvent);
}

ES_Event_t RunKeyboardService(ES_Event_t ThisEvent)
{
    ES_Event_t ReturnEvent;
    ReturnEvent.EventType = ES_NO_EVENT;
    ES_Event_t CurEvent;

    if (ThisEvent.EventType == ES_NEW_KEY)
    {   
        char key = ThisEvent.EventParam;
        switch (key)
        {
            case '7':
                CurEvent.EventType = ES_STEPPER_BWD;
                CurEvent.EventParam = 300;
                PostStepperService(CurEvent);
                DB_printf("Stepper Backward\n");
            break;
            case '8':
                CurEvent.EventType = ES_STEPPER_FWD;
                CurEvent.EventParam = 300;
                PostStepperService(CurEvent);
                DB_printf("Stepper Forward\n");
            break;
            case INIT_COMPLETE:
                CurEvent.EventType = ES_INIT_COMPLETE;
                PostPlannerHSM(CurEvent);
                break;
            case HAS_CRATE:
                CurEvent.EventType = ES_HAS_CRATE;
                PostPlannerHSM(CurEvent);
                break;
            case SIDE_DETECTED:
                CurEvent.EventType = ES_SIDE_DETECTED;
                PostPlannerHSM(CurEvent);
                break;
            case AT_STACK:
                CurEvent.EventType = ES_AT_STACK;
                PostPlannerHSM(CurEvent);
                break;
            case DROPPED:
                CurEvent.EventType = ES_DROPPED;
                PostPlannerHSM(CurEvent);
                break;
            case AT_CRATE:
                CurEvent.EventType = ES_AT_CRATE;
                PostPlannerHSM(CurEvent);
                break;
            case PICKUP_CRATE:
                CurEvent.EventType = ES_HAS_CRATE;
                PostPlannerHSM(CurEvent);
                break;
            case COLUMN_COMPLETE:
                CurEvent.EventType = ES_COLUMN_COMPLETE;
                PostPlannerHSM(CurEvent);
                break;
            case STEPPER_FORWARD:
                CurEvent.EventType = ES_STEPPER_FWD;
                CurEvent.EventParam = 100;
                PostStepperService(CurEvent);
                break;
            case STEPPER_BACKWARD:
                CurEvent.EventType = ES_STEPPER_BWD;
                CurEvent.EventParam = 100;
                PostStepperService(CurEvent);
                break;
            case NAV_MOVE_FORWARD:
                CurEvent.EventType = ES_NEW_NAV_CMD;
                CurEvent.EventParam = NAV_CMD_MOVE_FORWARD;
                PostSPIMasterService(CurEvent);
                break;
            case NAV_MOVE_BACKWARD:
                CurEvent.EventType = ES_NEW_NAV_CMD;
                CurEvent.EventParam = NAV_CMD_MOVE_BACKWARD;
                PostSPIMasterService(CurEvent);
                break;
            case NAV_TURN_LEFT:
                CurEvent.EventType = ES_NEW_NAV_CMD;
                CurEvent.EventParam = NAV_CMD_TURN_LEFT;
                PostSPIMasterService(CurEvent);
                break;
            case NAV_TURN_RIGHT:
                CurEvent.EventType = ES_NEW_NAV_CMD;
                CurEvent.EventParam = NAV_CMD_TURN_RIGHT;
                PostSPIMasterService(CurEvent);
                break;
            case NAV_STOP:
                CurEvent.EventType = ES_NEW_NAV_CMD;
                CurEvent.EventParam = NAV_CMD_STOP;
                PostSPIMasterService(CurEvent);
                break;
            case NAV_CMD_TURN_CW:
                CurEvent.EventType = ES_NEW_NAV_CMD;
                CurEvent.EventParam = NAV_CMD_TURN_CW;
                PostSPIMasterService(CurEvent);
                break;
            case NAV_CMD_TURN_CCW:
                CurEvent.EventType = ES_NEW_NAV_CMD;
                CurEvent.EventParam = NAV_CMD_TURN_CCW;
                PostSPIMasterService(CurEvent);
                break;
            case START_PLANNER:
                CurEvent.EventType = ES_START_PLANNER;
                PostPlannerHSM(CurEvent);
                break;
            case TAPE_ALIGNED:
                CurEvent.EventType = ES_TAPE_ALIGNED;
                PostPlannerHSM(CurEvent);
                break;
            case AT_COLUMN_INTERSECTION:
                CurEvent.EventType = ES_AT_COLUMN_INTERSECTION;
                PostPlannerHSM(CurEvent);
                break;
            case QUERY_STATUS:
                CurEvent.EventType = ES_NEW_NAV_CMD;
                CurEvent.EventParam = NAV_CMD_QUERY_STATUS;
                PostSPIMasterService(CurEvent);
                break;
            default:
                return ReturnEvent;
        }
    }
    return ReturnEvent;
}
