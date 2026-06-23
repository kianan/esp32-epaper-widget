#pragma once
#include "screen_menu.h"
#include "jessie_bitmap.h"

namespace jessie {

void screenInit(WaveshareEPD& epd)
{
    epd.clearBuffer();
    epd.drawBitmap(0, 0, jessie_bitmap, 200, 200, 0);
    epd.display();
}

bool screenUpdate(WaveshareEPD& epd, TouchResult tr)
{
    return tr.event != TOUCH_NONE;
}

} // namespace jessie
