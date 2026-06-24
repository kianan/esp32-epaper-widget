#pragma once
#include "screen_menu.h"
#include <Fonts/FreeSansBold9pt7b.h>

namespace screen11 {

void screenInit(WaveshareEPD& epd)
{
    epd.clearBuffer();
    epd.setFont(&FreeSansBold9pt7b);
    epd.setTextColor(0);
    epd.setCursor(30, 80);  epd.print("Screen 11");
    epd.setCursor(20, 110); epd.print("Coming soon");
    epd.setCursor(5, 194);  epd.print("Swipe right: back");
    epd.displayBase();
}

bool screenUpdate(WaveshareEPD& epd, TouchResult tr)
{
    return tr.event == SWIPE_RIGHT;
}

} // namespace screen11
