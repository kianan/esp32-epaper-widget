#pragma once
#include "screen_menu.h"
#include "pins.h"
#include <SD_MMC.h>
#include <Fonts/FreeSansBold9pt7b.h>

namespace screen8 {

static void _run(WaveshareEPD& epd)
{
    epd.clearBuffer();
    epd.setTextColor(0);

    epd.setFont(&FreeSansBold9pt7b);
    epd.setCursor(10, 18); epd.print("SD Card");
    epd.drawLine(0, 25, 200, 25, 0);

    epd.setFont(NULL); epd.setTextSize(1);
    int y = 36;
    auto println = [&](const char* s) { epd.setCursor(0, y); epd.print(s); y += 10; };
    auto printf2 = [&](const char* fmt, ...) {
        char buf[64]; va_list a; va_start(a, fmt); vsnprintf(buf, sizeof(buf), fmt, a); va_end(a);
        epd.setCursor(0, y); epd.print(buf); y += 10;
    };

    SD_MMC.setPins(TF_CLK, TF_CMD, TF_D0);
    if (!SD_MMC.begin("/sdcard", true)) {
        println("Mount: FAILED");
        println("Check card is inserted");
    } else {
        uint8_t t = SD_MMC.cardType();
        const char* tname = (t == CARD_MMC) ? "MMC" : (t == CARD_SD) ? "SD" :
                            (t == CARD_SDHC) ? "SDHC" : "UNKNOWN";
        printf2("Type: %s", tname);
        printf2("Size: %llu MB", SD_MMC.cardSize() / (1024 * 1024));
        printf2("Total: %llu MB", SD_MMC.totalBytes() / (1024 * 1024));
        printf2("Used:  %llu MB", SD_MMC.usedBytes() / (1024 * 1024));
        y += 2;

        // Write test
        bool wok = false;
        File f = SD_MMC.open("/sdtest.txt", FILE_WRITE);
        if (f) { wok = f.print("ok") > 0; f.close(); }
        printf2("Write: %s", wok ? "OK" : "FAIL");

        // Read test
        bool rok = false;
        f = SD_MMC.open("/sdtest.txt");
        if (f) { rok = f.readString() == "ok"; f.close(); }
        printf2("Read:  %s", rok ? "OK" : "FAIL");

        if (wok) SD_MMC.remove("/sdtest.txt");
        SD_MMC.end();
    }

    epd.setFont(&FreeSansBold9pt7b);
    epd.setCursor(0, 197); epd.print("Swipe right: back  Tap: retest");
}

void screenInit(WaveshareEPD& epd)
{
    _run(epd);
    epd.displayBase();
    epd.initPartial();
}

bool screenUpdate(WaveshareEPD& epd, TouchResult tr)
{
    if (tr.event == SWIPE_RIGHT) return true;
    if (tr.event == TOUCH_TAP) { _run(epd); epd.displayPartial(); }
    return false;
}

} // namespace screen8
