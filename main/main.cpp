#include "Arduino.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "config.h"
#include "tasks/audio_task.h"
#include "tasks/ui_task.h"
#include "drivers/audio/audio_manager.h"
#include "nvs_manager.h"
#include "messages/audio_messages.h"

static const char* TAG = "MAIN";

// Глобальные очереди
QueueHandle_t audioQueue = NULL;

void setup() {
    Serial.begin(115200);
    
    ESP_LOGI(TAG, "=== ESP32-S3 Internet Radio ===");
    ESP_LOGI(TAG, "Free heap: %d bytes", esp_get_free_heap_size());
    
    // Создаём очереди для сообщений (максимум 10 сообщений)
    audioQueue = xQueueCreate(AUDIO_QUEUE_SIZE, sizeof(AudioMessage));

    // Инициализация аудио
    AudioManager::getInstance().begin(I2S_BCLK, I2S_LRC, I2S_DOUT);
    // AudioManager::getInstance().setDefaultVolume(DEFAULT_VOLUME);
    // AudioManager::getInstance().setTone(TONE_BASS, TONE_MID, TONE_TREBLE);
 
    // Инициализация NVS для сохранения настроек
    NVSManager::getInstance().init();    

    // Запуск задач
    ESP_LOGI(TAG, "Starting tasks...");
    startAudioTask(0, 3, 16384);   // Ядро 0, приоритет 3, стек 16KB
    startUiTask(1, 1, 4096);       // Ядро 1, приоритет 1, стек 4KB
    
    ESP_LOGI(TAG, "System ready!");
}

void loop() {
    static unsigned long lastCheck = 0;
    if (millis() - lastCheck > 10000) {
        lastCheck = millis();
        
        // Мониторинг (опционально)
        // AudioManager& audio = AudioManager::getInstance();
        
        // ESP_LOGI(TAG, "=== System Status ===");
        // ESP_LOGI(TAG, "Free heap: %d bytes", esp_get_free_heap_size());
        // ESP_LOGI(TAG, "Status: %s | Volume: %d", 
        //     audio.isPlaying() ? "Playing" : "Paused",
        //     audio.getVolume());
        
        // TaskHandle_t audioHandle = getAudioTaskHandle();
        // TaskHandle_t uiHandle = getUiTaskHandle();
        
        // if (audioHandle != NULL) {
        //     ESP_LOGI(TAG, "Audio task free stack: %d bytes", 
        //         uxTaskGetStackHighWaterMark(audioHandle));
        // }
        // if (uiHandle != NULL) {
        //     ESP_LOGI(TAG, "UI task free stack: %d bytes", 
        //         uxTaskGetStackHighWaterMark(uiHandle));
        // }
    }
    
    vTaskDelay(pdMS_TO_TICKS(1000));
}