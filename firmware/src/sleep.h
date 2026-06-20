#pragma once
#include <Arduino.h>
#include "esp_sleep.h"
#include "pins.h"

#define SLEEP_TIMEOUT_MS  (60UL * 1000)

// Saved before sleep so elapsed time can be computed on wake
RTC_DATA_ATTR int _rtcSleepSecsSinceMidnight = -1;

static unsigned long _lastActivityMs = 0;

void activityPing() {
    _lastActivityMs = millis();
}

bool sleepTimeoutReached() {
    return millis() - _lastActivityMs > SLEEP_TIMEOUT_MS;
}

// elapsedSecs since midnight when device went to sleep, -1 if RTC unavailable
int sleepElapsedSecs(int nowSecsSinceMidnight) {
    if (_rtcSleepSecsSinceMidnight < 0 || nowSecsSinceMidnight < 0) return 0;
    return (nowSecsSinceMidnight - _rtcSleepSecsSinceMidnight + 86400) % 86400;
}

void enterDeepSleep(int rtcSecsSinceMidnight, uint64_t timerWakeupUs = 0) {
    _rtcSleepSecsSinceMidnight = rtcSecsSinceMidnight;

    uint64_t wakePins = (1ULL << TOUCH_INT) | (1ULL << PWR_BTN);
    esp_sleep_enable_ext1_wakeup(wakePins, ESP_EXT1_WAKEUP_ANY_LOW);

    if (timerWakeupUs > 0) {
        esp_sleep_enable_timer_wakeup(timerWakeupUs);
    }

    esp_deep_sleep_start();
}
