#include "audio_task.h"
#include "config.h"
#include "stations.h"
#include "drivers/audio/audio_manager.h"
#include "drivers/nvs/nvs_manager.h"
#include "messages/audio_messages.h"
#include "messages/audio_to_ui_messages.h"
#include "esp_log.h"
#include "Arduino.h"

static const char* TAG = "AUDIO_TASK";
static TaskHandle_t s_audioTaskHandle = NULL;
static volatile bool s_audioTaskRunning = true;

extern QueueHandle_t audioQueue;
extern QueueHandle_t audioToUIQueue;

// Глобальный callback для информации от библиотеки Audio
void my_audio_info(Audio::msg_t m) {
    switch (m.e) {
        case Audio::evt_info:
            ESP_LOGI(TAG, "Stream info: %s", m.msg);
            break;
        case Audio::evt_id3data:
            ESP_LOGI(TAG, "ID3 metadata: %s", m.msg);
            // Обновляем дисплей с информацией о треке
            // tft.fillRect(10, 35, tft.width() - 20, 20, TFT_BLACK);
            // tft.setCursor(10, 45);
            // tft.setTextColor(TFT_WHITE, TFT_BLACK);
            // tft.setTextSize(1);
            // tft.print(m.msg);
            break;
        case Audio::evt_streamtitle:
            ESP_LOGI(TAG, "Stream title: %s", m.msg);
            // tft.fillRect(10, 35, tft.width() - 20, 20, TFT_BLACK);
            // tft.setCursor(10, 45);
            // tft.print(m.msg);
            break;
        case Audio::evt_bitrate:
            ESP_LOGI(TAG, "Bitrate: %s", m.msg);
            break;
        case Audio::evt_eof:
            ESP_LOGW(TAG, "End of stream: %s", m.msg);
            break;
        default:
            ESP_LOGD(TAG, "Event %d: %s", m.e, m.msg);
            break;
    }
}

// Обработка команд из очереди
void processAudioCommands(AudioManager& audio) {
    AudioMessage msg;
    
    // Проверяем очередь (не блокируем)
    if (xQueueReceive(audioQueue, &msg, 0) == pdTRUE) {
        switch (msg.type) {
            case CMD_SET_VOLUME:
                audio.setVolume(msg.value1);
                ESP_LOGI(TAG, "Volume set to %d", msg.value1);
                break;
                
            case CMD_SET_TONE:
                audio.setTone(msg.value1, msg.value2, msg.value3);
                ESP_LOGI(TAG, "Tone set: B=%d, M=%d, T=%d", msg.value1, msg.value2, msg.value3);
                break;
                
            case CMD_PLAY_URL:
                ESP_LOGI(TAG, "Playing URL: %s", msg.url);
                audio.connectToStream(msg.url);
                break;
                
            case CMD_PLAY_PAUSE:
                audio.playPause();
                break;
                
            default:
                break;
        }
    }
}

void audioTaskFunction(void* parameter) {
    ESP_LOGI(TAG, "Started on core %d, free heap: %d", 
        xPortGetCoreID(), esp_get_free_heap_size());
    
    AudioManager& audio = AudioManager::getInstance();
    
    // Регистрация callback
    Audio::audio_info_callback = my_audio_info;
    ESP_LOGD(TAG, "Callback registered");
    
    // WiFi подключение
    ESP_LOGI(TAG, "Connecting to WiFi SSID: %s", ssid);
    WiFi.begin(ssid, password);
    
    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        attempts++;
        ESP_LOGD(TAG, "Waiting for WiFi... (%d sec)", attempts / 2);
        if (attempts > 30) {
            ESP_LOGW(TAG, "WiFi timeout, retrying...");
            attempts = 0;
            WiFi.reconnect();
        }
    }
    
    ESP_LOGI(TAG, "WiFi connected successfully!");
    ESP_LOGI(TAG, "IP address: %s", WiFi.localIP().toString().c_str());
    ESP_LOGI(TAG, "RSSI: %d dBm", WiFi.RSSI());
    
    // Запуск аудио потока
    ESP_LOGI(TAG, "Connecting to stream: %s", radio_url);
    audio.connectToStream(radio_url);
    
    // Основной цикл
    unsigned long lastLoopLog = 0;
    while (s_audioTaskRunning) {
        // Обрабатываем команды из очереди
        processAudioCommands(audio);

        // Аудио-цикл
        audio.loop();
        
        // if (millis() - lastLoopLog > 30000) {
        //     lastLoopLog = millis();
        //     ESP_LOGD(TAG, "Audio loop running, playing: %s", 
        //         audio.isPlaying() ? "yes" : "no");
        // }
        
        vTaskDelay(pdMS_TO_TICKS(5));
    }
    
    ESP_LOGW(TAG, "Audio task stopping");
    vTaskDelete(NULL);
}

void startAudioTask(int core, int priority, int stackSize) {
    ESP_LOGI(TAG, "Starting on core %d, priority %d, stack %d", core, priority, stackSize);
    s_audioTaskRunning = true;
    xTaskCreatePinnedToCore(
        audioTaskFunction,
        "AudioTask",
        stackSize,
        NULL,
        priority,
        &s_audioTaskHandle,
        core
    );
}

TaskHandle_t getAudioTaskHandle() {
    return s_audioTaskHandle;
}

void stopAudioTask() {
    ESP_LOGI(TAG, "Stopping audio task");
    s_audioTaskRunning = false;
}