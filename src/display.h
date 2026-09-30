#ifndef DISPLAY_H
#define DISPLAY_H

#include <Arduino.h>

// Forward declarations
struct TarotCard;

void setupDisplay();
void drawSplashScreen();
void drawCardFace(const TarotCard& card, bool reversed, uint8_t batteryPct);
void drawCardDescription(const TarotCard& card, bool reversed, uint8_t batteryPct);
void drawShuffleFrame(int dots, bool firstFrame);  // Single frame of shuffle animation

// Text helper: word-wrap text within a pixel width
void drawWrappedText(int16_t x, int16_t y, const char* text, int16_t maxWidth, int16_t lineHeight);

#endif // DISPLAY_H
