#pragma once
#include "screen_menu.h"
#include "jessie_bitmap.h"

void drawJessie(WaveshareEPD& epd) {
    epd.clearBuffer();
    epd.drawBitmap(0, 0, jessie_bitmap, 200, 200, 0);
    epd.display();
}

bool updateJessie(WaveshareEPD& epd, TouchResult tr) {
    return tr.event != TOUCH_NONE;
}
