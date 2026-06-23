#pragma once
#include "screen_menu.h"
#include "sleep.h"
#include <Fonts/FreeMonoBold24pt7b.h>
#include <Fonts/FreeSansBold9pt7b.h>
#include <Fonts/FreeSans9pt7b.h>

namespace pomodoro {

#define POMO_WORK_SECS  (25 * 60)
#define POMO_BREAK_SECS (5 * 60)
#define TIMER_Y1  68
#define TIMER_Y2  120
#define TIMER_H   (TIMER_Y2 - TIMER_Y1)

enum PomoState { POMO_IDLE, POMO_WORK, POMO_PAUSED, POMO_BREAK, POMO_DONE };

static PomoState     _state        = POMO_IDLE;
static int           _secsLeft     = POMO_WORK_SECS;
static int           _session      = 0;
static unsigned long _lastTick     = 0;
static bool          _partialReady = false;

static void _drawDigits(WaveshareEPD& epd)
{
    char buf[6];
    snprintf(buf, sizeof(buf), "%02d:%02d", _secsLeft / 60, _secsLeft % 60);
    epd.fillRect(0, TIMER_Y1, 200, TIMER_H, 1);
    epd.setFont(&FreeMonoBold24pt7b);
    epd.setTextColor(0);
    epd.setCursor(14, TIMER_Y2 - 8);
    epd.print(buf);
}

static void _drawFull(WaveshareEPD& epd)
{
    epd.clearBuffer();
    epd.setTextColor(0);

    epd.setFont(&FreeSansBold9pt7b);
    epd.setCursor(5, 25);
    switch (_state) {
        case POMO_IDLE:   epd.print("Ready");  break;
        case POMO_WORK:   epd.print("FOCUS");  break;
        case POMO_PAUSED: epd.print("PAUSED"); break;
        case POMO_BREAK:  epd.print("BREAK");  break;
        case POMO_DONE:   epd.print("Done!");  break;
    }

    epd.setFont(&FreeMonoBold24pt7b);
    char buf[6];
    snprintf(buf, sizeof(buf), "%02d:%02d", _secsLeft / 60, _secsLeft % 60);
    epd.setCursor(14, TIMER_Y2 - 8);
    epd.print(buf);

    epd.setFont(&FreeSans9pt7b);
    epd.setCursor(10, 148);
    epd.printf("Sessions: %d", _session);

    epd.setCursor(10, 165);
    if (_state == POMO_IDLE) epd.print("Tap to start");
    else if (_state == POMO_WORK) epd.print("Tap to pause");
    else if (_state == POMO_DONE) epd.print("Tap to reset");

    epd.setFont(NULL); epd.setTextSize(1);
    epd.setCursor(5, 194); epd.print("Swipe right: back");

    if (_partialReady) {
        epd.displayPartial();
    } else {
        epd.displayBase();
        epd.initPartial();
        _partialReady = true;
    }
}

void screenInit(WaveshareEPD& epd)
{
    _state        = POMO_IDLE;
    _secsLeft     = POMO_WORK_SECS;
    _lastTick     = 0;
    _partialReady = false;
    _drawFull(epd);
}

bool screenUpdate(WaveshareEPD& epd, TouchResult tr)
{
    if (tr.event == SWIPE_RIGHT) {
        if (_state == POMO_WORK) _state = POMO_PAUSED;
        _partialReady = false;
        return true;
    }

    if (tr.event == TOUCH_TAP) {
        bool changed = true;
        switch (_state) {
            case POMO_IDLE:
                _state    = POMO_WORK;
                _secsLeft = POMO_WORK_SECS;
                _lastTick = millis();
                _drawFull(epd);
                changed = false;
                break;
            case POMO_WORK:   _state = POMO_PAUSED; break;
            case POMO_PAUSED: _state = POMO_WORK; _lastTick = millis(); break;
            case POMO_DONE:   _state = POMO_IDLE; _secsLeft = POMO_WORK_SECS; break;
            default: changed = false;
        }
        if (changed) _drawFull(epd);
    }

    if (_state == POMO_WORK || _state == POMO_BREAK) {
        unsigned long now = millis();
        if (now - _lastTick >= 1000) {
            _lastTick = now;
            _secsLeft--;
            activityPing();

            if (_secsLeft <= 0) {
                if (_state == POMO_WORK) {
                    _session++;
                    _state    = POMO_BREAK;
                    _secsLeft = POMO_BREAK_SECS;
                } else {
                    _state    = POMO_DONE;
                    _secsLeft = 0;
                }
                _drawFull(epd);
            } else if (_partialReady) {
                _drawDigits(epd);
                epd.displayPartial();
            }
        }
    }

    return false;
}

} // namespace pomodoro
