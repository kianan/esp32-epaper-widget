#pragma once
#include <WiFi.h>
#include "rtc.h"

// SGT = UTC+8
#define NTP_SERVER     "pool.ntp.org"
#define NTP_GMT_OFFSET 28800  // seconds

// Sync RTC from NTP. Returns true if successful.
// Call after WiFi is connected.
bool ntpSync() {
    configTime(NTP_GMT_OFFSET, 0, NTP_SERVER);
    struct tm t;
    if (!getLocalTime(&t, 5000)) {
#ifdef DEBUG
        Serial.println("[ntp] sync failed");
#endif
        return false;
    }
    rtcWrite(t.tm_hour, t.tm_min);
#ifdef DEBUG
    Serial.printf("[ntp] synced %02d:%02d\n", t.tm_hour, t.tm_min);
#endif
    return true;
}
