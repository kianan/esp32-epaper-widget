#pragma once
#include "WaveshareEPD.h"
#include "touch.h"
#include "menu_logic.h"
#include "rtc.h"
#include <Fonts/FreeSansBold9pt7b.h>

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
        char _menuTimeBuf[6] = "--:--";
        int _mh, _mm;
        if (rtcRead(_mh, _mm)) snprintf(_menuTimeBuf, sizeof(_menuTimeBuf), "%02d:%02d", _mh, _mm);
        const char* tl[] = { "Display", _menuTimeBuf, "Sensors" };
        drawBlock(epd, tl, 3, 0, 0, GRID_MID_X, GRID_MID_Y);
        const char* tr[] = { "Focus", "Timer" };
        drawBlock(epd, tr, 2, GRID_MID_X, 0, GRID_MID_X, GRID_MID_Y);
        const char* bl[] = { "Photos" };
        drawBlock(epd, bl, 1, 0, GRID_MID_Y, GRID_MID_X, 190 - GRID_MID_Y);
        const char* br[] = { "Jessie" };
        drawBlock(epd, br, 1, GRID_MID_X, GRID_MID_Y, GRID_MID_X, 190 - GRID_MID_Y);
    } else if (_menuPage == 1) {
        const char* tl[] = { "WiFi" };  drawBlock(epd, tl, 1, 0,          0,          GRID_MID_X, GRID_MID_Y);
        const char* tr[] = { "API" };   drawBlock(epd, tr, 1, GRID_MID_X, 0,          GRID_MID_X, GRID_MID_Y);
        const char* bl[] = { "Audio" }; drawBlock(epd, bl, 1, 0,          GRID_MID_Y, GRID_MID_X, 190 - GRID_MID_Y);
        const char* br[] = { "SD" };    drawBlock(epd, br, 1, GRID_MID_X, GRID_MID_Y, GRID_MID_X, 190 - GRID_MID_Y);
    } else {
        const char* tl[] = { "Voice" }; drawBlock(epd, tl, 1, 0,          0,          GRID_MID_X, GRID_MID_Y);
        const char* tr[] = { "---" }; drawBlock(epd, tr, 1, GRID_MID_X, 0,          GRID_MID_X, GRID_MID_Y);
        const char* bl[] = { "---" }; drawBlock(epd, bl, 1, 0,          GRID_MID_Y, GRID_MID_X, 190 - GRID_MID_Y);
        const char* br[] = { "---" }; drawBlock(epd, br, 1, GRID_MID_X, GRID_MID_Y, GRID_MID_X, 190 - GRID_MID_Y);
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
    return menuScreenForTap(tr.x, tr.y, _menuPage);
}
