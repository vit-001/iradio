// root_screen.h
#pragma once
#include "lvgl.h"

class RootScreen {
private:
    lv_obj_t* screen;           // Корневой LVGL экран
    lv_obj_t* top_bar_container;
    lv_obj_t* content_container;
    lv_obj_t* bottom_bar_container;

    // Сброс дефолтных стилей LVGL (границы, скругления, внутренние отступы)
    void reset_styles(lv_obj_t* obj);
    
public:
    RootScreen();
    
    void Show();
    
    // Геттеры для доступа к контейнерам
    lv_obj_t* getTopBarContainer() const { return top_bar_container; }
    lv_obj_t* getContentContainer() const { return content_container; }
    lv_obj_t* getBottomBarContainer() const { return bottom_bar_container; }
    lv_obj_t* getScreen() const { return screen; }
};