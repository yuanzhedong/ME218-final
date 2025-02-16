/*----------------------------- Include Files -----------------------------*/
#include "ES_Configure.h"
#include "ES_Framework.h"
#include "PlannerHSM.h"
#include "dbprintf.h"

/*----------------------------- Module Defines ----------------------------*/

/*---------------------------- Module Variables ---------------------------*/
static PlannerState_t CurrentState;
static uint8_t MyPriority;
static uint8_t CURRENT_COLUMN = 1;
static uint8_t drop_crate_count = 1;

/*------------------------------ Module Code ------------------------------*/
bool InitPlannerHSM(uint8_t Priority) {
    ES_Event_t ThisEvent;
    MyPriority = Priority;
    ThisEvent.EventType = ES_ENTRY;
    StartPlannerHSM(ThisEvent);
    return true;
}

bool PostPlannerHSM(ES_Event_t ThisEvent) {
    return ES_PostToService(MyPriority, ThisEvent);
}

ES_Event_t RunPlannerHSM(ES_Event_t CurrentEvent) {
    bool MakeTransition = false;
    PlannerState_t NextState = CurrentState;
    ES_Event_t EntryEventKind = { ES_ENTRY, 0 };
    ES_Event_t ReturnEvent = { ES_NO_EVENT, 0 };

    DB_printf("Current State: %d, Current Column: %d, Drop Crate Count: %d\r\n", CurrentState, CURRENT_COLUMN, drop_crate_count);

    switch (CurrentState) {
        case INIT_PLANNER:
            if (CurrentEvent.EventType == ES_ENTRY) {
                CURRENT_COLUMN = 1;
                drop_crate_count = 1;
            }
            else if (CurrentEvent.EventType == ES_INIT_COMPLETE) {
                NextState = SEARCH_PICKUP_CRATE;
                MakeTransition = true;
            }
            break;

        case SEARCH_PICKUP_CRATE:
            if (CurrentEvent.EventType == ES_HAS_CRATE) {
                NextState = SIDE_DETECTION;
                MakeTransition = true;
            }
            break;

        case SIDE_DETECTION:
            if (CurrentEvent.EventType == ES_SIDE_DETECTED) {
                NextState = NAVIGATE_TO_COLUMN1;
                MakeTransition = true;
            }
            break;

        case NAVIGATE_TO_COLUMN1:
            if (CurrentEvent.EventType == ES_AT_COLUMN1_INTERSECTION) {
                NextState = PROCESS_COLUMN;
                MakeTransition = true;
            }
            break;

        case PROCESS_COLUMN:
            NextState = GO_TO_STACK;
            MakeTransition = true;
            break;

        case GO_TO_STACK:
            if (CurrentEvent.EventType == ES_AT_STACK) {
                NextState = DROP_CRATE;
                MakeTransition = true;
            }
            break;

        case DROP_CRATE:
            if (CurrentEvent.EventType == ES_DROPPED) {
                drop_crate_count++;
                NextState = UPDATE_PROGRESS2;
                MakeTransition = true;
            }
            break;

        case UPDATE_PROGRESS2:
            if (CURRENT_COLUMN == 2 && drop_crate_count == 3) {
                ES_Event_t ColumnDoneEvent;
                ColumnDoneEvent.EventType = ES_COLUMN_DONE;
                PostPlannerHSM(ColumnDoneEvent);
                NextState = GAME_OVER;
                MakeTransition = true;
            }
            break;

        case GO_TO_CRATE:
            if (CurrentEvent.EventType == ES_AT_CRATE) {
                NextState = PICKUP_CRATE;
                MakeTransition = true;
            }
            break;

        case PICKUP_CRATE:
            if (CurrentEvent.EventType == ES_HAS_CRATE) {
                drop_crate_count++;
                NextState = UPDATE_PROGRESS1;
                MakeTransition = true;
            }
            break;

        case UPDATE_PROGRESS1:
            if (CURRENT_COLUMN == 1 && drop_crate_count == 3) {
                ES_Event_t ColumnDoneEvent;
                ColumnDoneEvent.EventType = ES_COLUMN_DONE;
                PostPlannerHSM(ColumnDoneEvent);
                ES_Event_t Column1CompleteEvent;
                Column1CompleteEvent.EventType = ES_COLUMN1_COMPLETE;
                PostPlannerHSM(Column1CompleteEvent);
                NextState = NAVIGATE_TO_COLUMN2;
                MakeTransition = true;
            }
            break;

        case NAVIGATE_TO_COLUMN2:
            CURRENT_COLUMN = 2;
            drop_crate_count = 0;
            if (CurrentEvent.EventType == ES_AT_COLUMN2_INTERSECTION) {
                NextState = PROCESS_COLUMN;
                MakeTransition = true;
            }
            break;

        case GAME_OVER:
            // Final state, FSM terminates
            break;
    }

    if (MakeTransition) {
        DB_printf("Transitioning from State: %d to State: %d\r\n", CurrentState, NextState);
        CurrentEvent.EventType = ES_EXIT;
        RunPlannerHSM(CurrentEvent);
        CurrentState = NextState;
        RunPlannerHSM(EntryEventKind);
    }
    return ReturnEvent;
}

void StartPlannerHSM(ES_Event_t CurrentEvent) {
    CurrentState = INIT_PLANNER;
    RunPlannerHSM(CurrentEvent);
}

PlannerState_t QueryPlannerHSM(void) {
    return CurrentState;
}
