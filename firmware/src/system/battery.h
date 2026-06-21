#pragma once
#include "pins.h"

// Battery voltage divider is ×2 (two equal resistors to GND)
// analogReadMilliVolts uses factory ADC calibration for accuracy
int batteryPercent() {
    uint32_t mv = analogReadMilliVolts(BAT_ADC);
    float vbat = mv * 2.0f / 1000.0f; // actual battery voltage

    // LiPo usable range: 3.3V (0%) – 4.2V (100%)
    int pct = (int)((vbat - 3.3f) / (4.2f - 3.3f) * 100.0f);
    if (pct < 0)   pct = 0;
    if (pct > 100) pct = 100;
    return pct;
}
