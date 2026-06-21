#pragma once
#include <Arduino.h>
#include <SPI.h>
#include <Adafruit_GFX.h>

#define EPD_W 200
#define EPD_H 200
#define EPD_BUFFER_SIZE (EPD_W * EPD_H / 8)  // 5000 bytes

class WaveshareEPD : public Adafruit_GFX {
public:
    WaveshareEPD(uint8_t cs, uint8_t dc, uint8_t rst, uint8_t busy,
                 uint8_t sclk, uint8_t mosi);

    void init();
    void clearBuffer(uint8_t color = 0xFF); // 0xFF = white
    void loadBuffer(const uint8_t* data, size_t len); // copy raw bytes into frame buffer
    void display();              // full refresh
    void displayBase();          // full refresh + writes both frame buffers (call before partial refresh)
    void displayPartial();       // partial refresh (faster, may ghost)
    void initPartial();          // call once before a series of displayPartial()
    void drawPixel(int16_t x, int16_t y, uint16_t color) override;
    void dumpBuffer(); // send frame buffer over Serial for screenshot capture

private:
    uint8_t _cs, _dc, _rst, _busy, _sclk, _mosi;
    uint8_t _buffer[EPD_BUFFER_SIZE];
    SPIClass _spi;

    void sendCommand(uint8_t cmd);
    void sendData(uint8_t data);
    void sendBytes(const uint8_t* buf, int len);
    void waitBusy();
    void setWindow(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1);
    void setRamCursor(uint16_t x, uint16_t y);
    void setLut(const uint8_t* lut);
    void turnOnDisplay();
    void turnOnDisplayPartial();
};
