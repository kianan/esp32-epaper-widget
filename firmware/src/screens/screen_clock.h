#pragma once
#include "screen_menu.h"
#include "shtc3.h"
#include "rtc.h"
#include "battery.h"
#include <Fonts/FreeMonoBold18pt7b.h>
#include <Fonts/FreeSans9pt7b.h>

#define CLOCK_MENU_STRIP_Y 170

bool updateClock(WaveshareEPD& epd, TouchResult tr) {
    static unsigned long lastDraw = 0;
    unsigned long now = millis();

    if (tr.event != TOUCH_NONE) return true; // any gesture = back to menu

    if (now - lastDraw < 10000 && lastDraw != 0) return false;
    lastDraw = now;

    epd.clearBuffer();
    epd.setTextColor(0);

    // Time
    epd.setFont(&FreeMonoBold18pt7b);
    int h = 0, m = 0;
    char timeBuf[6];
    if (rtcRead(h, m)) {
        snprintf(timeBuf, sizeof(timeBuf), "%02d:%02d", h, m);
    } else {
        snprintf(timeBuf, sizeof(timeBuf), "--:--");
    }
    {
        int16_t x1, y1; uint16_t tw, th;
        epd.getTextBounds(timeBuf, 0, 0, &x1, &y1, &tw, &th);
        epd.setCursor((200 - tw) / 2 - x1, 82);
    }
    epd.print(timeBuf);

    epd.setFont(&FreeSans9pt7b);

    auto printCentered = [&](const char* s, int y) {
        int16_t x1, y1; uint16_t tw, th;
        epd.getTextBounds(s, 0, 0, &x1, &y1, &tw, &th);
        epd.setCursor((200 - tw) / 2 - x1, y);
        epd.print(s);
    };

    float tempC = -999, humidity = -999;
    shtc3Read(tempC, humidity);

    char tempBuf[24];
    if (tempC > -900)
        snprintf(tempBuf, sizeof(tempBuf), "%.1f C", tempC);
    else
        snprintf(tempBuf, sizeof(tempBuf), "Temp: err");
    printCentered(tempBuf, 112);

    char humBuf[24];
    if (humidity > -900)
        snprintf(humBuf, sizeof(humBuf), "%.0f%% RH", humidity);
    else
        snprintf(humBuf, sizeof(humBuf), "Hum: err");
    printCentered(humBuf, 133);

    char battBuf[16];
    snprintf(battBuf, sizeof(battBuf), "Batt: %d%%", batteryPercent());
    printCentered(battBuf, 154);

    epd.drawLine(0, CLOCK_MENU_STRIP_Y, 200, CLOCK_MENU_STRIP_Y, 0);
    epd.setCursor(55, 190); epd.print("< MENU");

    epd.display();
    return false;
}
