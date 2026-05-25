#ifndef CONFIG_H
#define CONFIG_H

// WiFi настройки (объявления)
extern const char* ssid;
extern const char* password;

// URL радиостанции
extern const char* radio_url;

// Пины для PCM5102A
#define I2S_BCLK 10
#define I2S_LRC  11
#define I2S_DOUT 12

// Пины для энкодера
#define ENC_A    16
#define ENC_B    15
#define ENC_BTN  17

// Пины для дисплея ST7789 
#define LCD_HOST        SPI2_HOST
#define LCD_MOSI        6
#define LCD_SCLK        7
#define LCD_DC          5
#define LCD_CS          4
#define LCD_RST         8
#define LCD_BL          9

// Разрешение дисплея (для ST7789 обычно 240x240 или 240x320)
#define LCD_H_RES       320
#define LCD_V_RES       170


// Настройки тембра
#define TONE_BASS   3
#define TONE_MID    -6
#define TONE_TREBLE 2

// Начальная громкость (0-21)
#define DEFAULT_VOLUME 10

// Задержки антидребезга (мс)
#define DEBOUNCE_DELAY 100
#define BUTTON_DELAY   200

#endif // CONFIG_H