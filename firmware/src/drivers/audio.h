#pragma once
#include <Arduino.h>
#include <math.h>
#include "esp32-hal-i2c.h"
#include "pins.h"
#include "codec_board.h"
#include "codec_init.h"
#include "esp_codec_dev.h"

// The Ding: 900ms — layered C5+E5 sines, 2ms attack, slow exponential ring-out
static const int DING_SAMPLES = 14400;  // 900ms at 16kHz
// The Eh-Eh: two 150ms square-wave bursts at 150Hz with 80ms gap, hard cutoff
static const int EHEH_SAMPLES = 6080;   // 2×2400 + 1280 gap
// Record: 5s stereo 16kHz 16-bit = 320KB, allocated from PSRAM
static const int REC_SAMPLES  = 80000;  // 5s × 16kHz
static const int REC_BYTES    = REC_SAMPLES * 4; // stereo 16-bit

static bool                   _audioInited = false;
static esp_codec_dev_handle_t _playback    = nullptr;
static esp_codec_dev_handle_t _capture     = nullptr;
static int16_t*               _ding        = nullptr;
static int16_t*               _eheh        = nullptr;
static int16_t*               _recbuf      = nullptr;
static bool                   _hasRec      = false;

static void _generateDing(int16_t* buf, int samples)
{
    const float sr     = 16000.0f;
    const float f1     = 523.25f;  // C5
    const float f2     = 659.26f;  // E5
    const int   attack = (int)(0.002f * sr);  // 2ms
    for (int i = 0; i < samples; i++) {
        float t   = (float)i / sr;
        float env = (i < attack)
                    ? (float)i / attack
                    : expf(-2.5f * (t - 0.002f));
        // Layer both frequencies at equal amplitude
        float wave = 0.5f * sinf(2.0f * M_PI * f1 * t)
                   + 0.5f * sinf(2.0f * M_PI * f2 * t);
        float val = wave * env * 28000.0f;
        if (val >  32767.0f) val =  32767.0f;
        if (val < -32768.0f) val = -32768.0f;
        int16_t s = (int16_t)val;
        buf[i * 2]     = s;
        buf[i * 2 + 1] = s;
    }
}

static void _generateEhEh(int16_t* buf, int samples)
{
    const float sr    = 16000.0f;
    const float freq  = 150.0f;
    const int   burst = (int)(0.150f * sr);  // 2400 samples per burst
    const int   gap   = (int)(0.080f * sr);  // 1280 samples silence
    for (int i = 0; i < samples; i++) {
        float t   = (float)i / sr;
        bool  on  = (i < burst) || (i >= burst + gap && i < 2 * burst + gap);
        float phase = fmodf(t * freq, 1.0f);
        float wave  = (phase < 0.5f) ? 1.0f : -1.0f;  // square wave
        float val   = on ? wave * 24000.0f : 0.0f;
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

    _ding   = (int16_t*)malloc(DING_SAMPLES * 4);   // ~56KB
    _eheh   = (int16_t*)malloc(EHEH_SAMPLES * 4);   // ~24KB
    _recbuf = (int16_t*)heap_caps_malloc(REC_BYTES, MALLOC_CAP_SPIRAM);
    if (_ding)  _generateDing(_ding, DING_SAMPLES);
    if (_eheh)  _generateEhEh(_eheh, EHEH_SAMPLES);
#ifdef DEBUG
    Serial.printf("[audio] recbuf %s (%d bytes)\n", _recbuf ? "ok" : "FAIL", REC_BYTES);
#endif

    _capture = get_record_handle();
    if (_capture) {
        esp_codec_dev_sample_info_t fs = {};
        fs.sample_rate     = 16000;
        fs.channel         = 2;
        fs.bits_per_sample = 16;
        int ret = esp_codec_dev_open(_capture, &fs);
#ifdef DEBUG
        Serial.printf("[audio] capture open ret %d\n", ret);
#endif
    }

    _audioInited = true;
#ifdef DEBUG
    Serial.println("[audio] init ok");
#endif
    return true;
}

static void audioPlayDing()
{
    if (!_audioInited || !_playback || !_ding) return;
    int ret = esp_codec_dev_write(_playback, _ding, DING_SAMPLES * 4);
#ifdef DEBUG
    Serial.printf("[audio] ding ret %d\n", ret);
#endif
}

static void audioPlayEhEh()
{
    if (!_audioInited || !_playback || !_eheh) return;
    int ret = esp_codec_dev_write(_playback, _eheh, EHEH_SAMPLES * 4);
#ifdef DEBUG
    Serial.printf("[audio] eheh ret %d\n", ret);
#endif
}

// Returns false if no PSRAM buffer available
static bool audioCanRecord() { return _recbuf != nullptr; }

static void audioRecord()
{
    if (!_audioInited || !_capture || !_recbuf) return;
    _hasRec = false;
    int ret = esp_codec_dev_read(_capture, _recbuf, REC_BYTES);
    _hasRec = (ret == 0);
#ifdef DEBUG
    Serial.printf("[audio] record ret %d\n", ret);
#endif
}

static void audioPlayRecording()
{
    if (!_audioInited || !_playback || !_hasRec || !_recbuf) return;
    int ret = esp_codec_dev_write(_playback, _recbuf, REC_BYTES);
#ifdef DEBUG
    Serial.printf("[audio] playRec ret %d\n", ret);
#endif
}
