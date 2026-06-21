#pragma once
#include <Arduino.h>
#include "esp_sleep.h"
#include "driver/gpio.h"
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
    // Hold VBAT_PWR_EN HIGH so the board 3.3V rail (and FT6336U) stays powered
    // during deep sleep — required for touch-to-wake to work
    gpio_hold_en((gpio_num_t)VBAT_PWR_EN);

    uint64_t wakePins = (1ULL << TOUCH_INT) | (1ULL << PWR_BTN);
    esp_sleep_enable_ext1_wakeup(wakePins, ESP_EXT1_WAKEUP_ANY_LOW);

    if (timerWakeupUs > 0) {
        esp_sleep_enable_timer_wakeup(timerWakeupUs);
    }

    esp_deep_sleep_start();
}
