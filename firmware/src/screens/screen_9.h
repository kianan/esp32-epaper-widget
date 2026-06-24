#pragma once
#include "screen_menu.h"
#include "audio.h"
#include "touch.h"
#include "pins.h"
#include "sleep.h"
#include <SD_MMC.h>
#include <vector>
#include <algorithm>
#include <Fonts/FreeSansBold9pt7b.h>

namespace screen9 {

static const int TOP_H    = 55;
static const int STATUS_Y = 175;
static const int VISIBLE  = 4;
static const int LIST_Y0  = 75;   // baseline of first list item
static const int LIST_DY  = 25;   // px between items

enum VoiceState { VS_IDLE, VS_RECORDING, VS_PLAYING };
static VoiceState        _state      = VS_IDLE;
static bool              _listActive = false;
static int               _selected   = 0;
static int               _scrollTop  = 0;
static std::vector<String> _files;
static unsigned long     _pressStart = 0;
static bool              _longFired  = false;

static void _buildWavHeader(uint8_t* h, uint32_t dataBytes)
{
    uint32_t fileSize = dataBytes + 36;
    uint32_t byteRate = 16000 * 2 * 2;
    memcpy(h,    "RIFF", 4); memcpy(h+4,  &fileSize, 4);
    memcpy(h+8,  "WAVE", 4); memcpy(h+12, "fmt ",    4);
    *(uint32_t*)(h+16) = 16;  *(uint16_t*)(h+20) = 1;
    *(uint16_t*)(h+22) = 2;   *(uint32_t*)(h+24) = 16000;
    *(uint32_t*)(h+28) = byteRate; *(uint16_t*)(h+32) = 4;
    *(uint16_t*)(h+34) = 16;
    memcpy(h+36, "data", 4); *(uint32_t*)(h+40) = dataBytes;
}

static bool _sdMount()
{
    SD_MMC.setPins(TF_CLK, TF_CMD, TF_D0);
    return SD_MMC.begin("/sdcard", true);
}

static void _loadFiles()
{
    _files.clear();
    if (!_sdMount()) return;
    File root = SD_MMC.open("/");
    File entry = root.openNextFile();
    while (entry) {
        if (!entry.isDirectory()) {
            String n = entry.name();
            if (n.startsWith("REC_") && n.endsWith(".WAV"))
                _files.push_back(n);
        }
        entry = root.openNextFile();
    }
    root.close();
    SD_MMC.end();
    std::sort(_files.begin(), _files.end(),
              [](const String& a, const String& b) { return a > b; });
}

static String _nextFilename()
{
    int maxN = 0;
    for (auto& n : _files) {
        int v = n.substring(4, 7).toInt();
        if (v > maxN) maxN = v;
    }
    char buf[16];
    snprintf(buf, sizeof(buf), "REC_%03d.WAV", maxN + 1);
    return String(buf);
}

static void _clampScroll()
{
    if (_selected < _scrollTop)             _scrollTop = _selected;
    if (_selected >= _scrollTop + VISIBLE)  _scrollTop = _selected - VISIBLE + 1;
    if (_scrollTop < 0)                     _scrollTop = 0;
}

static void _draw(WaveshareEPD& epd)
{
    epd.clearBuffer();
    epd.setFont(&FreeSansBold9pt7b);
    epd.setTextColor(0);

    // Top panel
    int16_t bx, by; uint16_t bw, bh;
    int midY = TOP_H / 2;

    if (_state == VS_RECORDING) {
        const char* txt = "Recording...";
        epd.getTextBounds(txt, 0, 0, &bx, &by, &bw, &bh);
        const int cr = 4, gap = 6;
        int startX = (200 - (int)(cr * 2 + gap + bw)) / 2;
        epd.fillCircle(startX + cr, midY, cr, 0);
        epd.setCursor(startX + cr * 2 + gap - bx, midY - (int)bh / 2 - by);
        epd.print(txt);
    } else {
        const char* txt = "Tap to record";
        epd.getTextBounds(txt, 0, 0, &bx, &by, &bw, &bh);
        epd.setCursor((200 - (int)bw) / 2 - bx, midY - (int)bh / 2 - by);
        epd.print(txt);
    }

    // Dividers
    epd.drawLine(0, TOP_H,    200, TOP_H,    0);
    epd.drawLine(0, STATUS_Y, 200, STATUS_Y, 0);

    // List
    if (_files.empty()) {
        epd.setFont(NULL); epd.setTextSize(1);
        const char* msg = "No recordings";
        epd.setCursor((200 - (int)strlen(msg) * 6) / 2, (TOP_H + STATUS_Y) / 2 - 4);
        epd.print(msg);
        epd.setFont(&FreeSansBold9pt7b);
    } else {
        for (int i = 0; i < VISIBLE && (_scrollTop + i) < (int)_files.size(); i++) {
            int fi = _scrollTop + i;
            bool sel = _listActive && (fi == _selected);
            String line = (sel ? "> " : "  ") + _files[fi];
            epd.setCursor(4, LIST_Y0 + i * LIST_DY);
            epd.print(line);
        }
    }

    // Status strip
    epd.setFont(NULL); epd.setTextSize(1);
    if (_state == VS_PLAYING) {
        epd.setCursor(4, STATUS_Y + 12);
        epd.print("Playing...  tap to stop");
    } else if (_listActive && !_files.empty()) {
        epd.setCursor(4, STATUS_Y + 12);
        epd.print("tap=play  long tap=del");
    }
}

static void _doRecord(WaveshareEPD& epd)
{
    if (!_sdMount()) return;
    String fname = "/" + _nextFilename();
    File f = SD_MMC.open(fname, FILE_WRITE);
    if (!f) { SD_MMC.end(); return; }

    uint8_t hdr[44] = {};
    f.write(hdr, 44);

    static uint8_t chunk[4096];
    uint32_t total = 0;

    while (true) {
        audioReadChunk(chunk, sizeof(chunk));
        f.write(chunk, sizeof(chunk));
        total += sizeof(chunk);
        activityPing();
        TouchResult tr = readTouch();
        if (tr.event == TOUCH_TAP && tr.y < TOP_H) break;
    }

    f.seek(0);
    _buildWavHeader(hdr, total);
    f.write(hdr, 44);
    f.close();
    SD_MMC.end();

    _loadFiles();
    _selected   = 0;
    _scrollTop  = 0;
    _listActive = true;
}

static void _doPlay()
{
    if (_selected >= (int)_files.size()) return;
    if (!_sdMount()) return;
    File f = SD_MMC.open("/" + _files[_selected]);
    if (!f) { SD_MMC.end(); return; }
    f.seek(44);

    static uint8_t buf[4096];
    while (f.available() > 0) {
        int n = f.read(buf, sizeof(buf));
        if (n <= 0) break;
        audioWriteChunk(buf, n);
        TouchResult tr = readTouch();
        if (tr.event == TOUCH_TAP) break;
    }
    f.close();
    SD_MMC.end();
}

static void _doDelete()
{
    if (_selected >= (int)_files.size()) return;
    if (!_sdMount()) return;
    SD_MMC.remove("/" + _files[_selected]);
    SD_MMC.end();
    _loadFiles();
    if (_selected >= (int)_files.size()) _selected = (int)_files.size() - 1;
    if (_selected < 0) _selected = 0;
    _clampScroll();
}

void screenInit(WaveshareEPD& epd)
{
    _state      = VS_IDLE;
    _listActive = false;
    _selected   = 0;
    _scrollTop  = 0;
    _pressStart = 0;
    _longFired  = false;
    audioInit();
    _loadFiles();
    _draw(epd);
    epd.displayBase();
    epd.initPartial();
}

bool screenUpdate(WaveshareEPD& epd, TouchResult tr)
{
    bool active = touchActive();

    // Track press start for long press
    if (active && _pressStart == 0) {
        _pressStart = millis();
        _longFired  = false;
    }
    if (!active) _pressStart = 0;

    // Long press fires during TOUCH_NONE (finger held)
    if (tr.event == TOUCH_NONE) {
        if (active && !_longFired && _pressStart > 0 && millis() - _pressStart >= 3000) {
            _longFired = true;
            int ly = touchStartY();
            if (_state == VS_IDLE && _listActive && !_files.empty()
                    && ly >= TOP_H && ly < STATUS_Y) {
                _doDelete();
                _draw(epd); epd.displayPartial();
            }
        }
        return false;
    }

    // Consume the TAP that ends a long press
    if (tr.event == TOUCH_TAP && _longFired) {
        _longFired = false;
        return false;
    }
    _longFired = false;

    if (tr.event == SWIPE_RIGHT && _state == VS_IDLE) return true;

    if (tr.event == TOUCH_TAP) {
        bool inTop  = tr.y < TOP_H;
        bool inList = tr.y >= TOP_H && tr.y < STATUS_Y;

        if (inTop && _state == VS_IDLE) {
            _listActive = false;
            _state = VS_RECORDING;
            _draw(epd); epd.displayPartial();
            _doRecord(epd);
            _state = VS_IDLE;
            _draw(epd); epd.displayPartial();

        } else if (inList && _state == VS_IDLE && !_files.empty()) {
            if (!_listActive) {
                _listActive = true;
                _draw(epd); epd.displayPartial();
            } else {
                _state = VS_PLAYING;
                _draw(epd); epd.displayPartial();
                _doPlay();
                _state = VS_IDLE;
                _draw(epd); epd.displayPartial();
            }
        }
    } else if (tr.event == SWIPE_UP && _listActive && _state == VS_IDLE) {
        if (_selected > 0) {
            _selected--;
            _clampScroll();
            _draw(epd); epd.displayPartial();
        }
    } else if (tr.event == SWIPE_DOWN && _listActive && _state == VS_IDLE) {
        if (_selected < (int)_files.size() - 1) {
            _selected++;
            _clampScroll();
            _draw(epd); epd.displayPartial();
        }
    }

    return false;
}

} // namespace screen9
