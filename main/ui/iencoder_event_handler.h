/**
 * @file iencoder_event_handler.h
 * @brief Интерфейс для обработки событий энкодера
 */

#ifndef UI_INTERFACES_IENCODER_EVENT_HANDLER_H
#define UI_INTERFACES_IENCODER_EVENT_HANDLER_H

/**
 * @class IEncoderEventHandler
 * @brief Интерфейс для обработки событий энкодера
 * 
 * Реализуйте этот интерфейс в классах, которые должны реагировать
 * на действия пользователя через энкодер (статус-бары, виджеты, экраны)
 */
class IEncoderEventHandler {
public:
    virtual ~IEncoderEventHandler() = default;
    
    /**
     * @brief Поворот энкодера вправо
     */
    virtual void onTurnRight() {
        // По умолчанию без реакции
    }
    
    /**
     * @brief Поворот энкодера влево
     */
    virtual void onTurnLeft() {
        // По умолчанию без реакции
    }
    
    /**
     * @brief Короткое нажатие
     */
    virtual void onShortPress() {
        // По умолчанию без реакции
    }
    
    /**
     * @brief Долгое нажатие (обычно 800+ мс)
     */
    virtual void onLongPress() {
        // По умолчанию без реакции
    }
    
    /**
     * @brief Двойное нажатие
     */
    virtual void onDoublePress() {
        // По умолчанию без реакции
    }
};

#endif // UI_INTERFACES_IENCODER_EVENT_HANDLER_H