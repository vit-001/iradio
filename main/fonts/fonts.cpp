#include "fonts.h"
#include "esp_log.h"

static const char* TAG = "FONTS";

void init_fonts(void) {
    ESP_LOGI(TAG, "Fonts initialized:");
    ESP_LOGI(TAG, "  Inter Medium 14px - UI text");
    ESP_LOGI(TAG, "  Inter Medium 18px - station/song titles");
    ESP_LOGI(TAG, "  Roboto Mono 12px - status bar, bitrate");
    ESP_LOGI(TAG, "  Roboto Mono 14px - dB values, frequency");
    ESP_LOGI(TAG, "  Bebas Neue 24px - volume, clock");
}