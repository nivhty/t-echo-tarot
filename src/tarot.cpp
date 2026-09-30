#include "tarot.h"
#include "../include/t_echo_pins.h"
#include "../include/tarot_deck.h"

void tarotInit() {
    // nRF52840 has hardware RNG accessible via the Adafruit core
    // Use multiple entropy sources for seeding
    uint32_t seed = 0;
    for (int i = 0; i < 4; i++) {
        seed ^= (analogRead(Adc_Pin) << (i * 8));
        delay(1);
    }
    // Mix in micros for additional entropy
    seed ^= micros();
    randomSeed(seed);
}

uint8_t drawRandomCardIndex() {
    return random(78);
}

bool rollReversed() {
    return random(2) == 0;
}

const TarotCard& getCard(uint8_t index) {
    return tarotDeck[index];
}
