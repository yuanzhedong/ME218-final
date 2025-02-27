#ifndef PLANNER_POLICY_SERVICE_H
#define PLANNER_POLICY_SERVICE_H

#include "ES_Types.h"
#include "ES_Events.h"

#define INVALID_POLICY_IDX -1
#define NAV_TO_COLUMN_1_POLICY 0
#define NAV_TO_COLUMN_2_POLICY 1
#define NAV_TO_COLUMN_3_POLICY 2
#define NAV_TO_COLUMN_4_POLICY 3

// Public Function Prototypes
bool InitPlannerPolicyService(uint8_t Priority);
bool PostPlannerPolicyService(ES_Event_t ThisEvent);
ES_Event_t RunPlannerPolicyService(ES_Event_t ThisEvent);

#endif /* PLANNER_POLICY_SERVICE_H */
