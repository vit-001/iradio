/**
 * @file audio_to_ui_messages.h
 * @brief Сообщения от AudioTask в UI задачу
 * 
 * Содержит структуры для передачи различной информации:
 * - статус WiFi (уровень сигнала)
 * - информация о воспроизведении (станция, трек, громкость)
 * - спектрограмма (для визуализации)
 * - общие события (ошибки, уведомления)
 * 
 * Эта очередь читается UI задачей, а ScreenManager рассылает события всем экранам.
 */

#ifndef AUDIO_TO_UI_MESSAGES_H
#define AUDIO_TO_UI_MESSAGES_H

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "esp_wifi_types.h"
#include "drivers/audio/audio_types.h"

/**
 * @enum AudioToUIEventType
 * @brief Типы событий от AudioTask к UI
 */
enum AudioToUIEventType : uint8_t {
    // ===== Информация о воспроизведении =====
    EVENT_PLAYBACK_INFO = 0,   ///< Информация о текущем воспроизведении
    EVENT_TRACK_CHANGED,       ///< Сменился трек
    EVENT_STATION_CHANGED,     ///< Сменилась станция
    EVENT_VOLUME_CHANGED,      ///< Изменилась громкость
    EVENT_BITRATE_CHANGED,      ///< Изменился битрейт
    EVENT_PLAYBACK_STATE_CHANGED, ///< Изменилось состояние воспроизведения (Playing, Paused, Reconnecting)
    
    // ===== Статус WiFi =====
    EVENT_WIFI_STATUS,         ///< Статус подключения WiFi
    EVENT_WIFI_RSSI,           ///< Уровень сигнала WiFi
    
    // ===== Эквалайзер =====
    EVENT_EQ_VALUES,           ///< Значения эквалайзера (Bass, Mid, Treble)
    EVENT_SPECTRUM_DATA,       ///< Данные спектрограммы (в будущем)
    
    // ===== Общие события =====
    EVENT_ERROR,               ///< Ошибка
    EVENT_NOTIFICATION,        ///< Уведомление
    EVENT_BUFFERING,           ///< Буферизация потока
};

/**
 * @struct PlaybackInfo
 * @brief Информация о текущем воспроизведении
 */
struct PlaybackInfo {
    char station_name[64];     ///< Название радиостанции
    char song_title[128];      ///< Название трека
    int volume;                ///< Текущая громкость (0-21)
    bool is_playing;           ///< Идёт ли воспроизведение
    bool is_buffering;         ///< Идёт ли буферизация
    uint32_t bitrate;          ///< Битрейт (kbps)
};


/**
 * @struct PlaybackState
 * @brief Состояние воспроизведения
 */
struct PlaybackStateInfo {
    PlaybackState state ;     ///< Идёт ли воспроизведение
    bool is_playing;         ///< Идёт ли воспроизведение
};


/**
 * @struct WiFiStatus
 * @brief Информация о WiFi подключении
 */
struct WiFiStatus {
    bool is_connected;         ///< Подключено ли к WiFi
    int8_t rssi;               ///< Уровень сигнала (-100..0 dBm)
};

/**
 * @struct EQValues
 * @brief Значения эквалайзера
 */
struct EQValues {
    int bass;                  ///< Басы (-40..+6 dB)
    int mid;                   ///< Средние (-40..+6 dB)
    int treble;                ///< Высокие (-40..+6 dB)
};

/**
 * @struct SpectrumData
 * @brief Данные спектрограммы (в будущем)
 */
struct SpectrumData {
    uint16_t bands[32];        ///< Амплитуды частотных полос (0-255)
    uint8_t band_count;        ///< Количество полос (max 32)
};

/**
 * @struct Notification
 * @brief Текстовое уведомление
 */
struct Notification {
    char message[128];         ///< Текст уведомления
    uint32_t duration_ms;      ///< Длительность показа (0 = постоянно)
    uint32_t color;            ///< Цвет текста (RGB565)
};

/**
 * @struct AudioToUIMessage
 * @brief Основная структура сообщения от AudioTask к UI
 */
struct AudioToUIMessage {
    AudioToUIEventType type;   ///< Тип события
    
    union {
        PlaybackInfo playbackInfo;      ///< EVENT_PLAYBACK_INFO
        PlaybackStateInfo playbackState;///< EVENT_PLAYBACK_STATE_CHANGED
        char url[256];              ///< EVENT_STATION_CHANGED
        WiFiStatus wifi;            ///< EVENT_WIFI_STATUS
        EQValues eq;                ///< EVENT_EQ_VALUES
        SpectrumData spectrum;      ///< EVENT_SPECTRUM_DATA
        Notification notification;  ///< EVENT_NOTIFICATION / EVENT_ERROR
        int8_t rssi;                ///< EVENT_WIFI_RSSI (только сигнал)
        int volume;                 ///< EVENT_VOLUME_CHANGED
        uint32_t bitrate;           ///< EVENT_BITRATE_CHANGED
    } data;
};

// Глобальная очередь (объявляется в main.cpp, используется в audio_task и ui_task)
extern QueueHandle_t audioToUIQueue;

#endif // AUDIO_TO_UI_MESSAGES_H