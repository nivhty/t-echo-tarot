#include "display.h"
#include <Arduino.h>
#include "t_echo_pins.h"
#include "config.h"
#include "tarot_deck.h"
#include "tarot_bitmaps.h"

#include <GxEPD.h>
#include <GxDEPG0150BN/GxDEPG0150BN.h>
#include <Fonts/FreeMonoBold9pt7b.h>
#include <Fonts/FreeMono9pt7b.h>
#include <GxIO/GxIO_SPI/GxIO_SPI.h>
#include <GxIO/GxIO.h>

SPIClass* dispPort = nullptr;
GxIO_Class* io = nullptr;
GxEPD_Class* display = nullptr;

void setupDisplay() {
    pinMode(Power_Enable_Pin, OUTPUT);
    digitalWrite(Power_Enable_Pin, HIGH);
    pinMode(Power_Enable1_Pin, OUTPUT);
    digitalWrite(Power_Enable1_Pin, HIGH);

    delay(100);

    dispPort = new SPIClass(
        /*SPIPORT*/ NRF_SPIM2,
        /*MISO*/    ePaper_Miso,
        /*SCLK*/    ePaper_Sclk,
        /*MOSI*/    ePaper_Mosi);

    io = new GxIO_Class(
        *dispPort,
        /*CS*/  ePaper_Cs,
        /*DC*/  ePaper_Dc,
        /*RST*/ ePaper_Rst);

    display = new GxEPD_Class(
        *io,
        /*RST*/  ePaper_Rst,
        /*BUSY*/ ePaper_Busy);

    dispPort->begin();
    display->init();
    display->setRotation(0);
}

void drawSplashScreen() {
    display->fillScreen(GxEPD_WHITE);
    display->setTextColor(GxEPD_BLACK);

    // Outer double border
    display->drawRect(10, 10, 180, 180, GxEPD_BLACK);
    display->drawRect(15, 15, 170, 170, GxEPD_BLACK);

    // Mystical circle
    display->drawCircle(100, 100, 70, GxEPD_BLACK);
    display->drawCircle(100, 100, 68, GxEPD_BLACK);

    // Pentagram star inside circle
    // 5 points of the star at 72° intervals, starting from top
    // Top: (100, 30), Right-upper: (167, 79), Right-lower: (141, 157)
    // Left-lower: (59, 157), Left-upper: (33, 79)
    display->drawLine(100, 30, 141, 157, GxEPD_BLACK);   // Top to right-lower
    display->drawLine(141, 157, 33, 79, GxEPD_BLACK);    // Right-lower to left-upper
    display->drawLine(33, 79, 167, 79, GxEPD_BLACK);     // Left-upper to right-upper
    display->drawLine(167, 79, 59, 157, GxEPD_BLACK);    // Right-upper to left-lower
    display->drawLine(59, 157, 100, 30, GxEPD_BLACK);    // Left-lower back to top

    // Small decorative circles at corners
    display->fillCircle(25, 25, 5, GxEPD_BLACK);
    display->fillCircle(175, 25, 5, GxEPD_BLACK);
    display->fillCircle(25, 175, 5, GxEPD_BLACK);
    display->fillCircle(175, 175, 5, GxEPD_BLACK);

    // Title text
    display->setFont(&FreeMonoBold9pt7b);
    int16_t x1, y1;
    uint16_t w, h;
    display->getTextBounds("T-ECHO TAROT", 0, 0, &x1, &y1, &w, &h);
    display->setCursor((200 - w) / 2, 105);
    display->print("T-ECHO TAROT");

    // Subtitle
    display->setFont(&FreeMono9pt7b);
    display->getTextBounds("Draw a card", 0, 0, &x1, &y1, &w, &h);
    display->setCursor((200 - w) / 2, 125);
    display->print("Draw a card");

    display->update();
}

