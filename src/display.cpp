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

    // --- Top header bar (full width) ---
    display->drawFastHLine(0, 18, 200, GxEPD_BLACK);
    display->setFont(&FreeMono9pt7b);
    display->setCursor(4, 14);
    display->print("TAROT");
    // Battery right-aligned
    char batStr[8];
    snprintf(batStr, sizeof(batStr), "%d%%", batteryPct);
    display->setCursor(160, 14);
    display->print(batStr);

    // --- Card bitmap: 160x128, left-aligned, centred vertically below header ---
    // Available height below header: 200-19 = 181px. Centre 128px → offset_y = 19 + (181-128)/2 = 45
    uint8_t cardIdx = 0;
    for (uint8_t i = 0; i < DECK_SIZE; i++) {
        if (&tarotDeck[i] == &card) {
            cardIdx = i;
            break;
        }
    }
    const uint8_t* bmp = tarotBitmaps[cardIdx];

    const int16_t imgX = 0;
    const int16_t imgY = 19 + (181 - CARD_IMG_L_HEIGHT) / 2;  // vertically centred

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

    // --- Vertical divider ---
    display->drawFastVLine(CARD_IMG_L_WIDTH, 19, 181, GxEPD_BLACK);

    // --- Right panel: card name + suit/reversed ---
    // Right panel spans x: 162..199 (38 px wide), y: 20..199
    // Print vertically (rotated text isn't available in GxEPD without custom font);
    // instead print horizontally, wrapping within the narrow column.
    const int16_t panelX = CARD_IMG_L_WIDTH + 4;  // 164
    const int16_t panelW = 200 - panelX;           // ~36 px — tight but readable at 9pt

    display->setFont(&FreeMonoBold9pt7b);
    // Card name — wrap manually, one word per line in this narrow strip
    const char* name = card.name;
    int16_t py = 36;
    // Print each word of the name on its own line
    char nameBuf[32];
    strncpy(nameBuf, name, sizeof(nameBuf) - 1);
    nameBuf[sizeof(nameBuf) - 1] = '\0';
    char* tok = strtok(nameBuf, " ");
    while (tok && py < 170) {
        display->setCursor(panelX, py);
        display->print(tok);
        py += 14;
        tok = strtok(nullptr, " ");
    }

    // Divider before suit
    display->drawFastHLine(panelX, py + 2, panelW, GxEPD_BLACK);
    py += 14;

    // Suit / Reversed
    display->setFont(&FreeMono9pt7b);
    if (reversed) {
        display->setCursor(panelX, py);
        display->print("Rev.");
    } else {
        const char* suitStr = "";
        switch (card.suit) {
            case SUIT_MAJOR:     suitStr = "Maj."; break;
            case SUIT_WANDS:     suitStr = "Wand"; break;
            case SUIT_CUPS:      suitStr = "Cups"; break;
            case SUIT_SWORDS:    suitStr = "Swrd"; break;
            case SUIT_PENTACLES: suitStr = "Pent"; break;
        }
        display->setCursor(panelX, py);
        display->print(suitStr);
    }

    display->update();
}


void drawCardDescription(const TarotCard& card, bool reversed, uint8_t batteryPct) {
    display->fillScreen(GxEPD_WHITE);
    display->setTextColor(GxEPD_BLACK);

    // --- Header ---
    display->setFont(&FreeMono9pt7b);
    display->setCursor(5, 15);
    display->print("TAROT");
    display->setCursor(150, 15);
    display->print(batteryPct);
    display->print("%");
    display->drawFastHLine(0, 20, 200, GxEPD_BLACK);

    // --- Card name ---
    display->setFont(&FreeMonoBold9pt7b);
    display->setCursor(5, 40);
    display->print(card.name);

    // --- Orientation & suit ---
    display->setFont(&FreeMono9pt7b);
    display->setCursor(5, 58);
    if (reversed) {
        display->print("Reversed");
    } else {
        display->print("Upright");
    }
    display->drawFastHLine(0, 63, 200, GxEPD_BLACK);

    // --- Meaning text (word-wrapped) ---
    const char* meaningText = reversed ? card.reversed : card.upright;
    drawWrappedText(5, 82, meaningText, 190, 16);

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

void drawShuffleFrame() {
    display->fillScreen(GxEPD_WHITE);
    // Draw a card-back pattern with random rectangles
    for (int i = 0; i < 20; ++i) {
        int x = random(0, 180);
        int y = random(0, 180);
        int w = random(10, 40);
        int h = random(10, 40);
        if (random(2)) {
            display->fillRect(x, y, w, h, GxEPD_BLACK);
        } else {
            display->drawRect(x, y, w, h, GxEPD_BLACK);
        }
    }
    // Center text
    display->setFont(&FreeMonoBold9pt7b);
    display->setCursor(30, 105);
    display->print("Shuffling...");
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
