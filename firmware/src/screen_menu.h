#pragma once
#include "WaveshareEPD.h"
#include "touch.h"
#include <Fonts/FreeSansBold9pt7b.h>

enum Screen { SCREEN_MENU, SCREEN_CLOCK, SCREEN_POMODORO, SCREEN_PHOTOS, SCREEN_JESSIE };

static void drawBlock(WaveshareEPD& epd, const char** lines, int count,
                      int qx, int qy, int qw, int qh) {
    const int GAP = 3;
    int16_t x1, y1;
    uint16_t tw, th;

    int lineH = 0;
    for (int i = 0; i < count; i++) {
        epd.getTextBounds(lines[i], 0, 0, &x1, &y1, &tw, &th);
        if ((int)th > lineH) lineH = th;
    }

    int totalH = lineH * count + GAP * (count - 1);
    int blockTop = qy + (qh - totalH) / 2;

    for (int i = 0; i < count; i++) {
        epd.getTextBounds(lines[i], 0, 0, &x1, &y1, &tw, &th);
        int cx = qx + (qw - (int)tw) / 2 - x1;
        int cy = blockTop + i * (lineH + GAP) - y1;
        epd.setCursor(cx, cy);
        epd.print(lines[i]);
    }
}

void drawMenu(WaveshareEPD& epd) {
    epd.clearBuffer();
    epd.setFont(&FreeSansBold9pt7b);
    epd.setTextColor(0);

    epd.drawLine(100, 0, 100, 200, 0);
    epd.drawLine(0, 100, 200, 100, 0);

    const char* tlLines[] = { "Clock", "+ Temp", "+ Batt" };
    drawBlock(epd, tlLines, 3, 0, 0, 100, 100);

    const char* trLines[] = { "Focus", "Timer" };
    drawBlock(epd, trLines, 2, 100, 0, 100, 100);

    const char* blLines[] = { "Photos" };
    drawBlock(epd, blLines, 1, 0, 100, 100, 100);

    const char* brLines[] = { "Jessie" };
    drawBlock(epd, brLines, 1, 100, 100, 100, 100);

    epd.displayBase();
}

Screen menuHandleTouch(TouchResult tr) {
    if (tr.event != TOUCH_TAP) return SCREEN_MENU;
    if (tr.x < 100 && tr.y < 100)   return SCREEN_CLOCK;
    if (tr.x >= 100 && tr.y < 100)  return SCREEN_POMODORO;
    if (tr.x < 100  && tr.y >= 100) return SCREEN_PHOTOS;
    if (tr.x >= 100 && tr.y >= 100) return SCREEN_JESSIE;
    return SCREEN_MENU;
}
