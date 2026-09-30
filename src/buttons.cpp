#include "buttons.h"
#include <AceButton.h>
#include "../include/t_echo_pins.h"
#include "../include/config.h"

using namespace ace_button;

static AceButton userButton(UserButton_Pin);
static ButtonEvent currentEvent = BTN_NONE;

void buttonEventHandler(AceButton* button, uint8_t eventType, uint8_t buttonState) {
    if (eventType == AceButton::kEventClicked) {
        currentEvent = BTN_CLICK;
    } else if (eventType == AceButton::kEventLongPressed) {
        currentEvent = BTN_LONG_PRESS;
    }
}

void setupButtons() {
    pinMode(UserButton_Pin, INPUT_PULLUP);
    pinMode(Touch_Pin, INPUT);

    ButtonConfig* buttonConfig = ButtonConfig::getSystemButtonConfig();
    buttonConfig->setEventHandler(buttonEventHandler);
    buttonConfig->setFeature(ButtonConfig::kFeatureClick);
    buttonConfig->setFeature(ButtonConfig::kFeatureLongPress);
    buttonConfig->setFeature(ButtonConfig::kFeatureSuppressAfterLongPress);
    buttonConfig->setLongPressDelay(LONG_PRESS_MS);

    userButton.init(UserButton_Pin, HIGH, 0);
}

void checkButtons() {
    userButton.check();

    static bool lastTouch = false;
    bool touch = digitalRead(Touch_Pin) == HIGH;
    if (touch && !lastTouch) {
        currentEvent = BTN_TOUCH;
    }
    lastTouch = touch;
}

ButtonEvent consumeButtonEvent() {
    ButtonEvent evt = currentEvent;
    currentEvent = BTN_NONE;
    return evt;
}
