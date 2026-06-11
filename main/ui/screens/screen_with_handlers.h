/**
 * @file screen_with_handlers.h
 * @brief Расширенный базовый класс с поддержкой интерфейсов
 */

#ifndef UI_SCREENS_SCREEN_WITH_HANDLERS_H
#define UI_SCREENS_SCREEN_WITH_HANDLERS_H

#include "screen.h"
#include "ui/iaudio_event_handler.h"
#include "ui/iencoder_event_handler.h"

/**
 * @class ScreenWithHandlers
 * @brief Базовый класс для экранов с поддержкой обработчиков событий
 * 
 * Упрощает создание экранов, которым нужны оба типа обработчиков
 */
class ScreenWithHandlers : public Screen, 
                           public IAudioEventHandler, 
                           public IEncoderEventHandler {
public:
    explicit ScreenWithHandlers(ScreenManager* manager, lv_obj_t* parent)
        : Screen(manager, parent) {}
    
    virtual ~ScreenWithHandlers() = default;
    
    // Все методы из интерфейсов уже есть, но могут быть переопределены

    /**
     * @brief Обработка событий энкодера по умолчанию
     *
     * Длинное нажатие → следующий экран.
     * Двойное нажатие → предыдущий экран.
     *
     * Остальные события игнорируются и могут быть
     * переопределены в наследниках.
     */
    void handleEncoderEvent(const EncoderEvent& event) override;

};

#endif // UI_SCREENS_SCREEN_WITH_HANDLERS_H