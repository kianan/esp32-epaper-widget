#include "WaveshareEPD.h"

// LUT waveform tables from Waveshare's official driver
static const uint8_t WF_Full[159] = {
    0x80,0x48,0x40,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
    0x40,0x48,0x80,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
    0x80,0x48,0x40,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
    0x40,0x48,0x80,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
    0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
    0xA,0x0,0x0,0x0,0x0,0x0,0x0,
    0x8,0x1,0x0,0x8,0x1,0x0,0x2,
    0xA,0x0,0x0,0x0,0x0,0x0,0x0,
    0x0,0x0,0x0,0x0,0x0,0x0,0x0,
    0x0,0x0,0x0,0x0,0x0,0x0,0x0,
    0x0,0x0,0x0,0x0,0x0,0x0,0x0,
    0x0,0x0,0x0,0x0,0x0,0x0,0x0,
    0x0,0x0,0x0,0x0,0x0,0x0,0x0,
    0x0,0x0,0x0,0x0,0x0,0x0,0x0,
    0x0,0x0,0x0,0x0,0x0,0x0,0x0,
    0x0,0x0,0x0,0x0,0x0,0x0,0x0,
    0x0,0x0,0x0,0x0,0x0,0x0,0x0,
    0x22,0x22,0x22,0x22,0x22,0x22,0x0,0x0,0x0,
    0x22,0x17,0x41,0x0,0x32,0x20
};

static const uint8_t WF_Partial[159] = {
    0x0,0x40,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
    0x80,0x80,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
    0x40,0x40,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
    0x0,0x80,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
    0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
    0xF,0x0,0x0,0x0,0x0,0x0,0x0,
    0x1,0x1,0x0,0x0,0x0,0x0,0x0,
    0x0,0x0,0x0,0x0,0x0,0x0,0x0,
    0x0,0x0,0x0,0x0,0x0,0x0,0x0,
    0x0,0x0,0x0,0x0,0x0,0x0,0x0,
    0x0,0x0,0x0,0x0,0x0,0x0,0x0,
    0x0,0x0,0x0,0x0,0x0,0x0,0x0,
    0x0,0x0,0x0,0x0,0x0,0x0,0x0,
    0x0,0x0,0x0,0x0,0x0,0x0,0x0,
    0x0,0x0,0x0,0x0,0x0,0x0,0x0,
    0x0,0x0,0x0,0x0,0x0,0x0,0x0,
    0x0,0x0,0x0,0x0,0x0,0x0,0x0,
    0x22,0x22,0x22,0x22,0x22,0x22,0x0,0x0,0x0,
    0x02,0x17,0x41,0xB0,0x32,0x28,
};

WaveshareEPD::WaveshareEPD(uint8_t cs, uint8_t dc, uint8_t rst, uint8_t busy,
                            uint8_t sclk, uint8_t mosi)
    : Adafruit_GFX(EPD_W, EPD_H),
      _cs(cs), _dc(dc), _rst(rst), _busy(busy), _sclk(sclk), _mosi(mosi),
      _spi(FSPI) {
    memset(_buffer, 0xFF, EPD_BUFFER_SIZE);
}

void WaveshareEPD::init() {
    pinMode(_cs,   OUTPUT); digitalWrite(_cs,   HIGH);
    pinMode(_dc,   OUTPUT); digitalWrite(_dc,   HIGH);
    pinMode(_rst,  OUTPUT); digitalWrite(_rst,  HIGH);
    pinMode(_busy, INPUT);

    _spi.begin(_sclk, -1, _mosi, -1);

    // Hardware reset
    digitalWrite(_rst, HIGH); delay(50);
    digitalWrite(_rst, LOW);  delay(20);
    digitalWrite(_rst, HIGH); delay(100);

    waitBusy();
    sendCommand(0x12); // SW reset
    waitBusy();

    sendCommand(0x01); // Driver output control
    sendData(0xC7);
    sendData(0x00);
    sendData(0x01);

    sendCommand(0x11); // Data entry mode: X inc, Y dec
    sendData(0x01);

    setWindow(0, EPD_W - 1, EPD_H - 1, 0);

    sendCommand(0x3C); // Border waveform
    sendData(0x01);

    sendCommand(0x18); // Read built-in temperature sensor
    sendData(0x80);

    sendCommand(0x22); // Load temperature and waveform
    sendData(0xB1);
    sendCommand(0x20);

    setRamCursor(0, EPD_H - 1);
    waitBusy();

    setLut(WF_Full);
}

void WaveshareEPD::clearBuffer(uint8_t color) {
    memset(_buffer, color, EPD_BUFFER_SIZE);
}

void WaveshareEPD::loadBuffer(const uint8_t* data, size_t len) {
    memcpy(_buffer, data, min(len, (size_t)EPD_BUFFER_SIZE));
}

void WaveshareEPD::display() {
    sendCommand(0x24);
    sendBytes(_buffer, EPD_BUFFER_SIZE);
    turnOnDisplay();
}

void WaveshareEPD::displayBase() {
    // Hardware reset to fully exit partial mode before full refresh
    digitalWrite(_rst, HIGH); delay(50);
    digitalWrite(_rst, LOW);  delay(20);
    digitalWrite(_rst, HIGH); delay(100);
    waitBusy();
    setLut(WF_Full);
    sendCommand(0x3C); sendData(0x01); // border waveform
    // Write to both frame buffers so partial refresh has a clean base
    sendCommand(0x24);
    sendBytes(_buffer, EPD_BUFFER_SIZE);
    sendCommand(0x26);
    sendBytes(_buffer, EPD_BUFFER_SIZE);
    turnOnDisplay();
}

