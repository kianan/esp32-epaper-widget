#pragma once
#include "WaveshareEPD.h"
#include "touch.h"
#include "audio.h"
#include <Fonts/FreeSansBold9pt7b.h>

namespace audioScreen {

#define AUDIO_MID_X 100
#define AUDIO_MID_Y 100

enum RecState { IDLE, RECORDING, RECORDED };
static RecState _state = IDLE;

static void _draw(WaveshareEPD& epd)
{
    epd.clearBuffer();
    epd.setFont(&FreeSansBold9pt7b);
    epd.setTextColor(0);

    epd.drawLine(0,          AUDIO_MID_Y, 200, AUDIO_MID_Y, 0);
    epd.drawLine(AUDIO_MID_X, 0,          AUDIO_MID_X, 200, 0);

    epd.setCursor(32, 57);  epd.print("1-Up");
    epd.setCursor(127, 57); epd.print("Eh Eh");

    if (_state == RECORDING) {
        epd.setCursor(5, 150); epd.print("Recording");
    } else {
        epd.setCursor(22, 141); epd.print("Record");
        epd.setCursor(41, 159); epd.print("5s");
    }

    epd.setFont(&FreeSansBold9pt7b);
    epd.setCursor(114, 150); epd.print("Playback");

    epd.setFont(NULL); epd.setTextSize(1);
    epd.setCursor(5, 194); epd.print("Swipe right: back");

    epd.displayBase();
}

void screenInit(WaveshareEPD& epd)
{
    _state = IDLE;
    audioInit();
    _draw(epd);
}

bool screenUpdate(WaveshareEPD& epd, TouchResult tr)
{
    if (tr.event == SWIPE_RIGHT) return true;

    if (tr.event == TOUCH_TAP) {
        bool left = (tr.x < AUDIO_MID_X);
        bool top  = (tr.y < AUDIO_MID_Y);

        if (top && left) {
            audioPlayDing();
        } else if (top && !left) {
            audioPlayEhEh();
        } else if (!top && left) {
            if (audioCanRecord()) {
                _state = RECORDING;
                _draw(epd);
                audioRecord();
                _state = RECORDED;
                _draw(epd);
            }
        } else {
            if (_state == RECORDED) audioPlayRecording();
        }
    }
    return false;
}

} // namespace audioScreen
