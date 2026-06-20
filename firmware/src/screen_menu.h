#pragma once
#include "WaveshareEPD.h"
#include "touch.h"
#include <Fonts/FreeSansBold9pt7b.h>

enum Screen {
    SCREEN_MENU,
    SCREEN_CLOCK, SCREEN_POMODORO, SCREEN_PHOTOS, SCREEN_JESSIE,  // page 0
    SCREEN_5, SCREEN_6, SCREEN_7, SCREEN_8                        // page 1
};

#define MENU_PAGES  2
#define GRID_MID_Y  95   // horizontal divider — leaves 10px for dots
#define GRID_MID_X  100

static int _menuPage = 0;

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

static void _drawMenuPage(WaveshareEPD& epd) {
    epd.drawLine(GRID_MID_X, 0, GRID_MID_X, 190, 0);
    epd.drawLine(0, GRID_MID_Y, 200, GRID_MID_Y, 0);

    if (_menuPage == 0) {
        const char* tl[] = { "Clock", "+ Temp", "+ Batt" };
        drawBlock(epd, tl, 3, 0, 0, GRID_MID_X, GRID_MID_Y);
        const char* tr[] = { "Focus", "Timer" };
        drawBlock(epd, tr, 2, GRID_MID_X, 0, GRID_MID_X, GRID_MID_Y);
        const char* bl[] = { "Photos" };
        drawBlock(epd, bl, 1, 0, GRID_MID_Y, GRID_MID_X, 190 - GRID_MID_Y);
        const char* br[] = { "Jessie" };
        drawBlock(epd, br, 1, GRID_MID_X, GRID_MID_Y, GRID_MID_X, 190 - GRID_MID_Y);
    } else {
        const char* tl[] = { "---" }; drawBlock(epd, tl, 1, 0,           0,          GRID_MID_X, GRID_MID_Y);
        const char* tr[] = { "---" }; drawBlock(epd, tr, 1, GRID_MID_X,  0,          GRID_MID_X, GRID_MID_Y);
        const char* bl[] = { "---" }; drawBlock(epd, bl, 1, 0,           GRID_MID_Y, GRID_MID_X, 190 - GRID_MID_Y);
        const char* br[] = { "---" }; drawBlock(epd, br, 1, GRID_MID_X,  GRID_MID_Y, GRID_MID_X, 190 - GRID_MID_Y);
    }

    // Page indicator dots
    const int dotR = 3, dotSpacing = 12, dotY = 196;
    int startX = 100 - ((MENU_PAGES - 1) * dotSpacing) / 2;
    for (int i = 0; i < MENU_PAGES; i++) {
        int cx = startX + i * dotSpacing;
        if (i == _menuPage) epd.fillCircle(cx, dotY, dotR, 0);
        else                epd.drawCircle(cx, dotY, dotR, 0);
    }
}

void drawMenu(WaveshareEPD& epd) {
    epd.clearBuffer();
    epd.setFont(&FreeSansBold9pt7b);
    epd.setTextColor(0);
    _drawMenuPage(epd);
    epd.displayBase();
}

Screen menuHandleTouch(TouchResult tr, WaveshareEPD& epd) {
    if (tr.event == SWIPE_LEFT) {
        _menuPage = (_menuPage + 1) % MENU_PAGES;
        drawMenu(epd);
        return SCREEN_MENU;
    }
    if (tr.event == SWIPE_RIGHT) {
        _menuPage = (_menuPage - 1 + MENU_PAGES) % MENU_PAGES;
        drawMenu(epd);
        return SCREEN_MENU;
    }
    if (tr.event != TOUCH_TAP) return SCREEN_MENU;

    bool top  = tr.y < GRID_MID_Y;
    bool left = tr.x < GRID_MID_X;

    if (_menuPage == 0) {
        if (left  && top)  return SCREEN_CLOCK;
        if (!left && top)  return SCREEN_POMODORO;
        if (left  && !top) return SCREEN_PHOTOS;
        if (!left && !top) return SCREEN_JESSIE;
    } else {
        if (left  && top)  return SCREEN_5;
        if (!left && top)  return SCREEN_6;
        if (left  && !top) return SCREEN_7;
        if (!left && !top) return SCREEN_8;
    }
    return SCREEN_MENU;
}