void WaveshareEPD::initPartial() {
    digitalWrite(_rst, HIGH); delay(50);
    digitalWrite(_rst, LOW);  delay(20);
    digitalWrite(_rst, HIGH); delay(50);
    waitBusy();

    setLut(WF_Partial);

    sendCommand(0x37);
    sendData(0x00); sendData(0x00); sendData(0x00); sendData(0x00);
    sendData(0x00); sendData(0x40); sendData(0x00); sendData(0x00);
    sendData(0x00); sendData(0x00);

    sendCommand(0x3C); sendData(0x80);
    sendCommand(0x22); sendData(0xC0);
    sendCommand(0x20);
    waitBusy();
}

void WaveshareEPD::displayPartial() {
    sendCommand(0x24);
    sendBytes(_buffer, EPD_BUFFER_SIZE);
    turnOnDisplayPartial();
}

void WaveshareEPD::dumpBuffer() {
    Serial.flush();
    Serial.println("EPDDUMP_START");
    Serial.flush();
    for (int i = 0; i < EPD_BUFFER_SIZE; i++) {
        if (_buffer[i] < 0x10) Serial.print('0');
        Serial.print(_buffer[i], HEX);
        if ((i + 1) % 25 == 0) {
            Serial.println();
            Serial.flush();
        }
    // // Send raw binary in 64-byte chunks (USB full-speed packet size)
    // const int CHUNK = 64;
    // for (int i = 0; i < EPD_BUFFER_SIZE; i += CHUNK) {
    //     int len = min(CHUNK, EPD_BUFFER_SIZE - i);
    //     Serial.write(_buffer + i, len);
    //     Serial.flush();
    //     delay(10);
    }
    Serial.println("EPDDUMP_END");
    Serial.flush();
}

void WaveshareEPD::drawPixel(int16_t x, int16_t y, uint16_t color) {
    if (x < 0 || x >= EPD_W || y < 0 || y >= EPD_H) return;

    int16_t px = x;
    int16_t py = y;

    uint16_t idx = py * (EPD_W / 8) + (px >> 3);
    uint8_t  bit = 7 - (px & 0x07);

    if (color == 0) // black
        _buffer[idx] &= ~(1 << bit);
    else            // white
        _buffer[idx] |=  (1 << bit);
}

// ---- Private helpers ----

void WaveshareEPD::sendCommand(uint8_t cmd) {
    digitalWrite(_dc, LOW);
    digitalWrite(_cs, LOW);
    _spi.beginTransaction(SPISettings(4000000, MSBFIRST, SPI_MODE0));
    _spi.transfer(cmd);
    _spi.endTransaction();
    digitalWrite(_cs, HIGH);
}

void WaveshareEPD::sendData(uint8_t data) {
    digitalWrite(_dc, HIGH);
    digitalWrite(_cs, LOW);
    _spi.beginTransaction(SPISettings(4000000, MSBFIRST, SPI_MODE0));
    _spi.transfer(data);
    _spi.endTransaction();
    digitalWrite(_cs, HIGH);
}

void WaveshareEPD::sendBytes(const uint8_t* buf, int len) {
    digitalWrite(_dc, HIGH);
    digitalWrite(_cs, LOW);
    _spi.beginTransaction(SPISettings(4000000, MSBFIRST, SPI_MODE0));
    _spi.writeBytes(buf, len);
    _spi.endTransaction();
    digitalWrite(_cs, HIGH);
}

void WaveshareEPD::waitBusy() {
    unsigned long t = millis();
    while (digitalRead(_busy) == HIGH) {
        delay(5);
        if (millis() - t > 5000) break; // safety timeout
    }
}

void WaveshareEPD::setWindow(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1) {
    sendCommand(0x44);
    sendData((x0 >> 3) & 0xFF);
    sendData((x1 >> 3) & 0xFF);
    sendCommand(0x45);
    sendData(y0 & 0xFF); sendData((y0 >> 8) & 0xFF);
    sendData(y1 & 0xFF); sendData((y1 >> 8) & 0xFF);
}

void WaveshareEPD::setRamCursor(uint16_t x, uint16_t y) {
    sendCommand(0x4E); sendData(x & 0xFF);
    sendCommand(0x4F); sendData(y & 0xFF); sendData((y >> 8) & 0xFF);
}

void WaveshareEPD::setLut(const uint8_t* lut) {
    sendCommand(0x32);
    sendBytes(lut, 153);
    waitBusy();
    sendCommand(0x3F); sendData(lut[153]);
    sendCommand(0x03); sendData(lut[154]);
    sendCommand(0x04); sendData(lut[155]); sendData(lut[156]); sendData(lut[157]);
    sendCommand(0x2C); sendData(lut[158]);
}

void WaveshareEPD::turnOnDisplay() {
    sendCommand(0x22); sendData(0xC7);
    sendCommand(0x20);
    waitBusy();
}

void WaveshareEPD::turnOnDisplayPartial() {
    sendCommand(0x22); sendData(0xCF);
    sendCommand(0x20);
    waitBusy();
}
