
/**
 * @file iencoder_event_handler.h
 * @brief Интерфейс для обработки событий энкодера
 */

#pragma once

#include "drivers/encoder/encoder.h"

/**
 * @class IEncoderEventHandler
 * @brief Интерфейс для обработки событий энкодера
 *
 * Реализуйте этот интерфейс в классах, которые должны реагировать
 * на действия пользователя через энкодеры.
 */
class IEncoderEventHandler {
public:
    virtual ~IEncoderEventHandler() = default;

    /**
     * @brief Обработка события энкодера
     *
     * @param event Событие энкодера
     */
    virtual void handleEncoderEvent(const EncoderEvent& event) {
        // По умолчанию без реакции
    }
};

