#include <Arduino.h>
#include <Wire.h>
#include <LittleFS.h>

#include "pins.h"
#include "touch.h"
#include "screen_menu.h"
#include "screen_clock.h"
#include "screen_pomodoro.h"
#include "screen_photos.h"
#include "screen_jessie.h"
#include "pcf85063.h"

WaveshareEPD epd(EPD_CS, EPD_DC, EPD_RST, EPD_BUSY, EPD_SCLK, EPD_SDI);

Screen currentScreen = SCREEN_MENU;

void setup() {
    Serial.begin(115200);

    pinMode(BOOT_BTN, INPUT_PULLUP);

    // Enable battery power rail so board runs without USB
    pinMode(VBAT_PWR_EN, OUTPUT);
    digitalWrite(VBAT_PWR_EN, HIGH);

    // Keep audio amp off (not used yet)
    pinMode(AUDIO_PWR_EN, OUTPUT);
    digitalWrite(AUDIO_PWR_EN, LOW);

    // EPD power enable is active LOW (GPIO6 LOW = display on)
    pinMode(EPD_3V3_EN, OUTPUT);
    digitalWrite(EPD_3V3_EN, LOW);
    delay(50);

    epd.init();

    LittleFS.begin(true); // true = format on fail

    Wire.begin(TOUCH_SDA, TOUCH_SCL);
    delay(200);
    touchInit();

    drawMenu(epd);
}

void loop() {
    if (Serial.available()) {
        String cmd = Serial.readStringUntil('\n');
        cmd.trim();
        if (cmd == "SCREENSHOT") {
            epd.dumpBuffer();
        } else if (cmd.startsWith("SET_TIME ")) {
            // SET_TIME HH:MM
            String t = cmd.substring(9);
            int h = t.substring(0, 2).toInt();
            int m = t.substring(3, 5).toInt();
            if (h >= 0 && h < 24 && m >= 0 && m < 60) {
                pcf85063Write(h, m);
                Serial.println("OK");
            } else {
                Serial.println("ERR: use SET_TIME HH:MM");
            }
        }
    }

    TouchResult tr = readTouch();

    switch (currentScreen) {
        case SCREEN_MENU: {
            Screen next = menuHandleTouch(tr);
            if (next != SCREEN_MENU) {
                currentScreen = next;
                if (currentScreen == SCREEN_CLOCK)    updateClock(epd, {TOUCH_NONE});
                if (currentScreen == SCREEN_POMODORO) pomoInit(epd);
                if (currentScreen == SCREEN_PHOTOS)   photosInit(epd);
                if (currentScreen == SCREEN_JESSIE)   drawJessie(epd);
            }
            break;
        }

        case SCREEN_CLOCK:
            if (updateClock(epd, tr)) {
                currentScreen = SCREEN_MENU;
                drawMenu(epd);
            }
            break;

        case SCREEN_POMODORO:
            if (updatePomodoro(epd, tr)) {
                currentScreen = SCREEN_MENU;
                drawMenu(epd);
            }
            break;

        case SCREEN_PHOTOS:
            if (updatePhotos(epd, tr)) {
                currentScreen = SCREEN_MENU;
                drawMenu(epd);
            }
            break;

        case SCREEN_JESSIE:
            if (updateJessie(epd, tr)) {
                currentScreen = SCREEN_MENU;
                drawMenu(epd);
            }
            break;
    }

    delay(50);
}
