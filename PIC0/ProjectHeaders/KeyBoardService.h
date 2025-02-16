/*----------------------------- Header Guard -----------------------------*/
#ifndef KEYBOARD_SERVICE_H
#define KEYBOARD_SERVICE_H

/*----------------------------- Include Files -----------------------------*/
#include "ES_Configure.h"
#include "ES_Framework.h"

/*----------------------------- Public Functions ----------------------------*/
bool InitKeyboardService(uint8_t Priority);
bool PostKeyboardService(ES_Event_t ThisEvent);
ES_Event_t RunKeyboardService(ES_Event_t CurrentEvent);

#endif /* KEYBOARD_SERVICE_H */
