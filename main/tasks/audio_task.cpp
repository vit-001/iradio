#include "audio_task.h"
#include "config.h"
#include "station/stations.h"
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
    AudioManager& audio = AudioManager::getInstance();

    AudioToUIMessage audMsg;

    switch (m.e) {
        case Audio::evt_info:{
            ESP_LOGI(TAG, "Stream info: /%d/ %s", m.e, m.msg);

            break;
        }
        case Audio::evt_id3data:
            ESP_LOGI(TAG, "ID3 metadata: /%d/ %s", m.e, m.msg);
            // Обновляем дисплей с информацией о треке
            // tft.fillRect(10, 35, tft.width() - 20, 20, TFT_BLACK);
            // tft.setCursor(10, 45);
            // tft.setTextColor(TFT_WHITE, TFT_BLACK);
            // tft.setTextSize(1);
            // tft.print(m.msg);
            break;
        case Audio::evt_streamtitle:
            ESP_LOGI(TAG, "Stream title: /%d/ %s", m.e, m.msg);

            break;
        case Audio::evt_bitrate:
            ESP_LOGI(TAG, "Bitrate: /%d/ %s", m.e, m.msg);
            audMsg.type = EVENT_BITRATE_CHANGED;
            audMsg.data.bitrate = atoi(m.msg);
            xQueueSend(audioToUIQueue, &audMsg, portMAX_DELAY);
            ESP_LOGI(TAG, "Bitrate updated to %d", audMsg.data.bitrate);

            break;
        case Audio::evt_eof:
            ESP_LOGW(TAG, "End of stream: /%d/ %s", m.e, m.msg);
            break;
        default:
            ESP_LOGD(TAG, "Event %d: /%d/ %s", m.e, m.e, m.msg);
            break;
    }
}

// Обработка команд из очереди
void processAudioCommands(AudioManager& audio) {
    AudioMessage msg;
    AudioToUIMessage audMsg;
    int currentVolume = audio.getVolume();
    
    // Проверяем очередь (не блокируем)
    if (xQueueReceive(audioQueue, &msg, 0) == pdTRUE) {
        switch (msg.type) {
            case CMD_SET_VOLUME:
                currentVolume = msg.value1;
                if (currentVolume <= MIN_VOLUME) currentVolume = MIN_VOLUME;
                if (currentVolume >= MAX_VOLUME) currentVolume = MAX_VOLUME;
                audio.setVolume(currentVolume);                    
                audMsg.type = EVENT_VOLUME_CHANGED;
                audMsg.data.volume = audio.getVolume();
                xQueueSend(audioToUIQueue, &audMsg, portMAX_DELAY);
                ESP_LOGI(TAG, "Volume set to %d", audio.getVolume());
                break;

            case CMD_VOLUME_UP:
                if (currentVolume < MAX_VOLUME) {
                    audio.setVolume(currentVolume + 1);                    
                    audMsg.type = EVENT_VOLUME_CHANGED;
                    audMsg.data.volume = audio.getVolume();
                    xQueueSend(audioToUIQueue, &audMsg, portMAX_DELAY);
                    ESP_LOGI(TAG, "Volume increased to %d", audio.getVolume());
                }
                break;
                
            case CMD_VOLUME_DOWN:
                if (currentVolume > MIN_VOLUME) {
                    audio.setVolume(currentVolume - 1);
                    audMsg.type = EVENT_VOLUME_CHANGED;
                    audMsg.data.volume = audio.getVolume();
                    xQueueSend(audioToUIQueue, &audMsg, portMAX_DELAY);
                    ESP_LOGI(TAG, "Volume decreased to %d", audio.getVolume());
                }
                break;
                
            case CMD_SET_TONE:
                audio.setTone(msg.value1, msg.value2, msg.value3);
                ESP_LOGI(TAG, "Tone set: B=%d, M=%d, T=%d", msg.value1, msg.value2, msg.value3);
                break;
                
            case CMD_PLAY_URL:
                ESP_LOGI(TAG, "Playing URL: %s", msg.url);
                
                // сбрасываем битрейт при смене станции, чтобы не показывать старый битрейт от предыдущей станции
                audMsg.type = EVENT_BITRATE_CHANGED;
                audMsg.data.bitrate = 0;
                xQueueSend(audioToUIQueue, &audMsg, portMAX_DELAY);

                // отправляем событие смены станции в UI
                audMsg.type = EVENT_STATION_CHANGED;
                strncpy(audMsg.data.url, msg.url, sizeof(audMsg.data.url) - 1);
                audMsg.data.url[sizeof(audMsg.data.url) - 1] = '\0';
                xQueueSend(audioToUIQueue, &audMsg, portMAX_DELAY);

                // подключаем новую станцию
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
   
    // Wi-Fi подключение
    audio.setWiFiCredentials(ssid, password);
    if (!audio.connectWiFi()) {
        ESP_LOGE(TAG, "WiFi connection failed, task will stop");
        vTaskDelete(NULL);
    }

    // // Запуск аудио потока
    // ESP_LOGI(TAG, "Connecting to stream: %s", radio_url);
    // audio.connectToStream(radio_url);
    
    // Основной цикл
    unsigned long lastLoopLog = 0;
    while (s_audioTaskRunning) {
        // Обрабатываем команды из очереди
        processAudioCommands(audio);

        // Аудио-цикл
        audio.loop();
        
        if (millis() - lastLoopLog > 500) {
            lastLoopLog = millis();
            // audio.inBufferStatus(); // для отладки: вывод статуса буфера в лог
        }
        
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