#pragma once
#include <Wire.h>
#include "pins.h"

#define FT6336_ADDR      0x38
#define FT6336_TD_STATUS 0x02
#define SWIPE_MIN        40   // pixels to qualify as swipe

enum TouchEvent { TOUCH_NONE, TOUCH_TAP, SWIPE_LEFT, SWIPE_RIGHT, SWIPE_UP, SWIPE_DOWN };

struct TouchResult {
    TouchEvent event;
    int x;  // valid for TOUCH_TAP only
    int y;
};

static enum { TS_IDLE, TS_PRESSING } _tsState = TS_IDLE;
static int   _tsStartX, _tsStartY, _tsCurrX, _tsCurrY;
static unsigned long _tsCooldown = 0;

void touchInit() {
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
            _tsStartX = _tsCurrX = x;
            _tsStartY = _tsCurrY = y;
            _tsState  = TS_PRESSING;
        } else {
            _tsCurrX = x;
            _tsCurrY = y;
        }
    } else if (_tsState == TS_PRESSING) {
        _tsState     = TS_IDLE;
        _tsCooldown  = millis() + 200;

        int dx = _tsCurrX - _tsStartX;
        int dy = _tsCurrY - _tsStartY;
        int adx = abs(dx), ady = abs(dy);

        if (adx < SWIPE_MIN && ady < SWIPE_MIN) {
            tr = {TOUCH_TAP, _tsStartX, _tsStartY};
        } else if (adx >= ady) {
            tr.event = (dx > 0) ? SWIPE_RIGHT : SWIPE_LEFT;
        } else {
            tr.event = (dy > 0) ? SWIPE_DOWN : SWIPE_UP;
        }
    }

    return tr;
}
