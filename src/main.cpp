#include <Arduino.h>
#include "../include/t_echo_pins.h"
#include "../include/config.h"
#include "display.h"
#include "buttons.h"
#include "tarot.h"
#include "power.h"
#include "state_manager.h"

enum AppState : uint8_t {
    STATE_SPLASH,
    STATE_CARD_FACE,
    STATE_CARD_DESC,
};

AppState currentState = STATE_SPLASH;
uint8_t currentCardIndex = 0;
bool currentReversed = false;
unsigned long lastActivityTime = 0;
unsigned long backlightOffTime = 0;

void doShuffleAnimation() {
    for (int i = 0; i < SHUFFLE_FRAMES; i++) {
        drawShuffleFrame((i % 3) + 1, i == 0); // cycles 1, 2, 3
        delay(SHUFFLE_FRAME_MS);
    }
}

void drawCurrentCard() {
    const TarotCard& card = getCard(currentCardIndex);
    drawCardFace(card, currentReversed, batteryPercent());
}

void drawCurrentDescription() {
    const TarotCard& card = getCard(currentCardIndex);
    drawCardDescription(card, currentReversed, batteryPercent());
}

void setup() {
    Serial.begin(115200);

    // Enable power rails
    pinMode(Power_Enable_Pin, OUTPUT);
    digitalWrite(Power_Enable_Pin, HIGH);
    pinMode(Power_Enable1_Pin, OUTPUT);
    digitalWrite(Power_Enable1_Pin, HIGH);

    // Backlight off initially
    pinMode(ePaper_Backlight, OUTPUT);
    digitalWrite(ePaper_Backlight, LOW);

    // Disable all unused radios (LoRa, GPS, BLE)
    disableAllRadios();

    // Initialize display (GxEPD)
    setupDisplay();

    // Initialize button handling
    setupButtons();

    // Seed random number generator
    tarotInit();

    // Check if we woke from deep sleep with retained state
    uint32_t state = NRF_POWER->GPREGRET;
    if (state & (1 << 31)) {
        currentCardIndex = state & 0xFF;
        currentReversed = (state >> 8) & 1;
        currentState = (AppState)((state >> 9) & 3);
        
        Serial.println("[READY] Woke from deep sleep, restoring state");
        // E-ink retains the image, so no need to redraw here!
    } else if (loadStateFromFlash(currentCardIndex, currentReversed)) {
        currentState = STATE_CARD_FACE;
        Serial.println("[READY] Loaded state from flash, restoring state");
        drawCurrentCard();
    } else {
        // Fresh boot (reset button or power cycle)
        drawSplashScreen();
        currentState = STATE_SPLASH;
        Serial.println("[READY] T-Echo Tarot booted fresh");
    }
    
    lastActivityTime = millis();
}

void loop() {
    // Poll buttons
    checkButtons();

    // Auto-off backlight after timeout
    if (backlightOffTime > 0 && millis() > backlightOffTime) {
        digitalWrite(ePaper_Backlight, LOW);
        backlightOffTime = 0;
    }

    // Consume button event
    ButtonEvent event = consumeButtonEvent();
    if (event != BTN_NONE) {
        lastActivityTime = millis();
    }

    // --- State machine ---
    switch (currentState) {
        case STATE_SPLASH:
            // Only draw a card on explicit button press — never auto-advance
            if (event == BTN_CLICK || event == BTN_LONG_PRESS) {
                currentCardIndex = drawRandomCardIndex();
                currentReversed = rollReversed();
                doShuffleAnimation();
                drawCurrentCard();
                currentState = STATE_CARD_FACE;
            }
            break;

        case STATE_CARD_FACE:
            if (event == BTN_CLICK) {
                // Draw a new random card
                currentCardIndex = drawRandomCardIndex();
                currentReversed = rollReversed();
                doShuffleAnimation();
                drawCurrentCard();
            } else if (event == BTN_LONG_PRESS) {
                // Show card description
                drawCurrentDescription();
                currentState = STATE_CARD_DESC;
            } else if (event == BTN_TOUCH) {
                // Turn on backlight for BACKLIGHT_DURATION_MS
                digitalWrite(ePaper_Backlight, HIGH);
                backlightOffTime = millis() + BACKLIGHT_DURATION_MS;
            }
            break;

        case STATE_CARD_DESC:
            if (event == BTN_LONG_PRESS) {
                // Back to card face
                drawCurrentCard();
                currentState = STATE_CARD_FACE;
            } else if (event == BTN_CLICK) {
                // Draw a new card and go to face view
                currentCardIndex = drawRandomCardIndex();
                currentReversed = rollReversed();
                doShuffleAnimation();
                drawCurrentCard();
                currentState = STATE_CARD_FACE;
            } else if (event == BTN_TOUCH) {
                // Turn on backlight for BACKLIGHT_DURATION_MS
                digitalWrite(ePaper_Backlight, HIGH);
                backlightOffTime = millis() + BACKLIGHT_DURATION_MS;
            }
            break;
    }

    // Deep sleep after inactivity
    if (millis() - lastActivityTime > INACTIVITY_TIMEOUT_MS) {
        uint32_t state = (1 << 31) | (currentState << 9) | (currentReversed << 8) | currentCardIndex;
        NRF_POWER->GPREGRET = state;
        
        // Save to Flash for reset button survival
        saveStateToFlash(currentCardIndex, currentReversed);
        
        enterDeepSleep();
    }
}
