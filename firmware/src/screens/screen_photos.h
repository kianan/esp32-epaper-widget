#pragma once
#include "screen_menu.h"
#include <LittleFS.h>

#define MAX_PHOTOS   100

static String _photoFiles[MAX_PHOTOS];
static int    _photoCount = 0;
static int    _photoIndex = 0;

void _scanPhotos() {
    _photoCount = 0;
    File root = LittleFS.open("/");
    File f = root.openNextFile();
    while (f && _photoCount < MAX_PHOTOS) {
        if (!f.isDirectory()) {
            String name = f.name();
            if (name.endsWith(".bin")) {
                // LittleFS may or may not include leading slash
                _photoFiles[_photoCount++] = name.startsWith("/") ? name : "/" + name;
            }
        }
        f = root.openNextFile();
    }
}

void _showCurrentPhoto(WaveshareEPD& epd) {
    epd.clearBuffer();

    if (_photoCount == 0) {
        epd.setTextColor(0);
        epd.setCursor(20, 90);  epd.print("No photos found.");
        epd.setCursor(20, 106); epd.print("Convert & upload");
        epd.setCursor(20, 122); epd.print("via photo_bin/");
    } else {
        File f = LittleFS.open(_photoFiles[_photoIndex]);
        if (f && f.size() >= 5000) {
            uint8_t tmp[5000];
            f.read(tmp, 5000);
            epd.loadBuffer(tmp, 5000);
            f.close();
        }
    }

    epd.display();
}

void photosInit(WaveshareEPD& epd) {
    _scanPhotos();
    _photoIndex = 0;
    _showCurrentPhoto(epd);
}

// Returns true = back to menu
// Swipe left/right = next/prev  Swipe down = back to menu
// Tap quadrants: TL=back, TR=random, BL=prev, BR=next
bool updatePhotos(WaveshareEPD& epd, TouchResult tr) {
    if (tr.event == TOUCH_NONE) return false;

    if (tr.event == SWIPE_DOWN) return true;

    if (tr.event == SWIPE_LEFT) {
        _photoIndex = (_photoIndex + 1) % max(_photoCount, 1);
    } else if (tr.event == SWIPE_RIGHT) {
        _photoIndex = (_photoIndex - 1 + _photoCount) % max(_photoCount, 1);
    } else if (tr.event == TOUCH_TAP) {
        if (tr.x < 100 && tr.y < 100)   return true;
        if (tr.x >= 100 && tr.y < 100)  _photoIndex = random(_photoCount);
        if (tr.x < 100  && tr.y >= 100) _photoIndex = (_photoIndex - 1 + _photoCount) % max(_photoCount, 1);
        if (tr.x >= 100 && tr.y >= 100) _photoIndex = (_photoIndex + 1) % max(_photoCount, 1);
    } else {
        return false;
    }

    _showCurrentPhoto(epd);
    return false;
}
