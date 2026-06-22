#pragma once
#include "WaveshareEPD.h"
#include "touch.h"
#include "audio.h"
#include <Fonts/FreeSansBold9pt7b.h>

static void _drawAudioScreen(WaveshareEPD& epd)
{
    epd.clearBuffer();
    epd.setFont(&FreeSansBold9pt7b);
    epd.setTextColor(0);

    epd.setCursor(10, 18); epd.print("Audio Test");
    epd.drawLine(0, 25, 200, 25, 0);
    epd.drawLine(0, 112, 200, 112, 0);

    // Top half — chime 1
    epd.setCursor(10, 60);  epd.print("Chime 1");
    epd.setCursor(10, 85);  epd.print("880 Hz (high)");

    // Bottom half — chime 2
    epd.setCursor(10, 148); epd.print("Chime 2");
    epd.setCursor(10, 173); epd.print("659 Hz (low)");

    epd.setFont(NULL); epd.setTextSize(1);
    epd.setCursor(5, 194); epd.print("Swipe right: back");

    epd.displayBase();
}

void screen7Init(WaveshareEPD& epd)
{
    _drawAudioScreen(epd);
    audioInit();
}

bool updateScreen7(WaveshareEPD& epd, TouchResult tr)
{
    if (tr.event == SWIPE_RIGHT) return true;

    if (tr.event == TOUCH_TAP) {
        if (tr.y < 112) {
            audioPlayChime(1);
        } else {
            audioPlayChime(2);
        }
    }
    return false;
}
