#pragma once

// ============================================================================
// WiFi настройки
// ============================================================================

extern const char* ssid;
extern const char* password;


// ============================================================================
// PCM5102A
// ============================================================================

constexpr int I2S_BCLK = 10;
constexpr int I2S_LRC  = 11;
constexpr int I2S_DOUT = 12;


// ============================================================================
// Encoder #1
// ============================================================================

constexpr int ENC_A   = 16;
constexpr int ENC_B   = 15;
constexpr int ENC_BTN = 17;


// ============================================================================
// Encoder #2
// ============================================================================

constexpr int ENC_2_A   = 41;
constexpr int ENC_2_B   = 42;
constexpr int ENC_2_BTN = 40;


// ============================================================================
// SD card
// ============================================================================

constexpr int SD_CS   = 47;
constexpr int SD_MOSI = 18;
constexpr int SD_MISO = 38;
constexpr int SD_SCLK = 20;


// ============================================================================
// ST7789 display
// ============================================================================

#define LCD_HOST SPI2_HOST

constexpr int LCD_MOSI = 6;
constexpr int LCD_SCLK = 7;
constexpr int LCD_DC   = 5;
constexpr int LCD_CS   = 4;
constexpr int LCD_RST  = 8;
constexpr int LCD_BL   = 9;


// ============================================================================
// Display resolution
// ============================================================================

constexpr int LCD_H_RES = 320;
constexpr int LCD_V_RES = 240;

constexpr int LCD_GAP_X = 0;
constexpr int LCD_GAP_Y = 0;


// ============================================================================
// Display settings
// ============================================================================

// Таймаут отключения дисплея (мс)
// 0 = не отключать
constexpr unsigned long DISPLAY_SLEEP_TIMEOUT_MS = 0; 


// ============================================================================
// Encoder settings
// ============================================================================

constexpr bool ENC_1_INVERT = true;
constexpr bool ENC_2_INVERT = true;

constexpr unsigned long ENCODER_LONG_PRESS_TIME   = 800;
constexpr unsigned long ENCODER_DOUBLE_CLICK_TIME = 300;


// ============================================================================
// Tone settings
// ============================================================================

constexpr int TONE_BASS   = 3;
constexpr int TONE_MID    = -6;
constexpr int TONE_TREBLE = 2;


// ============================================================================
// Volume settings
// ============================================================================

constexpr int DEFAULT_VOLUME = 10;

constexpr int MIN_VOLUME = 0;
constexpr int MAX_VOLUME = 15;


// ============================================================================
// Queue sizes
// ============================================================================

constexpr int AUDIO_QUEUE_SIZE       = 10;
constexpr int AUDIO_TO_UI_QUEUE_SIZE = 10;