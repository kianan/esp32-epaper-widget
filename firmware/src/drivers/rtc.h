// Real-time clock driver (PCF85063A, I2C 0x51)
// Keeps time across power cycles via coin cell battery.
// Provides read/write for HH:MM:SS; survives deep sleep.
#pragma once
#include <Wire.h>

#define RTC_I2C_ADDR 0x51

static inline uint8_t bcdDec(uint8_t b) { return (b >> 4) * 10 + (b & 0x0F); }
static inline uint8_t bcdEnc(uint8_t n) { return ((n / 10) << 4) | (n % 10); }

// Read HH:MM — returns false if RTC missing or battery died (time invalid)
bool rtcRead(int& hours, int& minutes) {
    Wire.beginTransmission(RTC_I2C_ADDR);
    Wire.write(0x04);
    if (Wire.endTransmission() != 0) return false;
    if (Wire.requestFrom(RTC_I2C_ADDR, 3) < 3) return false;
    uint8_t sec = Wire.read();
    uint8_t min = Wire.read();
    uint8_t hr  = Wire.read();
    if (sec & 0x80) return false;
    minutes = bcdDec(min & 0x7F);
    hours   = bcdDec(hr  & 0x3F);
    return true;
}

// Read HH:MM:SS — used to compute elapsed time after deep sleep wake
bool rtcReadTime(int& hours, int& minutes, int& seconds) {
    Wire.beginTransmission(RTC_I2C_ADDR);
    Wire.write(0x04);
    if (Wire.endTransmission() != 0) return false;
    if (Wire.requestFrom(RTC_I2C_ADDR, 3) < 3) return false;
    uint8_t sec = Wire.read();
    uint8_t min = Wire.read();
    uint8_t hr  = Wire.read();
    if (sec & 0x80) return false;
    seconds = bcdDec(sec & 0x7F);
    minutes = bcdDec(min & 0x7F);
    hours   = bcdDec(hr  & 0x3F);
    return true;
}

// Returns seconds since midnight, or -1 if time invalid
int rtcSecsSinceMidnight() {
    int h, m, s;
    if (!rtcReadTime(h, m, s)) return -1;
    return h * 3600 + m * 60 + s;
}

// Set time — also clears oscillator-stop flag and starts the clock
bool rtcWrite(int hours, int minutes) {
    Wire.beginTransmission(RTC_I2C_ADDR);
    Wire.write(0x00);
    Wire.write(0x00); // Control_1: 24h mode, oscillator running
    if (Wire.endTransmission() != 0) return false;
    Wire.beginTransmission(RTC_I2C_ADDR);
    Wire.write(0x04);
    Wire.write(bcdEnc(0));        // seconds = 0, clears OS flag
    Wire.write(bcdEnc(minutes));
    Wire.write(bcdEnc(hours));
    return Wire.endTransmission() == 0;
}