void drawCardFace(const TarotCard& card, bool reversed, uint8_t batteryPct) {
    display->fillScreen(GxEPD_WHITE);
    display->setTextColor(GxEPD_BLACK);

    display->setRotation(0); // Ensure portrait mode for the image

    // --- Card bitmap: 128x160, positioned on the left ---
    uint8_t cardIdx = 0;
    for (uint8_t i = 0; i < DECK_SIZE; i++) {
        if (&tarotDeck[i] == &card) {
            cardIdx = i;
            break;
        }
    }
    const uint8_t* bmp = tarotBitmaps[cardIdx];

    const int16_t imgX = 10;
    const int16_t imgY = 20;

    if (bmp != nullptr) {
        if (reversed) {
            // Draw rotated upside down? No need for this simplified version, or we can just draw normally.
            display->drawBitmap(bmp, imgX, imgY, 128, 160, GxEPD_BLACK);
        } else {
            display->drawBitmap(bmp, imgX, imgY, 128, 160, GxEPD_BLACK);
        }
    } else {
        // Placeholder card outline
        display->drawRect(imgX, imgY, 128, 160, GxEPD_BLACK);
        display->setCursor(imgX + 10, imgY + 80);
        display->setFont(&FreeMono9pt7b);
        display->print("IMG MISSING");
    }

    // --- Card name: Rotated 90 degrees on the right side ---
    display->setRotation(1); // 90 degrees clockwise
    display->setFont(&FreeMonoBold9pt7b);
    
    // In Rotation 1:
    // X axis goes down the physical right side.
    // Y axis goes left across the physical top side.
    // So X_rot1 = 20 aligns with the top of the image (imgY = 20).
    // Y_rot1 = 25 places the baseline at X_rot0 = 174 (far right edge).
    // drawWrappedText will automatically wrap and move Y_rot1 down (which moves X_rot0 left).
    String nameStr = String(card.name) + (reversed ? " (R)" : "");
    drawWrappedText(20, 25, nameStr.c_str(), 160, 16);
    
    // Switch back to Rotation 0
    display->setRotation(0);

    display->update();
}


void drawCardDescription(const TarotCard& card, bool reversed, uint8_t batteryPct) {
    display->fillScreen(GxEPD_WHITE);
    display->setTextColor(GxEPD_BLACK);

    // --- Orientation & suit ---
    display->setFont(&FreeMonoBold9pt7b);
    display->setCursor(5, 20);
    if (reversed) {
        display->print("Reversed");
    } else {
        display->print("Upright");
    }
    display->drawFastHLine(0, 25, 200, GxEPD_BLACK);

    // --- Meaning text (word-wrapped) ---
    display->setFont(&FreeMono9pt7b);
    const char* meaningText = reversed ? card.reversed : card.upright;
    int16_t nextY = drawWrappedText(5, 45, meaningText, 190, 18);

    // --- Keywords ---
    display->setFont(&FreeMonoBold9pt7b);
    display->setCursor(5, nextY + 25);
    display->print("Keywords:");
    
    display->setFont(&FreeMono9pt7b);
    drawWrappedText(5, nextY + 45, card.keywords, 190, 18);

    display->update();
}

void drawShuffleFrame(int dots, bool firstFrame) {
    if (firstFrame) {
        display->fillScreen(GxEPD_WHITE);
        display->update();
    }

    display->fillRect(40, 85, 150, 30, GxEPD_WHITE);
    display->setFont(&FreeMonoBold9pt7b);
    display->setCursor(45, 105);
    
    String text = "Shuffling";
    for(int i = 0; i < dots; i++) {
        text += ".";
    }
    
    display->print(text.c_str());
    display->updateWindow(40, 85, 150, 30, true);
}

int16_t drawWrappedText(int16_t x, int16_t y, const char* text, int16_t maxWidth, int16_t lineHeight) {
    if (!text) return y;
    String word = "";
    int16_t currentX = x;
    int16_t currentY = y;

    int len = strlen(text);
    for (int i = 0; i <= len; ++i) {
        char c = text[i];
        if (c == ' ' || c == '\0' || c == '\n') {
            if (word.length() > 0) {
                int16_t x1, y1;
                uint16_t w, h;
                String testStr = word + " ";
                display->getTextBounds(testStr.c_str(), currentX, currentY, &x1, &y1, &w, &h);
                if (currentX + w > x + maxWidth) {
                    currentX = x;
                    currentY += lineHeight;
                }
                display->setCursor(currentX, currentY);
                display->print(testStr.c_str());
                currentX += w;
                word = "";
            }
            if (c == '\n') {
                currentX = x;
                currentY += lineHeight;
            }
        } else {
            word += c;
        }
    }
    return currentY;
}
