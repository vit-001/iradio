/**
 * @file screen.h
 * @brief Базовый класс для всех экранов интерфейса
 * 
 * Каждый экран:
 * - Создаёт свои LVGL виджеты
 * - Обрабатывает события энкодера
 * - Реагирует на события от AudioTask
 * - Сам решает, как отображать свой индикатор режима
 */

#ifndef UI_SCREENS_SCREEN_H
#define UI_SCREENS_SCREEN_H

#include "lvgl.h"
#include "messages/audio_to_ui_messages.h"

// Forward declaration
class ScreenManager;

/**
 * @class Screen
 * @brief Абстрактный базовый класс для всех экранов
 */
class Screen {
public:
    /**
     * @brief Конструктор
     * @param manager указатель на менеджер экранов (для навигации)
     * @param parent указатель на контейнер/родительского объекта, в котором будет размещён экран
     */
    explicit Screen(ScreenManager* manager, lv_obj_t* parent) : m_manager(manager), m_parent(parent) {}
    
    virtual ~Screen() = default;
    
    // ==================== Жизненный цикл ====================
    
    /**
     * @brief Создание LVGL экрана и всех виджетов
     * @return указатель на LVGL объект экрана
     * 
     * Вызывается один раз при инициализации.
     * Здесь создаются все виджеты, настраиваются стили и компоновка.
     */
    virtual lv_obj_t* create() = 0;
    
    // ==================== События от AudioTask ====================
    
    /**
     * @brief Обработка события от AudioTask
     * @param msg сообщение от AudioTask
     * 
     * Вызывается ScreenManager при получении нового сообщения из очереди.
     * Каждый экран сам решает, на какие события реагировать.
     * 
     * Пример:
     * void MyScreen::handleAudioEvent(const AudioToUIMessage& msg) {
     *     switch (msg.type) {
     *         case EVENT_PLAYBACK_INFO:
     *             lv_label_set_text(m_songLabel, msg.data.playback.song_title);
     *             break;
     *         default:
     *             break;
     *     }
     * }
     */
    virtual void handleAudioEvent(const AudioToUIMessage& msg);
    
    // ==================== События энкодера ====================
    
    /**
     * @brief Поворот энкодера вправо (по часовой стрелке)
     */
    virtual void onTurnRight() = 0;
    
    /**
     * @brief Поворот энкодера влево (против часовой стрелки)
     */
    virtual void onTurnLeft() = 0;
    
    /**
     * @brief Короткое нажатие кнопки энкодера
     */
    virtual void onShortPress() = 0;
    
    /**
     * @brief Долгое нажатие (обычно 800+ мс)
     * @note По умолчанию переключает на следующий экран через ScreenManager
     */
    virtual void onLongPress();
    
    /**
     * @brief Двойное нажатие
     * @note По умолчанию переключает на следующий экран через ScreenManager
     */
    virtual void onDoublePress();
    
    // ==================== Переключение экранов ====================
    
    /**
     * @brief Показать экран
     */
    void Show() { 
        if (main_cont != nullptr) {
            lv_obj_clear_flag(main_cont, LV_OBJ_FLAG_HIDDEN); 
        }
    }
    
    /**
     * @brief Скрыть экран
     */
    void Hide() { 
        if (main_cont != nullptr) {
            lv_obj_add_flag(main_cont, LV_OBJ_FLAG_HIDDEN); 
        }
    }
    
    /**
     * @brief Принудительное обновление содержимого экрана
     * 
     * Вызывается после пробуждения дисплея из сна,
     * чтобы восстановить изображение.
     */
    virtual void refresh() = 0;


    /**
     * @brief Получить LVGL объект экрана
     * @return указатель на lv_obj_t
     */
    lv_obj_t* getLvglScreen() const { return main_cont; }
 
    


protected:
    lv_obj_t* m_parent = nullptr;    ///< Родительский контейнер для размещения экрана
    lv_obj_t* main_cont = nullptr;      ///< LVGL объект главного контейнера
    ScreenManager* m_manager = nullptr; ///< Указатель на менеджер (для навигации)
    
    /**
     * @brief Установить индикатор режима (имя и цвет)
     * 
     * Каждый экран сам решает, как и где отображать свой индикатор.
     * Этот метод просто устанавливает текст и цвет для переданного лейбла.
     * 
     * @param label объект лейбла (обычно создаётся экраном в create())
     * @param name текст режима
     * @param color цвет в формате RGB565
     */
    void setModeIndicator(lv_obj_t* label, const char* name, uint32_t color) {
        if (label) {
            lv_label_set_text(label, name);
            lv_obj_set_style_text_color(label, lv_color_hex(color), 0);
        }
    }
};

#endif // UI_SCREENS_SCREEN_H