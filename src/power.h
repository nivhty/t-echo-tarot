#ifndef POWER_H
#define POWER_H

#include <Arduino.h>

void disableAllRadios();
void enterDeepSleep();
float readBatteryVoltage();
uint8_t batteryPercent();

#endif // POWER_H
