#ifndef DecodeMorseService_H
#define DecodeMorseService_H

#include "ES_Types.h"

// Public Function Prototypes

bool InitDecodeMorseService(uint8_t Priority);
bool PostDecodeMorseService(ES_Event_t ThisEvent);
ES_Event_t RunDecodeMorseService(ES_Event_t ThisEvent);
char ConvertMorsetoChar ();

#endif /* ServTemplate_H */