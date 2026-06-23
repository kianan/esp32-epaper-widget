#pragma once
#include "screen_menu.h"
#include "shtc3.h"
#include "rtc.h"
#include "battery.h"
#include <Fonts/FreeMonoBold18pt7b.h>
#include <Fonts/FreeSans9pt7b.h>

namespace screenClock {

#define CLOCK_MENU_STRIP_Y 170

static unsigned long _lastDraw = 0;

static void _draw(WaveshareEPD& epd)
{
    epd.clearBuffer();
    epd.setTextColor(0);

    epd.setFont(&FreeMonoBold18pt7b);
    int h = 0, m = 0;
    char timeBuf[6];
    if (rtcRead(h, m)) snprintf(timeBuf, sizeof(timeBuf), "%02d:%02d", h, m);
    else                snprintf(timeBuf, sizeof(timeBuf), "--:--");
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
    snprintf(tempBuf, sizeof(tempBuf), tempC > -900 ? "%.1f C" : "Temp: err", tempC);
    printCentered(tempBuf, 112);

    char humBuf[24];
    snprintf(humBuf, sizeof(humBuf), humidity > -900 ? "%.0f%% RH" : "Hum: err", humidity);
    printCentered(humBuf, 133);

    char battBuf[16];
    snprintf(battBuf, sizeof(battBuf), "Batt: %d%%", batteryPercent());
    printCentered(battBuf, 154);

    epd.drawLine(0, CLOCK_MENU_STRIP_Y, 200, CLOCK_MENU_STRIP_Y, 0);
    epd.setFont(NULL); epd.setTextSize(1);
    epd.setCursor(5, 194); epd.print("Swipe right: back");

    epd.display();
}

void screenInit(WaveshareEPD& epd)
{
    _lastDraw = 0;
    _draw(epd);
}

bool screenUpdate(WaveshareEPD& epd, TouchResult tr)
{
    if (tr.event == SWIPE_RIGHT) return true;

    unsigned long now = millis();
    if (now - _lastDraw >= 10000) {
        _lastDraw = now;
        _draw(epd);
    }
    return false;
}

} // namespace screenClock
