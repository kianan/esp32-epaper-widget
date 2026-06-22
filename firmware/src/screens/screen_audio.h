#pragma once
#include "WaveshareEPD.h"
#include "touch.h"
#include "audio.h"
#include <Fonts/FreeSansBold9pt7b.h>

#define S7_MID_X  100
#define S7_MID_Y  100

enum S7State { S7_IDLE, S7_RECORDING, S7_RECORDED };
static S7State _s7State = S7_IDLE;

static void _drawAudioScreen(WaveshareEPD& epd)
{
    epd.clearBuffer();
    epd.setFont(&FreeSansBold9pt7b);
    epd.setTextColor(0);

    // Divider lines
    epd.drawLine(0,        S7_MID_Y, 200, S7_MID_Y, 0);
    epd.drawLine(S7_MID_X, 0,        S7_MID_X, 200, 0);

    // Top-left: "1-Up" — ~36px wide, centred in [0,100]
    epd.setCursor(32, 57); epd.print("1-Up");

    // Top-right: "Eh Eh" — ~46px wide, centred in [100,200]
    epd.setCursor(127, 57); epd.print("Eh Eh");

    // Bottom-left: "Record" + "5s", two lines centred; or "Recording..." when active
    if (_s7State == S7_RECORDING) {
        // "Recording..." ~90px wide in bold, centred in [0,100]
        epd.setCursor(5, 150); epd.print("Recording");
    } else {
        // "Record" ~55px wide, centred in [0,100]
        epd.setCursor(22, 141); epd.print("Record");
        // "5s" ~18px wide, centred in [0,100]
        epd.setCursor(41, 159); epd.print("5s");
    }

    // Bottom-right: "Playback" — ~71px wide, centred in [100,200]
    epd.setFont(&FreeSansBold9pt7b);
    epd.setCursor(114, 150); epd.print("Playback");

    epd.setFont(NULL); epd.setTextSize(1);
    epd.setCursor(5, 194); epd.print("Swipe right: back");

    epd.displayBase();
}

void screen7Init(WaveshareEPD& epd)
{
    _s7State = S7_IDLE;
    audioInit();
    _drawAudioScreen(epd);
}

bool updateScreen7(WaveshareEPD& epd, TouchResult tr)
{
    if (tr.event == SWIPE_RIGHT) return true;

    if (tr.event == TOUCH_TAP) {
        bool left = (tr.x < S7_MID_X);
        bool top  = (tr.y < S7_MID_Y);

        if (top && left) {
            audioPlayDing();
        } else if (top && !left) {
            audioPlayEhEh();
        } else if (!top && left) {
            if (audioCanRecord()) {
                _s7State = S7_RECORDING;
                _drawAudioScreen(epd);
                audioRecord();
                _s7State = S7_RECORDED;
                _drawAudioScreen(epd);
            }
        } else {
            if (_s7State == S7_RECORDED) audioPlayRecording();
        }
    }
    return false;
}
