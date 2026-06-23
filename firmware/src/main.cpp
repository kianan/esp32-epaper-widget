#include <Arduino.h>
#include <Wire.h>
#include <LittleFS.h>
#include "driver/gpio.h"
#include "esp_system.h"

#include "pins.h"
#include "touch.h"
#include "saved_state.h"
#include "screen_menu.h"
#include "screen_clock.h"
#include "screen_pomodoro.h"
#include "screen_photos.h"
#include "screen_jessie.h"
#include "screen_wifi.h"
#include "screen_api.h"
#include "screen_audio.h"
#include "screen_8.h"
#include "rtc.h"
#include "sleep.h"

WaveshareEPD epd(EPD_CS, EPD_DC, EPD_RST, EPD_BUSY, EPD_SCLK, EPD_SDI);

struct ScreenDef {
    void (*init)(WaveshareEPD&);
    bool (*update)(WaveshareEPD&, TouchResult);
};

// Order must match the Screen enum in menu_logic.h
static const ScreenDef screenRegistry[] = {
    { nullptr,                   nullptr                    },  // SCREEN_MENU
    { screenClock::screenInit,   screenClock::screenUpdate  },  // SCREEN_CLOCK
    { pomodoro::screenInit,      pomodoro::screenUpdate     },  // SCREEN_POMODORO
    { photos::screenInit,        photos::screenUpdate       },  // SCREEN_PHOTOS
    { jessie::screenInit,        jessie::screenUpdate       },  // SCREEN_JESSIE
    { wifi::screenInit,          wifi::screenUpdate         },  // SCREEN_5
    { api::screenInit,           api::screenUpdate          },  // SCREEN_6
    { audioScreen::screenInit,   audioScreen::screenUpdate  },  // SCREEN_7
    { screen8::screenInit,       screen8::screenUpdate      },  // SCREEN_8
};

Screen currentScreen = SCREEN_MENU;

void _saveAndSleep(uint64_t timerWakeupUs = 0) {
    _savedState.screen     = currentScreen;
    _savedState.photoIndex = photos::getIndex();
    enterDeepSleep(timerWakeupUs);
}

void setup() {
    Serial.begin(115200);
    Serial.setTxTimeoutMs(0);

    pinMode(BOOT_BTN, INPUT_PULLUP);
    pinMode(PWR_BTN, INPUT_PULLUP);

    // Release any hold from deep sleep, then enable battery power rail
    gpio_hold_dis((gpio_num_t)VBAT_PWR_EN);
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
    Wire.setClock(400000);
    delay(200);
    touchInit();

    bool wakeFromSleep = (esp_reset_reason() == ESP_RST_DEEPSLEEP);
    if (wakeFromSleep && _savedState.screen != SCREEN_MENU) {
        currentScreen = _savedState.screen;
        screenRegistry[currentScreen].init(epd);
    } else {
        drawMenu(epd);
    }

    activityPing();
}

void loop() {
#ifdef DEBUG
    if (Serial.available()) {
        String cmd = Serial.readStringUntil('\n');
        cmd.trim();
        if (cmd == "SCREENSHOT") {
            epd.dumpBuffer();
        } else if (cmd.startsWith("SET_TIME ")) {
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
#endif

    // PWR_BTN press → immediate deep sleep
    if (digitalRead(PWR_BTN) == LOW) {
        delay(50);  // debounce
        if (digitalRead(PWR_BTN) == LOW) enterDeepSleep();
    }

    // Sleep on inactivity
    if (sleepTimeoutReached()) {
        enterDeepSleep();
    }

    TouchResult tr = {TOUCH_NONE, 0, 0};
    bool wasActive = touchActive();
    if (_touchIntFired || wasActive) {
        if (_touchIntFired && !wasActive) {
#ifdef DEBUG
            Serial.println("[touch] polling started");
#endif
        }
        _touchIntFired = false;
        tr = readTouch();
        if (tr.event != TOUCH_NONE) activityPing();
        if (wasActive && !touchActive()) {
#ifdef DEBUG
            Serial.println("[touch] polling stopped — battery saving");
#endif
        }
    }

#ifdef DEBUG
    switch (tr.event) {
        case TOUCH_TAP:   Serial.printf("TAP (%d,%d)\n", tr.x, tr.y); break;
        case SWIPE_LEFT:  Serial.println("SWIPE_LEFT");  break;
        case SWIPE_RIGHT: Serial.println("SWIPE_RIGHT"); break;
        case SWIPE_UP:    Serial.println("SWIPE_UP");    break;
        case SWIPE_DOWN:  Serial.println("SWIPE_DOWN");  break;
        default: break;
    }
#endif

    if (currentScreen == SCREEN_MENU) {
        Screen next = menuHandleTouch(tr, epd);
        if (next != SCREEN_MENU) {
            currentScreen = next;
            screenRegistry[currentScreen].init(epd);
        }
    } else {
        if (screenRegistry[currentScreen].update(epd, tr)) {
            currentScreen = SCREEN_MENU;
            drawMenu(epd);
        }
    }
}
