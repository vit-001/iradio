/**
 * @file screen_with_handlers.cpp
 * @brief Базовый класс экрана с обработкой событий
 */

#include "screen_with_handlers.h"
#include "ui/screen_manager.h"
#include "esp_log.h"

static const char* TAG = "SCREEN";

// ============================================================================
// Обработка событий энкодера по умолчанию
// ============================================================================

void ScreenWithHandlers::handleEncoderEvent(const EncoderEvent& event)
{
    // Пока используем только первый энкодер
    if (event.encoderId != 1)
    {
        return;
    }

    switch (event.type)
    {
        case EncoderEventType::ButtonLong:
        {
            if (m_manager)
            {
                ESP_LOGD(TAG, "Long press -> next screen");
                m_manager->next();
            }
            break;
        }

        case EncoderEventType::ButtonDouble:
        {
            if (m_manager)
            {
                ESP_LOGD(TAG, "Double press -> previous screen");
                m_manager->previous();
            }
            break;
        }

        default:
            break;
    }
}