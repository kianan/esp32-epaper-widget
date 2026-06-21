#pragma once
#include <Wire.h>
#include "pins.h"

#define FT6336_ADDR      0x38
#define FT6336_TD_STATUS 0x02
#define SWIPE_MIN_X      25   // horizontal swipe threshold (pixels)
#define SWIPE_MIN_Y      20   // vertical swipe threshold (less room on small screen)

enum TouchEvent { TOUCH_NONE, TOUCH_TAP, SWIPE_LEFT, SWIPE_RIGHT, SWIPE_UP, SWIPE_DOWN };

struct TouchResult {
    TouchEvent event;
    int x;  // valid for TOUCH_TAP only
    int y;
};

static enum { TS_IDLE, TS_PRESSING } _tsState = TS_IDLE;
static volatile bool _touchIntFired = false;

void IRAM_ATTR _touchISR() { _touchIntFired = true; }

bool touchActive() { return _tsState == TS_PRESSING; }
static int   _tsStartX, _tsStartY;
static int   _tsPeakDx, _tsPeakDy;   // max displacement seen during press
static unsigned long _tsCooldown = 0;

void touchInit() {
    pinMode(TOUCH_INT, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(TOUCH_INT), _touchISR, FALLING);
    pinMode(EPD_TP_RST, OUTPUT);
    digitalWrite(EPD_TP_RST, LOW);  delay(10);
    digitalWrite(EPD_TP_RST, HIGH); delay(100);
}

TouchResult readTouch() {
    TouchResult tr = {TOUCH_NONE, 0, 0};
    if (millis() < _tsCooldown) return tr;

    Wire.beginTransmission(FT6336_ADDR);
    Wire.write(FT6336_TD_STATUS);
    if (Wire.endTransmission(false) != 0) return tr;
    Wire.requestFrom((uint8_t)FT6336_ADDR, (uint8_t)5);
    if (Wire.available() < 5) return tr;

    uint8_t td = Wire.read();
    uint8_t xh = Wire.read(), xl = Wire.read();
    uint8_t yh = Wire.read(), yl = Wire.read();

    if (td > 0) {
        int x = ((xh & 0x0F) << 8) | xl;
        int y = ((yh & 0x0F) << 8) | yl;
        if (_tsState == TS_IDLE) {
            _tsStartX  = x;
            _tsStartY  = y;
            _tsPeakDx  = 0;
            _tsPeakDy  = 0;
            _tsState   = TS_PRESSING;
        } else {
            // Track furthest displacement seen in any direction
            int dx = x - _tsStartX;
            int dy = y - _tsStartY;
            if (abs(dx) > abs(_tsPeakDx)) _tsPeakDx = dx;
            if (abs(dy) > abs(_tsPeakDy)) _tsPeakDy = dy;
        }
    } else if (_tsState == TS_PRESSING) {
        _tsState    = TS_IDLE;
        _tsCooldown = millis() + 200;

        int adx = abs(_tsPeakDx), ady = abs(_tsPeakDy);
        bool xSwipe = adx >= SWIPE_MIN_X;
        bool ySwipe = ady >= SWIPE_MIN_Y;

        if (!xSwipe && !ySwipe) {
            tr = {TOUCH_TAP, _tsStartX, _tsStartY};
        } else if (adx >= ady) {
            tr.event = (_tsPeakDx > 0) ? SWIPE_RIGHT : SWIPE_LEFT;
        } else {
            tr.event = (_tsPeakDy > 0) ? SWIPE_DOWN : SWIPE_UP;
        }
    }

    return tr;
}
