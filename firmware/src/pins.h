#pragma once

// E-Paper SPI
#define EPD_3V3_EN  6
#define EPD_TP_RST  7
#define EPD_BUSY    8
#define EPD_RST     9
#define EPD_DC      10
#define EPD_CS      11
#define EPD_SCLK    12
#define EPD_SDI     13

// ES8311 Audio Codec — I2S
#define I2S_MCLK    14
#define I2S_BCLK    15
#define I2S_DIN     16   // mic input (codec → ESP32)
#define I2S_WS      38   // word select / LRCK
#define I2S_DOUT    45   // speaker output (ESP32 → codec)
#define AUDIO_PA    46   // speaker power amplifier enable

// Touch FT6336 I2C (shared bus with RTC, SHTC3)
#define TOUCH_INT   21
#define TOUCH_SDA   47
#define TOUCH_SCL   48

// Buttons
#define BOOT_BTN     0   // active LOW, also deep-sleep wakeup
#define PWR_BTN      18

// TF card (SDMMC 1-bit mode)
#define TF_CLK       39
#define TF_CMD       41  // MOSI
#define TF_D0        40  // MISO

// Power management
#define BAT_ADC      4
#define VBAT_PWR_EN  17  // HIGH = enable battery power to board
#define AUDIO_PWR_EN 42  // LOW  = audio amp off (save power)

// Status LED
#define LED_PIN      3
