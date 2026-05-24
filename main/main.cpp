#include "Arduino.h"
#include "Audio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "config.h"

Audio audio;

// Глобальные переменные
int volume = DEFAULT_VOLUME;
bool playing = true;

// Очередь сообщений между ядрами
QueueHandle_t audioCommandQueue;

// Handle задач для мониторинга
TaskHandle_t audioTaskHandle = NULL;
TaskHandle_t uiTaskHandle = NULL;

// Структура команды для аудио задачи
struct AudioCommand {
    uint8_t type;      // 1: volume, 2: playpause, 3: station
    int value;
};

// === ЗАДАЧА АУДИО (Ядро 0 - PRO_CPU) ===
void audioTask(void* parameter) {
    Serial.print("Audio task running on core ");
    Serial.println(xPortGetCoreID());
    
    // Настройка аудио
    audio.setPinout(I2S_BCLK, I2S_LRC, I2S_DOUT);
    audio.setVolume(volume);
    audio.setTone(TONE_BASS, TONE_MID, TONE_TREBLE);
    
    // WiFi и подключение к потоку
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("\nWiFi connected!");
    
    audio.connecttohost(radio_url);
    playing = true;
    
    // Основной цикл аудио задачи
    while (true) {
        // Проверяем команды от UI задачи
        AudioCommand cmd;
        if (xQueueReceive(audioCommandQueue, &cmd, 0) == pdTRUE) {
            switch (cmd.type) {
                case 1:  // Изменение громкости
                    volume = cmd.value;
                    audio.setVolume(volume);
                    Serial.print("Volume: ");
                    Serial.println(volume);
                    break;
                case 2:  // Play/Pause
                    if (playing) {
                        audio.stopSong();
                        playing = false;
                        Serial.println("⏸ Paused");
                    } else {
                        audio.connecttohost(radio_url);
                        playing = true;
                        Serial.println("▶ Resumed");
                    }
                    break;
                case 3:  // Смена станции (для будущего)
                    // audio.connecttohost(stations[cmd.value]);
                    break;
            }
        }
        
        audio.loop();
        delay(5);  // Небольшая задержка для стабильности
    }
}

// === ЗАДАЧА UI (Ядро 1 - APP_CPU) ===
void uiTask(void* parameter) {
    Serial.print("UI task running on core ");
    Serial.println(xPortGetCoreID());
    
    // Настройка пинов энкодера
    pinMode(ENC_A, INPUT_PULLUP);
    pinMode(ENC_B, INPUT_PULLUP);
    pinMode(ENC_BTN, INPUT_PULLUP);
    
    int lastA = HIGH;
    unsigned long lastTurnTime = 0;
    unsigned long lastButtonTime = 0;
    
    while (true) {
        // Чтение энкодера
        int a = digitalRead(ENC_A);
        int b = digitalRead(ENC_B);
        
        if (a != lastA && a == LOW && (millis() - lastTurnTime) > DEBOUNCE_DELAY) {
            lastTurnTime = millis();
            
            AudioCommand cmd;
            cmd.type = 1;  // volume
            
            if (b == HIGH) {
                cmd.value = max(0, volume - 1);
            } else {
                cmd.value = min(21, volume + 1);
            }
            
            if (cmd.value != volume) {
                xQueueSend(audioCommandQueue, &cmd, portMAX_DELAY);
            }
        }
        lastA = a;
        
        // Чтение кнопки
        if (digitalRead(ENC_BTN) == LOW && (millis() - lastButtonTime) > BUTTON_DELAY) {
            lastButtonTime = millis();
            
            AudioCommand cmd;
            cmd.type = 2;  // play/pause
            xQueueSend(audioCommandQueue, &cmd, portMAX_DELAY);
            
            while (digitalRead(ENC_BTN) == LOW) delay(10);
        }
        
        delay(5);
    }
}

void setup() {
    Serial.begin(115200);
    Serial.println("Starting Dual-Core Internet Radio");
    
    // Создание очереди для команд
    audioCommandQueue = xQueueCreate(10, sizeof(AudioCommand));
    
    // Создание задачи аудио на ядре 0 (PRO_CPU)
    xTaskCreatePinnedToCore(
        audioTask,              // Функция задачи
        "AudioTask",            // Имя задачи
        8192,                   // Размер стека (байт)
        NULL,                   // Параметры
        2,                      // Приоритет (высокий)
        &audioTaskHandle,       // Handle задачи
        0                       // Ядро 0 (PRO_CPU)
    );
    
    // Создание задачи UI на ядре 1 (APP_CPU)
    xTaskCreatePinnedToCore(
        uiTask,                 // Функция задачи
        "UITask",               // Имя задачи
        4096,                   // Размер стека (байт)
        NULL,                   // Параметры
        1,                      // Приоритет (обычный)
        &uiTaskHandle,          // Handle задачи
        1                       // Ядро 1 (APP_CPU)
    );
}

void loop() {
    // Мониторинг состояния системы
    static unsigned long lastCheck = 0;
    if (millis() - lastCheck > 10000) {  // Каждые 10 секунд
        lastCheck = millis();
        
        // Информация о свободной памяти
        Serial.printf("Free heap: %d bytes\n", esp_get_free_heap_size());
        
        // Информация о стеке задач (если handle доступны)
        if (audioTaskHandle != NULL) {
            Serial.printf("Audio task free stack: %d bytes\n", 
                uxTaskGetStackHighWaterMark(audioTaskHandle));
        }
        if (uiTaskHandle != NULL) {
            Serial.printf("UI task free stack: %d bytes\n", 
                uxTaskGetStackHighWaterMark(uiTaskHandle));
        }
        
        // Статус воспроизведения
        Serial.printf("Status: %s | Volume: %d\n", 
            playing ? "Playing" : "Paused", volume);
    }
    
    // Отдаём процессорное время другим задачам
    vTaskDelay(pdMS_TO_TICKS(1000));
}

// Callback для информации о потоке (вызывается из audioTask)
void audio_info(const char* info) {
    Serial.printf("Info: %s\n", info);
}

void audio_id3data(const char* info) {
    Serial.printf("Now playing: %s\n", info);
}

void audio_eof_mp3(const char* info) {
    Serial.printf("End of stream: %s\n", info);
}