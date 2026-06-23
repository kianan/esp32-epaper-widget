#pragma once
#include "menu_logic.h"

// Single source of truth for deep sleep restore state.
// All fields are written exclusively in _saveAndSleep() in main.cpp.
RTC_DATA_ATTR struct {
    Screen screen;
    int    photoIndex;
    // to refactor if more screens use RTC_DATA_ATTR
} _savedState;
