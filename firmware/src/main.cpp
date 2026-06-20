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
#include "screen_5.h"
#include "screen_6.h"
#include "screen_7.h"
#include "screen_8.h"
#include "rtc.h"
#include "sleep.h"

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

    LittleFS.begin(true);

    Wire.begin(TOUCH_SDA, TOUCH_SCL);
    delay(200);
    touchInit();

    // Handle deep sleep wakeup
    esp_sleep_wakeup_cause_t wakeup = esp_sleep_get_wakeup_cause();
    if (wakeup == ESP_SLEEP_WAKEUP_TIMER) {
        // Pomodoro timer expired during sleep — advance state and show screen
        pomoOnTimerWake();
        currentScreen = SCREEN_POMODORO;
        _drawPomoFull(epd);
    } else if (wakeup != ESP_SLEEP_WAKEUP_UNDEFINED) {
        // Touch or button woke device
        if (pomoIsRunning()) {
            int elapsed = sleepElapsedSecs(rtcSecsSinceMidnight());
            pomoResumeAfterSleep(elapsed);
            currentScreen = SCREEN_POMODORO;
            _drawPomoFull(epd);
        } else {
            drawMenu(epd);
        }
    } else {
        drawMenu(epd);
    }

    activityPing();
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
                rtcWrite(h, m);
                Serial.println("OK");
            } else {
                Serial.println("ERR: use SET_TIME HH:MM");
            }
        }
    }

    // Sleep on inactivity
    if (sleepTimeoutReached()) {
        enterDeepSleep(rtcSecsSinceMidnight(), pomoSleepTimerUs());
    }

    TouchResult tr = {TOUCH_NONE, 0, 0};
    bool wasActive = touchActive();
    if (_touchIntFired || wasActive) {
        if (_touchIntFired && !wasActive) {
#ifdef DEBUG_GESTURES
            Serial.println("[touch] polling started");
#endif
        }
        _touchIntFired = false;
        tr = readTouch();
        if (tr.event != TOUCH_NONE) activityPing();
        if (wasActive && !touchActive()) {
#ifdef DEBUG_GESTURES
            Serial.println("[touch] polling stopped — battery saving");
#endif
        }
    }

#ifdef DEBUG_GESTURES
    switch (tr.event) {
        case TOUCH_TAP:    Serial.printf("TAP (%d,%d)\n", tr.x, tr.y); break;
        case SWIPE_LEFT:   Serial.println("SWIPE_LEFT");  break;
        case SWIPE_RIGHT:  Serial.println("SWIPE_RIGHT"); break;
        case SWIPE_UP:     Serial.println("SWIPE_UP");    break;
        case SWIPE_DOWN:   Serial.println("SWIPE_DOWN");  break;
        default: break;
    }
#endif

    switch (currentScreen) {
        case SCREEN_MENU: {
            Screen next = menuHandleTouch(tr, epd);
            if (next != SCREEN_MENU) {
                currentScreen = next;
                if (currentScreen == SCREEN_CLOCK)    updateClock(epd, {TOUCH_NONE});
                if (currentScreen == SCREEN_POMODORO) pomoInit(epd);
                if (currentScreen == SCREEN_PHOTOS)   photosInit(epd);
                if (currentScreen == SCREEN_JESSIE)   drawJessie(epd);
                if (currentScreen == SCREEN_5)        screen5Init(epd);
                if (currentScreen == SCREEN_6)        screen6Init(epd);
                if (currentScreen == SCREEN_7)        screen7Init(epd);
                if (currentScreen == SCREEN_8)        screen8Init(epd);
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

        case SCREEN_5:
            if (updateScreen5(epd, tr)) { currentScreen = SCREEN_MENU; drawMenu(epd); }
            break;
        case SCREEN_6:
            if (updateScreen6(epd, tr)) { currentScreen = SCREEN_MENU; drawMenu(epd); }
            break;
        case SCREEN_7:
            if (updateScreen7(epd, tr)) { currentScreen = SCREEN_MENU; drawMenu(epd); }
            break;
        case SCREEN_8:
            if (updateScreen8(epd, tr)) { currentScreen = SCREEN_MENU; drawMenu(epd); }
            break;
    }

    delay(10);
}
