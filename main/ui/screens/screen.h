/**
 * @file screen.h
 * @brief Базовый класс для всех экранов - только отображение LVGL
 */

#ifndef UI_SCREENS_SCREEN_H
#define UI_SCREENS_SCREEN_H

#include "lvgl.h"

// Forward declaration
class ScreenManager;

/**
 * @class Screen
 * @brief Базовый класс для отображения экранов через LVGL
 */
class Screen {
public:
    /**
     * @brief Конструктор
     * @param manager указатель на менеджер экранов
     * @param parent указатель на родительский LVGL объект
     */
    explicit Screen(ScreenManager* manager, lv_obj_t* parent) 
        : m_manager(manager), m_parent(parent) {}
    
    virtual ~Screen() = default;
    
    // ==================== Жизненный цикл ====================
    
    /**
     * @brief Создание LVGL экрана и всех виджетов
     * @return указатель на LVGL объект экрана
     */
    virtual lv_obj_t* create() = 0;
    
    /**
     * @brief Принудительное обновление содержимого экрана
     */
    virtual void refresh() = 0;
    
    // ==================== Управление видимостью ====================
    
    /**
     * @brief Показать экран
     */
    virtual void show() { 
        if (main_cont != nullptr) {
            lv_obj_clear_flag(main_cont, LV_OBJ_FLAG_HIDDEN); 
        }
    }
    
    /**
     * @brief Скрыть экран
     */
    virtual void hide() { 
        if (main_cont != nullptr) {
            lv_obj_add_flag(main_cont, LV_OBJ_FLAG_HIDDEN); 
        }
    }
    
    // ==================== Геттеры ====================
    
    lv_obj_t* getLvglScreen() const { return main_cont; }
    
protected:
    lv_obj_t* m_parent = nullptr;
    lv_obj_t* main_cont = nullptr;
    ScreenManager* m_manager = nullptr;
    
    /**
     * @brief Установить индикатор режима
     */
    void setModeIndicator(lv_obj_t* label, const char* name, uint32_t color) {
        if (label) {
            lv_label_set_text(label, name);
            lv_obj_set_style_text_color(label, lv_color_hex(color), 0);
        }
    }
};

#endif // UI_SCREENS_SCREEN_H