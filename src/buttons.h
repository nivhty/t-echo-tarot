#ifndef BUTTONS_H
#define BUTTONS_H

#include <Arduino.h>

// Button event types for the main loop to consume
enum ButtonEvent : uint8_t {
    BTN_NONE = 0,
    BTN_CLICK,          // Short press on user button
    BTN_LONG_PRESS,     // Long press on user button (>1.5s)
    BTN_TOUCH,          // Capacitive touch pad tapped
};

void setupButtons();
void checkButtons();
ButtonEvent consumeButtonEvent();  // Returns latest event and clears it

#endif // BUTTONS_H
