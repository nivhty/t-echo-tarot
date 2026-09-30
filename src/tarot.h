#ifndef TAROT_H
#define TAROT_H

#include <Arduino.h>
#include "../include/tarot_deck.h"

// Initialize the TRNG
void tarotInit();

// Draw a random card index (0-77) using hardware TRNG
uint8_t drawRandomCardIndex();

// Returns true with 50% probability (for reversed cards)
bool rollReversed();

// Get card data by index
const TarotCard& getCard(uint8_t index);

#endif // TAROT_H
