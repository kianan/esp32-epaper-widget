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

// Touch FT6336 I2C (shared bus with RTC)
#define TOUCH_INT   21
#define TOUCH_SDA   47
#define TOUCH_SCL   48

// Buttons
#define BOOT_BTN     0   // active LOW
#define PWR_BTN      18

// Power management
#define BAT_ADC      4
#define VBAT_PWR_EN  17  // HIGH = enable battery power to board
#define AUDIO_PWR_EN 42  // LOW  = audio amp off (save power)
