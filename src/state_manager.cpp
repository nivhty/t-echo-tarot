#include "state_manager.h"
#include <Adafruit_LittleFS.h>
#include <InternalFileSystem.h>

using namespace Adafruit_LittleFS_Namespace;

void saveStateToFlash(uint8_t cardIdx, uint8_t reversed) {
    InternalFS.begin();
    File file = InternalFS.open("/state.txt", FILE_O_WRITE);
    if (file) {
        file.write(cardIdx);
        file.write(reversed);
        file.close();
    }
}

bool loadStateFromFlash(uint8_t& cardIdx, uint8_t& reversed) {
    InternalFS.begin();
    File file = InternalFS.open("/state.txt", FILE_O_READ);
    if (file) {
        if (file.size() >= 2) {
            cardIdx = file.read();
            reversed = file.read();
            file.close();
            return true;
        }
        file.close();
    }
    return false;
}
