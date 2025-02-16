/*----------------------------- Include Files -----------------------------*/
#include "ES_Configure.h"
#include "ES_Framework.h"
#include "PlannerHSM.h"
#include "dbprintf.h"

/*----------------------------- Module Variables ---------------------------*/
static PlannerState_t CurrentState;
static uint8_t MyPriority;
static uint8_t CURRENT_COLUMN = 1;
static uint8_t drop_crate_count = 1;
static PlannerState_t ProcessColumnSubState = GO_TO_STACK; // Start substate

/*---------------------------- Helper Functions ---------------------------*/
const char* GetStateName(PlannerState_t state) {
    switch (state) {
        case INIT_PLANNER: return "INIT_PLANNER";
        case SEARCH_PICKUP_CRATE: return "SEARCH_PICKUP_CRATE";
        case SIDE_DETECTION: return "SIDE_DETECTION";
        case NAVIGATE_TO_COLUMN1: return "NAVIGATE_TO_COLUMN1";
        case PROCESS_COLUMN: return "PROCESS_COLUMN";
        case GO_TO_STACK: return "GO_TO_STACK";
        case DROP_CRATE: return "DROP_CRATE";
        case UPDATE_PROGRESS1: return "UPDATE_PROGRESS1";
        case UPDATE_PROGRESS2: return "UPDATE_PROGRESS2";
        case GO_TO_CRATE: return "GO_TO_CRATE";
        case PICKUP_CRATE: return "PICKUP_CRATE";
        case NAVIGATE_TO_COLUMN2: return "NAVIGATE_TO_COLUMN2";
        case GAME_OVER: return "GAME_OVER";
        default: return "UNKNOWN_STATE";
    }
}

