#pragma once
#include "screen_menu.h"
#include <Fonts/FreeSansBold9pt7b.h>

void screen5Init(WaveshareEPD& epd) {
    epd.clearBuffer();
    epd.setFont(&FreeSansBold9pt7b);
    epd.setTextColor(0);
    epd.setCursor(30, 80);  epd.print("Screen 5");
    epd.setCursor(20, 110); epd.print("Coming soon");
    epd.setCursor(30, 185); epd.print("Swipe down");
    epd.displayBase();
}

bool updateScreen5(WaveshareEPD& epd, TouchResult tr) {
    return tr.event == SWIPE_DOWN;
}
