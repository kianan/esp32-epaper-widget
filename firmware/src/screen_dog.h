#pragma once
#include "screen_menu.h"
#include "dog_bitmap.h"

void drawDog(WaveshareEPD& epd) {
    epd.clearBuffer();
    epd.drawBitmap(0, 0, dog_bitmap, DOG_BITMAP_W, DOG_BITMAP_H, 0);
    epd.display();
}

// Returns true when tapped (back to menu)
bool updateDog(WaveshareEPD& epd, bool touched) {
    if (touched) return true;
    return false;
}
