#pragma once
#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include "WaveshareEPD.h"
#include "screen_menu.h"
#include "touch.h"
#include "wifi_conn.h"
#include "ntp.h"
#include <Fonts/FreeSansBold9pt7b.h>

#define SCREEN6_URL "https://xd5f1t.mockapi.dog"

static void _drawScreen6Content(WaveshareEPD& epd, const String& status, const String& body) {
    epd.clearBuffer();
    epd.setFont(&FreeSansBold9pt7b);
    epd.setTextColor(0);
    epd.setCursor(10, 18); epd.print("API");
    epd.drawLine(0, 25, 200, 25, 0);

    // Default 6x8 font for content — 33 chars/line, ~21 lines available
    epd.setFont(NULL);
    epd.setTextSize(1);
    epd.setTextWrap(true);

    int y = 32;
    if (status.length()) {
        epd.setCursor(0, y);
        epd.print(status);
        y += 10;
    }
    epd.setCursor(0, y);
    epd.print(body.length() ? body.substring(0, 700) : "(empty)");

    epd.setTextWrap(false);
    epd.setFont(&FreeSansBold9pt7b);
    epd.setCursor(0, 197); epd.print("Swipe right: back  Tap: refresh");
}

static void _screen6Fetch(WaveshareEPD& epd, bool firstLoad) {
    if (!wifiConnected()) {
        _drawScreen6Content(epd, "No WiFi", "");
        if (firstLoad) epd.displayBase(); else epd.displayPartial();
        return;
    }

    // Show "Fetching..." — full refresh on first load, partial on refresh
    epd.clearBuffer();
    epd.setFont(&FreeSansBold9pt7b);
    epd.setTextColor(0);
    epd.setCursor(10, 18); epd.print("API");
    epd.drawLine(0, 25, 200, 25, 0);
    epd.setFont(NULL);
    epd.setTextSize(1);
    epd.setCursor(0, 40); epd.print("Fetching...");
    if (firstLoad) {
        epd.displayBase();
        epd.initPartial();
    } else {
        epd.displayPartial();
    }

    WiFiClientSecure client;
    client.setInsecure();
    HTTPClient http;
    http.begin(client, SCREEN6_URL);
    http.setTimeout(8000);
    int code = http.GET();

    if (code > 0) {
        String body = http.getString();
#ifdef DEBUG
        Serial.printf("[api] HTTP %d, %d bytes\n", code, body.length());
#endif
        _drawScreen6Content(epd, "HTTP " + String(code), body);
    } else {
        String err = http.errorToString(code);
#ifdef DEBUG
        Serial.printf("[api] error %d: %s\n", code, err.c_str());
#endif
        _drawScreen6Content(epd, "Error " + String(code), err);
    }
    http.end();
    epd.displayPartial();
}

void screen6Init(WaveshareEPD& epd) {
    bool needsConnect = !wifiConnected();
    if (needsConnect) {
        // Show "Connecting..." immediately before blocking WiFi connect
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
    // temp measure to sync clock in screen 6. migrate if there's better generic place
    if (wifiConnected()) ntpSync();
    // firstLoad=true triggers displayBase+initPartial; if we already showed
    // "Connecting...", use partial from here on
    _screen6Fetch(epd, !needsConnect);
}

bool updateScreen6(WaveshareEPD& epd, TouchResult tr) {
    if (tr.event == SWIPE_RIGHT) return true;
    if (tr.event == TOUCH_TAP)  _screen6Fetch(epd, false);
    return false;
}
