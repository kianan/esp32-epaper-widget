#pragma once
#include <WiFiClientSecure.h>
#include "WaveshareEPD.h"
#include "touch.h"
#include "audio.h"
#include "wifi_conn.h"
#include "secrets.h"
#include <Fonts/FreeSansBold9pt7b.h>

namespace audioScreen {

#define AUDIO_MID_X   100
#define AUDIO_MID_Y   100
#define DISCORD_HOST  "discord.com"

enum RecState { IDLE, RECORDING, RECORDED, SENDING };
static RecState _state = IDLE;

static void _buildWavHeader(uint8_t* h, uint32_t dataBytes)
{
    uint32_t fileSize   = dataBytes + 36;
    uint32_t byteRate   = 16000 * 2 * 2;
    uint16_t blockAlign = 2 * 2;

    memcpy(h,    "RIFF", 4); memcpy(h+4,  &fileSize,   4);
    memcpy(h+8,  "WAVE", 4); memcpy(h+12, "fmt ", 4);
    *(uint32_t*)(h+16) = 16;  *(uint16_t*)(h+20) = 1;
    *(uint16_t*)(h+22) = 2;   *(uint32_t*)(h+24) = 16000;
    *(uint32_t*)(h+28) = byteRate; *(uint16_t*)(h+32) = blockAlign;
    *(uint16_t*)(h+34) = 16;
    memcpy(h+36, "data", 4); *(uint32_t*)(h+40) = dataBytes;
}

static void _draw(WaveshareEPD& epd)
{
    epd.clearBuffer();
    epd.setFont(&FreeSansBold9pt7b);
    epd.setTextColor(0);

    epd.drawLine(0,          AUDIO_MID_Y, 200, AUDIO_MID_Y, 0);
    epd.drawLine(AUDIO_MID_X, 0,          AUDIO_MID_X, 200, 0);

    epd.setCursor(32, 57);  epd.print("1-Up");
    epd.setCursor(127, 57); epd.print("Eh Eh");

    switch (_state) {
        case RECORDING:
            epd.setCursor(5, 150);   epd.print("Recording");
            epd.setCursor(132, 150); epd.print("Send");
            break;
        case SENDING:
            epd.setCursor(22, 141); epd.print("Record");
            epd.setCursor(41, 159); epd.print("5s");
            epd.setCursor(107, 150); epd.print("Sending...");
            break;
        default:
            epd.setCursor(22, 141); epd.print("Record");
            epd.setCursor(41, 159); epd.print("5s");
            epd.setCursor(132, 150); epd.print("Send");
            break;
    }

    epd.setFont(NULL); epd.setTextSize(1);
    epd.setCursor(5, 194); epd.print("Swipe right: back");

    epd.displayBase();
}

static void _sendToDiscord(WaveshareEPD& epd)
{
    if (!wifiConnected()) wifiBegin();
    if (!wifiConnected()) {
#ifdef DEBUG
        Serial.println("[discord] no wifi");
#endif
        return;
    }

    uint8_t wavHeader[44] = {};
    _buildWavHeader(wavHeader, REC_BYTES);

    const char* boundary = "DiscordBoundary";
    const char* dispLine = "Content-Disposition: form-data; name=\"files[0]\"; filename=\"recording.wav\"\r\n";
    const char* ctLine   = "Content-Type: audio/wav\r\n";
    String partHead = String("--") + boundary + "\r\n" + dispLine + ctLine + "\r\n";
    String closing  = String("\r\n--") + boundary + "--\r\n";
    int contentLen  = partHead.length() + 44 + REC_BYTES + closing.length();

    WiFiClientSecure client;
    client.setInsecure();
    if (!client.connect(DISCORD_HOST, 443)) {
#ifdef DEBUG
        Serial.println("[discord] connect failed");
#endif
        return;
    }

    client.printf("POST %s HTTP/1.1\r\n", DISCORD_URL);
    client.printf("Host: %s\r\n", DISCORD_HOST);
    client.printf("Content-Type: multipart/form-data; boundary=%s\r\n", boundary);
    client.printf("Content-Length: %d\r\n", contentLen);
    client.print("Connection: close\r\n\r\n");

    client.print(partHead);
    client.write(wavHeader, 44);

    const int CHUNK = 4096;
    uint8_t* ptr = (uint8_t*)_recbuf;
    int remaining = REC_BYTES;
    while (remaining > 0) {
        int sz = min(remaining, CHUNK);
        client.write(ptr, sz);
        ptr += sz;
        remaining -= sz;
    }
    client.print(closing);

    // Read status line
    unsigned long timeout = millis() + 10000;
    while (client.connected() && millis() < timeout) {
        if (client.available()) {
            String line = client.readStringUntil('\n');
#ifdef DEBUG
            Serial.println("[discord] " + line);
#endif
            break;
        }
    }
    client.stop();
}

void screenInit(WaveshareEPD& epd)
{
    _state = IDLE;
    audioInit();
    _draw(epd);
}

bool screenUpdate(WaveshareEPD& epd, TouchResult tr)
{
    if (tr.event == SWIPE_RIGHT) return true;

    if (tr.event == TOUCH_TAP) {
        bool left = (tr.x < AUDIO_MID_X);
        bool top  = (tr.y < AUDIO_MID_Y);

        if (top && left) {
            audioPlayDing();
        } else if (top && !left) {
            audioPlayEhEh();
        } else if (!top && left) {
            if (audioCanRecord()) {
                _state = RECORDING;
                _draw(epd);
                audioRecord();
                audioPlayRecording();   // auto-playback after record
                _state = RECORDED;
                _draw(epd);
            }
        } else {
            // bottom-right: Send
            if (_state == RECORDED) {
                _state = SENDING;
                _draw(epd);
                _sendToDiscord(epd);
                _state = RECORDED;
                _draw(epd);
            }
        }
    }
    return false;
}

} // namespace audioScreen
