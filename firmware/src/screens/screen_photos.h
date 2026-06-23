#pragma once
#include "screen_menu.h"
#include "saved_state.h"
#include <LittleFS.h>

namespace photos {

#define MAX_PHOTOS 100

static String _files[MAX_PHOTOS];
static int    _count = 0;
static int    _index = 0;

static void _scan()
{
    _count = 0;
    File root = LittleFS.open("/");
    File f = root.openNextFile();
    while (f && _count < MAX_PHOTOS) {
        if (!f.isDirectory()) {
            String name = f.name();
            if (name.endsWith(".bin"))
                _files[_count++] = name.startsWith("/") ? name : "/" + name;
        }
        f = root.openNextFile();
    }
}

static void _showCurrent(WaveshareEPD& epd)
{
    epd.clearBuffer();
    if (_count == 0) {
        epd.setTextColor(0);
        epd.setCursor(20, 90);  epd.print("No photos found.");
        epd.setCursor(20, 106); epd.print("Convert & upload");
        epd.setCursor(20, 122); epd.print("via photo_bin/");
    } else {
        File f = LittleFS.open(_files[_index]);
        if (f && f.size() >= 5000) {
            uint8_t tmp[5000];
            f.read(tmp, 5000);
            epd.loadBuffer(tmp, 5000);
            f.close();
        }
    }
    epd.display();
}

int getIndex() { return _index; }

void screenInit(WaveshareEPD& epd)
{
    _scan();
    _index = (_savedState.photoIndex < _count) ? _savedState.photoIndex : 0;
    _showCurrent(epd);
}

bool screenUpdate(WaveshareEPD& epd, TouchResult tr)
{
    if (tr.event == TOUCH_NONE) return false;
    if (tr.event == SWIPE_DOWN) return true;

    if (tr.event == SWIPE_LEFT) {
        _index = (_index + 1) % max(_count, 1);
    } else if (tr.event == SWIPE_RIGHT) {
        _index = (_index - 1 + _count) % max(_count, 1);
    } else if (tr.event == TOUCH_TAP) {
        if (tr.x < 100 && tr.y < 100)   return true;
        if (tr.x >= 100 && tr.y < 100)  _index = random(_count);
        if (tr.x < 100  && tr.y >= 100) _index = (_index - 1 + _count) % max(_count, 1);
        if (tr.x >= 100 && tr.y >= 100) _index = (_index + 1) % max(_count, 1);
    } else {
        return false;
    }

    _showCurrent(epd);
    return false;
}

} // namespace photos
