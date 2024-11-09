#ifndef COINLEDSERVICE_H
#define COINLEDSERVICE_H

// Event Definitions
#include "ES_Configure.h" /* gets us event definitions */
#include "ES_Types.h"     /* gets bool type for returns */
#include "ES_Events.h"

// typedefs for the states
// State definitions for use with the query function
typedef enum
{
    InitPState,
    WaitForCoin,
    GameStart,
} CoinLEDServiceState_t;

// Public Function Prototypes

bool InitCoinLEDService(uint8_t Priority);
bool PostCoinLEDService(ES_Event_t ThisEvent);
ES_Event_t RunCoinLEDService(ES_Event_t ThisEvent);
CoinLEDServiceState_t QueryCoinLEDService(void);

#endif /* COINLEDSERVICE_H */
