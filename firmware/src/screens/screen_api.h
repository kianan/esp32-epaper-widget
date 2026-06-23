#pragma once
#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include "WaveshareEPD.h"
#include "screen_menu.h"
#include "touch.h"
#include "wifi_conn.h"
#include "ntp.h"
#include <Fonts/FreeSansBold9pt7b.h>

namespace api {

#define API_URL "https://xd5f1t.mockapi.dog"

static void _drawContent(WaveshareEPD& epd, const String& status, const String& body)
{
    epd.clearBuffer();
    epd.setFont(&FreeSansBold9pt7b);
    epd.setTextColor(0);
    epd.setCursor(10, 18); epd.print("API");
    epd.drawLine(0, 25, 200, 25, 0);

    epd.setFont(NULL);
    epd.setTextSize(1);
    epd.setTextWrap(true);

    int y = 32;
    if (status.length()) { epd.setCursor(0, y); epd.print(status); y += 10; }
    epd.setCursor(0, y);
    epd.print(body.length() ? body.substring(0, 700) : "(empty)");

    epd.setTextWrap(false);
    epd.setFont(&FreeSansBold9pt7b);
    epd.setCursor(0, 197); epd.print("Swipe right: back  Tap: refresh");
}

static void _fetch(WaveshareEPD& epd, bool firstLoad)
{
    if (!wifiConnected()) {
        _drawContent(epd, "No WiFi", "");
        if (firstLoad) epd.displayBase(); else epd.displayPartial();
        return;
    }

    epd.clearBuffer();
    epd.setFont(&FreeSansBold9pt7b);
    epd.setTextColor(0);
    epd.setCursor(10, 18); epd.print("API");
    epd.drawLine(0, 25, 200, 25, 0);
    epd.setFont(NULL); epd.setTextSize(1);
    epd.setCursor(0, 40); epd.print("Fetching...");
    if (firstLoad) { epd.displayBase(); epd.initPartial(); }
    else             epd.displayPartial();

    WiFiClientSecure client;
    client.setInsecure();
    HTTPClient http;
    http.begin(client, API_URL);
    http.setTimeout(8000);
    int code = http.GET();

    if (code > 0) {
        String body = http.getString();
#ifdef DEBUG
        Serial.printf("[api] HTTP %d, %d bytes\n", code, body.length());
#endif
        _drawContent(epd, "HTTP " + String(code), body);
    } else {
        String err = http.errorToString(code);
#ifdef DEBUG
        Serial.printf("[api] error %d: %s\n", code, err.c_str());
#endif
        _drawContent(epd, "Error " + String(code), err);
    }
    http.end();
    epd.displayPartial();
}

void screenInit(WaveshareEPD& epd)
{
    bool needsConnect = !wifiConnected();
    if (needsConnect) {
        epd.clearBuffer();
        epd.setFont(&FreeSansBold9pt7b);
        epd.setTextColor(0);
        epd.setCursor(10, 18); epd.print("API");
        epd.drawLine(0, 25, 200, 25, 0);
        epd.setFont(NULL); epd.setTextSize(1);
        epd.setCursor(0, 40); epd.print("Connecting...");
        epd.displayBase();
        epd.initPartial();
        wifiBegin();
    }
    if (wifiConnected()) ntpSync();
    _fetch(epd, !needsConnect);
}

bool screenUpdate(WaveshareEPD& epd, TouchResult tr)
{
    if (tr.event == SWIPE_RIGHT) return true;
    if (tr.event == TOUCH_TAP)  _fetch(epd, false);
    return false;
}

} // namespace api
