#pragma once
#include <Wire.h>

#define PCF85063_ADDR 0x51

// BCD decode helper
static inline uint8_t bcdDec(uint8_t b) { return (b >> 4) * 10 + (b & 0x0F); }
static inline uint8_t bcdEnc(uint8_t n) { return ((n / 10) << 4) | (n % 10); }

// Returns false if RTC not found or oscillator stopped (time invalid)
bool pcf85063Read(int& hours, int& minutes) {
    Wire.beginTransmission(PCF85063_ADDR);
    Wire.write(0x04); // seconds register
    if (Wire.endTransmission() != 0) return false;
    if (Wire.requestFrom(PCF85063_ADDR, 3) < 3) return false;

    uint8_t sec = Wire.read();
    uint8_t min = Wire.read();
    uint8_t hr  = Wire.read();

    if (sec & 0x80) return false; // OS flag = oscillator stopped, time invalid

    minutes = bcdDec(min & 0x7F);
    hours   = bcdDec(hr  & 0x3F);
    return true;
}

// Set time on RTC (also clears oscillator-stop flag and starts clock)
bool pcf85063Write(int hours, int minutes) {
    // Clear control register (start oscillator, 24h mode)
    Wire.beginTransmission(PCF85063_ADDR);
    Wire.write(0x00);
    Wire.write(0x00); // Control_1: normal mode, 24h
    if (Wire.endTransmission() != 0) return false;

    // Write seconds=0, minutes, hours
    Wire.beginTransmission(PCF85063_ADDR);
    Wire.write(0x04);
    Wire.write(bcdEnc(0));           // seconds (clears OS flag)
    Wire.write(bcdEnc(minutes));
    Wire.write(bcdEnc(hours));
    return Wire.endTransmission() == 0;
}
