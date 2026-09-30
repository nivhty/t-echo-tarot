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
    display->setRotation(3);
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
    display->setCursor(17, 105);
    display->print("T-ECHO TAROT");

    // Subtitle
    display->setFont(&FreeMono9pt7b);
    display->setCursor(30, 125);
    display->print("Draw a card");

    display->update();
}

void drawCardFace(const TarotCard& card, bool reversed, uint8_t batteryPct) {
    display->fillScreen(GxEPD_WHITE);
    display->setTextColor(GxEPD_BLACK);

    // --- Card bitmap: 160x128, centered horizontally ---
    uint8_t cardIdx = 0;
    for (uint8_t i = 0; i < DECK_SIZE; i++) {
        if (&tarotDeck[i] == &card) {
            cardIdx = i;
            break;
        }
    }
    const uint8_t* bmp = tarotBitmaps[cardIdx];

    const int16_t imgX = (200 - CARD_IMG_L_WIDTH) / 2; // (200 - 160)/2 = 20
    const int16_t imgY = 15; // slightly pushed down from top

    if (bmp != nullptr) {
        display->drawBitmap(bmp, imgX, imgY, CARD_IMG_L_WIDTH, CARD_IMG_L_HEIGHT, GxEPD_BLACK);
    } else {
        // Placeholder card outline
        display->drawRect(imgX + 4, imgY + 4, CARD_IMG_L_WIDTH - 8, CARD_IMG_L_HEIGHT - 8, GxEPD_BLACK);
        display->drawRect(imgX + 7, imgY + 7, CARD_IMG_L_WIDTH - 14, CARD_IMG_L_HEIGHT - 14, GxEPD_BLACK);
        display->drawLine(imgX + CARD_IMG_L_WIDTH/2, imgY + 12,
                          imgX + CARD_IMG_L_WIDTH/2, imgY + CARD_IMG_L_HEIGHT - 12, GxEPD_BLACK);
        display->drawLine(imgX + 12, imgY + CARD_IMG_L_HEIGHT/2,
                          imgX + CARD_IMG_L_WIDTH - 12, imgY + CARD_IMG_L_HEIGHT/2, GxEPD_BLACK);
    }

    // --- Card name at bottom ---
    display->setFont(&FreeMonoBold9pt7b);
    
    // Measure string to center it
    int16_t x1, y1;
    uint16_t w, h;
    String nameStr = String(card.name) + (reversed ? " (R)" : "");
    display->getTextBounds(nameStr.c_str(), 0, 0, &x1, &y1, &w, &h);
    
    int16_t textX = (200 - w) / 2;
    int16_t textY = imgY + CARD_IMG_L_HEIGHT + 25; // below image
    
    display->setCursor(textX, textY);
    display->print(nameStr.c_str());

    display->update();
}


void drawCardDescription(const TarotCard& card, bool reversed, uint8_t batteryPct) {
    display->fillScreen(GxEPD_WHITE);
    display->setTextColor(GxEPD_BLACK);

    // --- Card name ---
    display->setFont(&FreeMonoBold9pt7b);
    display->setCursor(5, 20);
    display->print(card.name);

    // --- Orientation & suit ---
    display->setFont(&FreeMono9pt7b);
    display->setCursor(5, 38);
    if (reversed) {
        display->print("Reversed");
    } else {
        display->print("Upright");
    }
    display->drawFastHLine(0, 43, 200, GxEPD_BLACK);

    // --- Meaning text (word-wrapped) ---
    const char* meaningText = reversed ? card.reversed : card.upright;
    drawWrappedText(5, 62, meaningText, 190, 16);

    // --- Keywords ---
    display->setFont(&FreeMono9pt7b);
    display->setCursor(5, 168);
    display->print(card.keywords);

    // --- Navigation hint ---
    display->drawFastHLine(0, 178, 200, GxEPD_BLACK);
    display->setCursor(30, 195);
    display->print("[CLICK: BACK]");

    display->update();
}

void drawShuffleFrame(int dots) {
    display->fillScreen(GxEPD_WHITE);

    display->setFont(&FreeMonoBold9pt7b);
    display->setCursor(45, 105);
    
    String text = "Shuffling";
    for(int i = 0; i < dots; i++) {
        text += ".";
    }
    
    display->print(text.c_str());
    display->update();
}

void drawWrappedText(int16_t x, int16_t y, const char* text, int16_t maxWidth, int16_t lineHeight) {
    if (!text) return;
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
}
