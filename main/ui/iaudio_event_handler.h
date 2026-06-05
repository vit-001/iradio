/**
 * @file iaudio_event_handler.h
 * @brief Интерфейс для обработки событий от AudioTask
 */

#ifndef UI_INTERFACES_IAUDIO_EVENT_HANDLER_H
#define UI_INTERFACES_IAUDIO_EVENT_HANDLER_H

#include "messages/audio_to_ui_messages.h"

/**
 * @class IAudioEventHandler
 * @brief Интерфейс для обработки аудио-событий
 * 
 * Реализуйте этот интерфейс в классах, которые должны реагировать
 * на события от AudioTask (статус-бары, виджеты, экраны)
 */
class IAudioEventHandler {
public:
    virtual ~IAudioEventHandler() = default;
    
    /**
     * @brief Обработка события от AudioTask
     * @param msg сообщение от AudioTask
     * 
     * Базовая реализация - пустышка (не реагирует на события)
     */
    virtual void handleAudioEvent(const AudioToUIMessage& msg) {
        (void)msg;  // подавляем warning о неиспользуемом параметре
        // По умолчанию игнорируем все сообщения
    }
};

#endif // UI_INTERFACES_IAUDIO_EVENT_HANDLER_H