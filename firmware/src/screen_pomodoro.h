#pragma once
#include "screen_menu.h"
#include <Fonts/FreeMonoBold24pt7b.h>
#include <Fonts/FreeSansBold9pt7b.h>
#include <Fonts/FreeSans9pt7b.h>

#define POMO_WORK_SECS  (25 * 60)
#define POMO_BREAK_SECS (5 * 60)

// Timer digits area (for partial refresh)
#define TIMER_Y1  68
#define TIMER_Y2  120
#define TIMER_H   (TIMER_Y2 - TIMER_Y1)

// Bottom strip: tap = back to menu
#define MENU_STRIP_Y  150

enum PomoState { POMO_IDLE, POMO_WORK, POMO_PAUSED, POMO_BREAK, POMO_DONE };

// RTC_DATA_ATTR survives deep sleep; others reset on wake (correct behaviour)
RTC_DATA_ATTR PomoState _pomoState    = POMO_IDLE;
RTC_DATA_ATTR int       _pomoSecsLeft = POMO_WORK_SECS;
RTC_DATA_ATTR int       _pomoSession  = 0;
static unsigned long    _pomoLastTick = 0;       // millis() resets on wake, so not RTC
static bool             _pomoPartialReady = false; // always needs full redraw after wake

void _drawTimerDigits(WaveshareEPD& epd) {
    char buf[6];
    snprintf(buf, sizeof(buf), "%02d:%02d", _pomoSecsLeft / 60, _pomoSecsLeft % 60);
    // Clear only the digits area then redraw
    epd.fillRect(0, TIMER_Y1, 200, TIMER_H, 1); // white
    epd.setFont(&FreeMonoBold24pt7b);
    epd.setTextColor(0);
    epd.setCursor(14, TIMER_Y2 - 8);
    epd.print(buf);
}

void _drawPomoFull(WaveshareEPD& epd) {
    epd.clearBuffer();
    epd.setTextColor(0);

    // State label
    epd.setFont(&FreeSansBold9pt7b);
    epd.setCursor(5, 25);
    switch (_pomoState) {
        case POMO_IDLE:   epd.print("Ready"); break;
        case POMO_WORK:   epd.print("FOCUS"); break;
        case POMO_PAUSED: epd.print("PAUSED"); break;
        case POMO_BREAK:  epd.print("BREAK"); break;
        case POMO_DONE:   epd.print("Done!"); break;
    }

    // Timer digits
    epd.setFont(&FreeMonoBold24pt7b);
    char buf[6];
    snprintf(buf, sizeof(buf), "%02d:%02d", _pomoSecsLeft / 60, _pomoSecsLeft % 60);
    epd.setCursor(14, TIMER_Y2 - 8);
    epd.print(buf);

    // Session count
    epd.setFont(&FreeSans9pt7b);
    epd.setCursor(10, 148);
    epd.printf("Sessions: %d", _pomoSession);

    // Hint below timer
    epd.setCursor(10, 165);
    if (_pomoState == POMO_IDLE) epd.print("Tap to start");
    else if (_pomoState == POMO_WORK) epd.print("Tap to pause");
    else if (_pomoState == POMO_DONE) epd.print("Tap to reset");

    // Menu strip separator + label
    epd.drawLine(0, MENU_STRIP_Y, 200, MENU_STRIP_Y, 0);
    epd.setCursor(55, 190);
    epd.print("< MENU");

    if (_pomoPartialReady) {
        epd.displayPartial();
    } else {
        epd.displayBase();
        epd.initPartial();
        _pomoPartialReady = true;
    }
}

bool pomoIsRunning() {
    return _pomoState == POMO_WORK || _pomoState == POMO_BREAK;
}

uint64_t pomoSleepTimerUs() {
    return pomoIsRunning() ? (uint64_t)_pomoSecsLeft * 1000000ULL : 0;
}

// Called on deep sleep timer wake — advance state as if timer expired
void pomoOnTimerWake() {
    _pomoLastTick     = 0;
    _pomoPartialReady = false;
    if (_pomoState == POMO_WORK) {
        _pomoSession++;
        _pomoState    = POMO_BREAK;
        _pomoSecsLeft = POMO_BREAK_SECS;
    } else if (_pomoState == POMO_BREAK) {
        _pomoState    = POMO_DONE;
        _pomoSecsLeft = 0;
    }
}

// Called on touch/button wake — subtract elapsed RTC seconds
void pomoResumeAfterSleep(int elapsedSecs) {
    _pomoLastTick     = 0;
    _pomoPartialReady = false;
    if (!pomoIsRunning()) return;
    _pomoSecsLeft -= elapsedSecs;
    if (_pomoSecsLeft <= 0) pomoOnTimerWake();
}

void pomoInit(WaveshareEPD& epd) {
    _pomoState         = POMO_IDLE;
    _pomoSecsLeft      = POMO_WORK_SECS;
    _pomoLastTick      = 0;
    _pomoPartialReady  = false;
    _drawPomoFull(epd);
}

// Returns true if should go back to menu
bool updatePomodoro(WaveshareEPD& epd, TouchResult tr) {
    bool touched = (tr.event == TOUCH_TAP);
    bool backGesture = (tr.event == SWIPE_DOWN) ||
                       (touched && tr.y >= MENU_STRIP_Y);

    if (backGesture) {
        if (_pomoState == POMO_WORK) _pomoState = POMO_PAUSED;
        _pomoPartialReady = false;
        return true;
    }

    if (touched && tr.y < MENU_STRIP_Y) {
        // Tap on main area = start/pause/reset
        bool changed = true;
        switch (_pomoState) {
            case POMO_IDLE:
                _pomoState    = POMO_WORK;
                _pomoSecsLeft = POMO_WORK_SECS;
                _pomoLastTick = millis();
                _drawPomoFull(epd);
                changed = false; // already drew
                break;
            case POMO_WORK:
                _pomoState = POMO_PAUSED;
                break;
            case POMO_PAUSED:
                _pomoState    = POMO_WORK;
                _pomoLastTick = millis();
                break;
            case POMO_DONE:
                _pomoState    = POMO_IDLE;
                _pomoSecsLeft = POMO_WORK_SECS;
                break;
            default: changed = false;
        }
        if (changed) _drawPomoFull(epd);
    }

    // Countdown tick
    if (_pomoState == POMO_WORK || _pomoState == POMO_BREAK) {
        unsigned long now = millis();
        if (now - _pomoLastTick >= 1000) {
            _pomoLastTick = now;
            _pomoSecsLeft--;

            if (_pomoSecsLeft <= 0) {
                if (_pomoState == POMO_WORK) {
                    _pomoSession++;
                    _pomoState    = POMO_BREAK;
                    _pomoSecsLeft = POMO_BREAK_SECS;
                } else {
                    _pomoState    = POMO_DONE;
                    _pomoSecsLeft = 0;
                }
                _drawPomoFull(epd);
            } else if (_pomoPartialReady) {
                // Partial refresh: only update the digits area
                _drawTimerDigits(epd);
                epd.displayPartial();
            }
        }
    }

    return false;
}
