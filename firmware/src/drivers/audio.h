#pragma once
#include <Arduino.h>
#include <math.h>
#include "esp32-hal-i2c.h"
#include "pins.h"
#include "codec_board.h"
#include "codec_init.h"
#include "esp_codec_dev.h"

// 300ms chime at 16kHz stereo 16-bit
static const int CHIME_SAMPLES = 4800;

static bool                   _audioInited = false;
static esp_codec_dev_handle_t _playback    = nullptr;
static int16_t*               _chime1      = nullptr;  // 880 Hz A5
static int16_t*               _chime2      = nullptr;  // 659 Hz E5

static void _generateChime(int16_t* buf, int samples, float freq)
{
    const float sr = 16000.0f;
    const int   attack = 80;  // 5ms
    for (int i = 0; i < samples; i++) {
        float t   = (float)i / sr;
        float env = (i < attack)
                    ? (float)i / attack
                    : expf(-5.0f * (t - 0.005f));
        float val = sinf(2.0f * M_PI * freq * t) * env * 28000.0f;
        if (val >  32767.0f) val =  32767.0f;
        if (val < -32768.0f) val = -32768.0f;
        int16_t s = (int16_t)val;
        buf[i * 2]     = s;
        buf[i * 2 + 1] = s;
    }
}

static bool audioInit()
{
    if (_audioInited) return true;

    // GPIO42 is already LOW (audio rail powered from main.cpp setup)
    // Inject Wire's I2C bus handle so codec stack doesn't double-init port 0
    void* wireBus = i2cBusHandle(0);
    if (wireBus) {
        inject_i2c_bus_handle(0, wireBus);
    }

    set_codec_board_type("S3_ePaper_1_54");
    codec_init_cfg_t cfg = {
        .in_mode   = CODEC_I2S_MODE_STD,
        .out_mode  = CODEC_I2S_MODE_STD,
        .in_use_tdm = false,
        .reuse_dev  = false,
    };
    if (init_codec(&cfg) != 0) {
#ifdef DEBUG
        Serial.println("[audio] init_codec failed");
#endif
        return false;
    }

    _playback = get_playback_handle();
    if (!_playback) {
#ifdef DEBUG
        Serial.println("[audio] no playback handle");
#endif
        return false;
    }

    esp_codec_dev_set_out_vol(_playback, 100.0f);
    esp_codec_dev_sample_info_t fs = {};
    fs.sample_rate     = 16000;
    fs.channel         = 2;
    fs.bits_per_sample = 16;
    int openRet = esp_codec_dev_open(_playback, &fs);
#ifdef DEBUG
    Serial.printf("[audio] open ret %d\n", openRet);
#endif

    // 4800 samples × 2 ch × 2 bytes = 19200 bytes each — fits in regular heap
    _chime1 = (int16_t*)malloc(CHIME_SAMPLES * 4);
    _chime2 = (int16_t*)malloc(CHIME_SAMPLES * 4);
    if (_chime1) _generateChime(_chime1, CHIME_SAMPLES, 880.0f);
    if (_chime2) _generateChime(_chime2, CHIME_SAMPLES, 659.0f);

    _audioInited = true;
#ifdef DEBUG
    Serial.println("[audio] init ok");
#endif
    return true;
}

// which: 1 = high chime (880Hz A5), 2 = low chime (659Hz E5)
static void audioPlayChime(int which)
{
    if (!_audioInited || !_playback) return;
    int16_t* buf = (which == 1) ? _chime1 : _chime2;
    if (!buf) return;
    int ret = esp_codec_dev_write(_playback, buf, CHIME_SAMPLES * 4);
#ifdef DEBUG
    Serial.printf("[audio] write ret %d\n", ret);
#endif
}
