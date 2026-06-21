#pragma once
#include <Wire.h>

#define SHTC3_ADDR 0x70

// Reads temperature (°C) and relative humidity (%). Returns false on I2C error.
bool shtc3Read(float& tempC, float& humidity) {
    Wire.beginTransmission(SHTC3_ADDR);
    Wire.write(0x35); Wire.write(0x17); // wake
    if (Wire.endTransmission() != 0) { tempC = -999; humidity = -999; return false; }
    delay(2);

    Wire.beginTransmission(SHTC3_ADDR);
    Wire.write(0x78); Wire.write(0x66); // measure T-first, polling
    Wire.endTransmission();
    delay(15);

    // 6 bytes: T_MSB T_LSB T_CRC RH_MSB RH_LSB RH_CRC
    if (Wire.requestFrom(SHTC3_ADDR, 6) < 6) { tempC = -999; humidity = -999; return false; }
    uint8_t t_msb  = Wire.read();
    uint8_t t_lsb  = Wire.read();
    Wire.read(); // T CRC
    uint8_t rh_msb = Wire.read();
    uint8_t rh_lsb = Wire.read();
    Wire.read(); // RH CRC

    Wire.beginTransmission(SHTC3_ADDR);
    Wire.write(0xB0); Wire.write(0x98); // sleep
    Wire.endTransmission();

    uint16_t t_raw  = (t_msb  << 8) | t_lsb;
    uint16_t rh_raw = (rh_msb << 8) | rh_lsb;
    tempC    = -45.0f + 175.0f * (t_raw  / 65535.0f);
    humidity = 100.0f         * (rh_raw / 65535.0f);
    return true;
}
