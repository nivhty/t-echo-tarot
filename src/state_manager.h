#ifndef STATE_MANAGER_H
#define STATE_MANAGER_H

#include <Arduino.h>

void saveStateToFlash(uint8_t cardIdx, uint8_t reversed);
bool loadStateFromFlash(uint8_t& cardIdx, uint8_t& reversed);

#endif