/*---------------------------- During Function ---------------------------*/
static ES_Event_t DuringPROCESS_COLUMN(ES_Event_t Event) {
    ES_Event_t ReturnEvent = Event;

    if (Event.EventType == ES_ENTRY || Event.EventType == ES_ENTRY_HISTORY) {
        DB_printf("Entering PROCESS_COLUMN, CURRENT_COLUMN: %d, Drop Crate Count: %d\n",
                  CURRENT_COLUMN, drop_crate_count);
        ProcessColumnSubState = GO_TO_STACK; // Always start here
    } else if (Event.EventType == ES_EXIT) {
        DB_printf("Exiting PROCESS_COLUMN\n");
    } else {
        // Handle substate transitions
        DB_printf("Current State: %s, Current Column: %d, Drop Crate Count: %d\r\n", GetStateName(ProcessColumnSubState), CURRENT_COLUMN, drop_crate_count);
        switch (ProcessColumnSubState) {
            case GO_TO_STACK:
                if (Event.EventType == ES_AT_STACK) {
                    DB_printf("Crate reached the stack\n");
                    ProcessColumnSubState = DROP_CRATE;
                }
                break;
            
            case DROP_CRATE:
                if (Event.EventType == ES_DROPPED) {
                    DB_printf("Crate dropped\n");
                    drop_crate_count++;
//                    ProcessColumnSubState = UPDATE_PROGRESS2;
                    if (CURRENT_COLUMN == 2 && drop_crate_count == 3) {
                        DB_printf("Finished processing column\n");
                        ES_Event_t ColumnDoneEvent;
                        ColumnDoneEvent.EventType = ES_COLUMN_DONE;
                        PostPlannerHSM(ColumnDoneEvent);
                        return ColumnDoneEvent;  
                    }else{
                       DB_printf("More crates available, moving to next crate\n");
                        ProcessColumnSubState = GO_TO_CRATE; 
                    }                    
                }
                break;     
//            case UPDATE_PROGRESS2:
//
//                break;
            
            case GO_TO_CRATE:
                if (Event.EventType == ES_AT_CRATE) {
                    DB_printf("Arrived at a new crate\n");
                    ProcessColumnSubState = PICKUP_CRATE;
                }
                break;
            
            case PICKUP_CRATE:
                if (Event.EventType == ES_HAS_CRATE) {
                    DB_printf("Picked up a crate\n");
//                    ProcessColumnSubState = UPDATE_PROGRESS1;
                    if (CURRENT_COLUMN == 1 && drop_crate_count == 3) {
                        DB_printf("Column processing completed\n");
                        ES_Event_t ColumnDoneEvent;
                        ColumnDoneEvent.EventType = ES_COLUMN_DONE;
                        PostPlannerHSM(ColumnDoneEvent);
                        return ColumnDoneEvent;  
                    }else{
                        DB_printf("Continuing column process\n");
                        ProcessColumnSubState = GO_TO_STACK;
                    }
                }
                break;
//            
//            case UPDATE_PROGRESS1:
//                if (CURRENT_COLUMN == 1 && drop_crate_count == 3) {
//                    DB_printf("Column processing completed\n");
//                }else{
//                    DB_printf("Continuing column process\n");
//                    ProcessColumnSubState = GO_TO_STACK;
//                }
//                break;

            default:
                break;
        }
    }
    return ReturnEvent;
}

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

    DB_printf("Current State: %s, Current Column: %d, Drop Crate Count: %d\r\n",
              GetStateName(CurrentState), CURRENT_COLUMN, drop_crate_count);

    switch (CurrentState) {
        case INIT_PLANNER:
            if (CurrentEvent.EventType == ES_ENTRY) {
                CURRENT_COLUMN = 1;
                drop_crate_count = 1;
            } else if (CurrentEvent.EventType == ES_INIT_COMPLETE) {
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
            CurrentEvent = DuringPROCESS_COLUMN(CurrentEvent);
            if (CurrentEvent.EventType == ES_COLUMN_DONE) {
                if (CURRENT_COLUMN == 1) {
                    NextState = NAVIGATE_TO_COLUMN2; // Move to next column
                } else {
                    NextState = GAME_OVER; // All processing complete
                }
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
        DB_printf("Transitioning from State: %s to State: %s\r\n", GetStateName(CurrentState), GetStateName(NextState));
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




///*----------------------------- Include Files -----------------------------*/
//#include "ES_Configure.h"
//#include "ES_Framework.h"
//#include "PlannerHSM.h"
//#include "dbprintf.h"
//
///*----------------------------- Module Defines ----------------------------*/
//
///*---------------------------- Module Variables ---------------------------*/
//static PlannerState_t CurrentState;
//static uint8_t MyPriority;
//static uint8_t CURRENT_COLUMN = 1;
//static uint8_t drop_crate_count = 1;
//
//const char* GetStateName(PlannerState_t state) {
//    switch (state) {
//        case INIT_PLANNER: return "INIT_PLANNER";
//        case SEARCH_PICKUP_CRATE: return "SEARCH_PICKUP_CRATE";
//        case SIDE_DETECTION: return "SIDE_DETECTION";
//        case NAVIGATE_TO_COLUMN1: return "NAVIGATE_TO_COLUMN1";
//        case PROCESS_COLUMN: return "PROCESS_COLUMN";
//        case GO_TO_STACK: return "GO_TO_STACK";
//        case DROP_CRATE: return "DROP_CRATE";
//        case UPDATE_PROGRESS1: return "UPDATE_PROGRESS1";
//        case UPDATE_PROGRESS2: return "UPDATE_PROGRESS2";
//        case GO_TO_CRATE: return "GO_TO_CRATE";
//        case PICKUP_CRATE: return "PICKUP_CRATE";
//        case NAVIGATE_TO_COLUMN2: return "NAVIGATE_TO_COLUMN2";
//        case GAME_OVER: return "GAME_OVER";
//        default: return "UNKNOWN_STATE";
//    }
//}
//
///*------------------------------ Module Code ------------------------------*/
//bool InitPlannerHSM(uint8_t Priority) {
//    ES_Event_t ThisEvent;
//    MyPriority = Priority;
//    ThisEvent.EventType = ES_ENTRY;
//    StartPlannerHSM(ThisEvent);
//    return true;
//}
//
//bool PostPlannerHSM(ES_Event_t ThisEvent) {
//    return ES_PostToService(MyPriority, ThisEvent);
//}
//
//ES_Event_t RunPlannerHSM(ES_Event_t CurrentEvent) {
//    bool MakeTransition = false;
//    PlannerState_t NextState = CurrentState;
//    ES_Event_t EntryEventKind = { ES_ENTRY, 0 };
//    ES_Event_t ReturnEvent = { ES_NO_EVENT, 0 };
//
//    DB_printf("Current State: %s, Current Column: %d, Drop Crate Count: %d\r\n",
//              GetStateName(CurrentState), CURRENT_COLUMN, drop_crate_count);
//
//    switch (CurrentState) {
//        case INIT_PLANNER:
//            if (CurrentEvent.EventType == ES_ENTRY) {
//                CURRENT_COLUMN = 1;
//                drop_crate_count = 1;
//            }
//            else if (CurrentEvent.EventType == ES_INIT_COMPLETE) {
//                NextState = SEARCH_PICKUP_CRATE;
//                MakeTransition = true;
//            }
//            break;
//
//        case SEARCH_PICKUP_CRATE:
//            if (CurrentEvent.EventType == ES_HAS_CRATE) {
//                NextState = SIDE_DETECTION;
//                MakeTransition = true;
//            }
//            break;
//
//        case SIDE_DETECTION:
//            if (CurrentEvent.EventType == ES_SIDE_DETECTED) {
//                NextState = NAVIGATE_TO_COLUMN1;
//                MakeTransition = true;
//            }
//            break;
//
//        case NAVIGATE_TO_COLUMN1:
//            if (CurrentEvent.EventType == ES_AT_COLUMN1_INTERSECTION) {
//                NextState = PROCESS_COLUMN;
//                MakeTransition = true;
//            }
//            break;
//
//        case PROCESS_COLUMN:
//            NextState = GO_TO_STACK;
//            MakeTransition = true;
//            break;
//
//        case GO_TO_STACK:
//            if (CurrentEvent.EventType == ES_AT_STACK) {
//                NextState = DROP_CRATE;
//                MakeTransition = true;
//            }
//            break;
//
//        case DROP_CRATE:
//            if (CurrentEvent.EventType == ES_DROPPED) {
//                drop_crate_count++;
//                NextState = UPDATE_PROGRESS2;
//                MakeTransition = true;
//            }
//            break;
//
//        case UPDATE_PROGRESS2:
//            if (CURRENT_COLUMN == 2 && drop_crate_count == 3) {
//                ES_Event_t ColumnDoneEvent;
//                ColumnDoneEvent.EventType = ES_COLUMN_DONE;
//                PostPlannerHSM(ColumnDoneEvent);
//                NextState = GAME_OVER;
//                MakeTransition = true;
//            }
//            break;
//
//        case GO_TO_CRATE:
//            if (CurrentEvent.EventType == ES_AT_CRATE) {
//                NextState = PICKUP_CRATE;
//                MakeTransition = true;
//            }
//            break;
//
//        case PICKUP_CRATE:
//            if (CurrentEvent.EventType == ES_HAS_CRATE) {
//                drop_crate_count++;
//                NextState = UPDATE_PROGRESS1;
//                MakeTransition = true;
//            }
//            break;
//
//        case UPDATE_PROGRESS1:
//            if (CURRENT_COLUMN == 1 && drop_crate_count == 3) {
//                ES_Event_t ColumnDoneEvent;
//                ColumnDoneEvent.EventType = ES_COLUMN_DONE;
//                PostPlannerHSM(ColumnDoneEvent);
//                ES_Event_t Column1CompleteEvent;
//                Column1CompleteEvent.EventType = ES_COLUMN1_COMPLETE;
//                PostPlannerHSM(Column1CompleteEvent);
//                NextState = NAVIGATE_TO_COLUMN2;
//                MakeTransition = true;
//            }
//            break;
//
//        case NAVIGATE_TO_COLUMN2:
//            CURRENT_COLUMN = 2;
//            drop_crate_count = 0;
//            if (CurrentEvent.EventType == ES_AT_COLUMN2_INTERSECTION) {
//                NextState = PROCESS_COLUMN;
//                MakeTransition = true;
//            }
//            break;
//
//        case GAME_OVER:
//            // Final state, FSM terminates
//            break;
//    }
//
//    if (MakeTransition) {
//        DB_printf("Transitioning from State: %s to State: %s\r\n", GetStateName(CurrentState), GetStateName(NextState));
//        CurrentEvent.EventType = ES_EXIT;
//        RunPlannerHSM(CurrentEvent);
//        CurrentState = NextState;
//        RunPlannerHSM(EntryEventKind);
//    }
//    return ReturnEvent;
//}
//
//void StartPlannerHSM(ES_Event_t CurrentEvent) {
//    CurrentState = INIT_PLANNER;
//    RunPlannerHSM(CurrentEvent);
//}
//
//PlannerState_t QueryPlannerHSM(void) {
//    return CurrentState;
//}
