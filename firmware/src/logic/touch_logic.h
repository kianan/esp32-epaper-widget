#pragma once

#define SWIPE_MIN_X 25
#define SWIPE_MIN_Y 20

enum TouchEvent { TOUCH_NONE, TOUCH_TAP, SWIPE_LEFT, SWIPE_RIGHT, SWIPE_UP, SWIPE_DOWN };

struct TouchResult { TouchEvent event; int x; int y; };

// Pure gesture classifier — no hardware dependency.
// peakDx/peakDy: maximum displacement seen during a press.
inline TouchEvent classifyGesture(int peakDx, int peakDy) {
    int adx = peakDx < 0 ? -peakDx : peakDx;
    int ady = peakDy < 0 ? -peakDy : peakDy;
    if (adx < SWIPE_MIN_X && ady < SWIPE_MIN_Y) return TOUCH_TAP;
    if (adx >= ady) return (peakDx > 0) ? SWIPE_RIGHT : SWIPE_LEFT;
    return (peakDy > 0) ? SWIPE_DOWN : SWIPE_UP;
}
