/*----------------------------- Header Guard -----------------------------*/
#ifndef PLANNER_HSM_H
#define PLANNER_HSM_H

/*----------------------------- Include Files -----------------------------*/
#include "ES_Configure.h"
#include "ES_Framework.h"

/*----------------------------- Public Defines ----------------------------*/
typedef enum {
    INIT_PLANNER,
    SEARCH_PICKUP_CRATE,
    TAPE_AND_SIDE_DETECTION,
    NAVIGATE_TO_COLUMN_1,
    PROCESS_COLUMN,
    GO_TO_STACK,
    DROP_CRATE,
    GO_TO_CRATE,
    PICKUP_CRATE,
    NAVIGATE_TO_COLUMN_2,
    CHECK_ROBO_STATUS,
    GAME_OVER
} PlannerState_t;

/*----------------------------- Public Functions ----------------------------*/
bool InitPlannerHSM(uint8_t Priority);
bool PostPlannerHSM(ES_Event_t ThisEvent);
ES_Event_t RunPlannerHSM(ES_Event_t CurrentEvent);
void StartPlannerHSM(ES_Event_t CurrentEvent);
PlannerState_t QueryPlannerHSM(void);

#endif /* PLANNER_HSM_H */
