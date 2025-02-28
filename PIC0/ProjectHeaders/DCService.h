#ifndef DCSERVICE_H
#define DCSERVICE_H

#include "ES_Configure.h"
#include "ES_Framework.h"
#include <stdint.h>
#include <stdbool.h>

/*----------------------------- Public Defines ----------------------------*/
#define DC_MOTOR_TIMER  1  // Timer ID for motor control

/*----------------------------- Public Functions ----------------------------*/

/**
 * @brief Initializes the DC Motor service.
 * 
 * @param Priority The priority of this service.
 * @return true if initialization is successful, false otherwise.
 */
bool InitDCMotorService(uint8_t Priority);

/**
 * @brief Posts an event to the DC Motor service.
 * 
 * @param ThisEvent The event to post.
 * @return true if the event is successfully posted, false otherwise.
 */
bool PostDCMotorService(ES_Event_t ThisEvent);

/**
 * @brief Runs the DC Motor service and handles incoming events.
 * 
 * @param ThisEvent The event to process.
 * @return ES_Event, ES_NO_EVENT if no error occurs.
 */
ES_Event_t RunDCMotorService(ES_Event_t ThisEvent);

#endif /* DCSERVICE_H */
