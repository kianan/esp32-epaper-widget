#pragma once
#include <Wire.h>

#define SHTC3_ADDR 0x70

// Returns temperature in Celsius, or -999 on failure
float shtc3ReadTemp() {
    // Wake
    Wire.beginTransmission(SHTC3_ADDR);
    Wire.write(0x35); Wire.write(0x17);
    if (Wire.endTransmission() != 0) return -999;
    delay(2);

    // Measure: T first, polling, no clock stretch
    Wire.beginTransmission(SHTC3_ADDR);
    Wire.write(0x78); Wire.write(0x66);
    Wire.endTransmission();
    delay(15);

    // Read 6 bytes: T_MSB T_LSB T_CRC RH_MSB RH_LSB RH_CRC
    if (Wire.requestFrom(SHTC3_ADDR, 6) < 6) return -999;
    uint8_t t_msb = Wire.read();
    uint8_t t_lsb = Wire.read();
    Wire.read(); // T CRC
    Wire.read(); Wire.read(); Wire.read(); // RH + CRC (unused)

    // Sleep
    Wire.beginTransmission(SHTC3_ADDR);
    Wire.write(0xB0); Wire.write(0x98);
    Wire.endTransmission();

    uint16_t raw = (t_msb << 8) | t_lsb;
    return -45.0f + 175.0f * (raw / 65535.0f);
}
