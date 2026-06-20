#pragma once
#include <Arduino.h>
#include "esp_sleep.h"
#include "pins.h"

#define SLEEP_TIMEOUT_MS  (60UL * 1000)

static unsigned long _lastActivityMs = 0;

void activityPing() {
    _lastActivityMs = millis();
}

bool sleepTimeoutReached() {
    return millis() - _lastActivityMs > SLEEP_TIMEOUT_MS;
}

void enterDeepSleep(uint64_t timerWakeupUs = 0) {
    uint64_t wakePins = (1ULL << TOUCH_INT) | (1ULL << PWR_BTN);
    esp_sleep_enable_ext1_wakeup(wakePins, ESP_EXT1_WAKEUP_ANY_LOW);

    if (timerWakeupUs > 0) {
        esp_sleep_enable_timer_wakeup(timerWakeupUs);
    }

    esp_deep_sleep_start();
}
