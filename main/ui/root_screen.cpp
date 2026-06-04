// root_screen.cpp
#include "root_screen.h"

void RootScreen::reset_styles(lv_obj_t* obj) {
    lv_obj_set_style_pad_all(obj, 0, 0);
    lv_obj_set_style_border_width(obj, 0, 0);
    lv_obj_set_style_radius(obj, 0, 0);
    // Опционально: делаем фон прозрачным, чтобы стилизовать в других классах
    lv_obj_set_style_bg_opa(obj, LV_OPA_TRANSP, 0);
}

RootScreen::RootScreen() {
    screen = lv_scr_act();
    lv_obj_set_size(screen, LV_PCT(100), LV_PCT(100));
    lv_obj_set_style_bg_color(screen, lv_color_hex(0x000000), 0);
    
    // Настраиваем Flex-пространство (вертикальное выравнивание)
    lv_obj_set_layout(screen, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(screen, LV_FLEX_FLOW_COLUMN);

    // Убираем внешние отступы и границы, чтобы занять 100% экрана
    lv_obj_set_style_pad_all(screen, 0, 0);
    lv_obj_set_style_pad_row(screen, 0, 0); // Нет зазоров между строками
    lv_obj_set_style_border_width(screen, 0, 0);
    lv_obj_set_style_radius(screen, 0, 0);        
    
    // Создаем три контейнера
    top_bar_container = lv_obj_create(screen);
    lv_obj_set_size(top_bar_container, LV_PCT(100), 30);
    reset_styles(top_bar_container);
    
    content_container = lv_obj_create(screen);
    lv_obj_set_width(content_container, LV_PCT(100));
    lv_obj_set_flex_grow(content_container, 1);
    reset_styles(content_container);
    
    bottom_bar_container = lv_obj_create(screen);
    lv_obj_set_size(bottom_bar_container, LV_PCT(100), 25);
    reset_styles(bottom_bar_container);
}

void RootScreen::Show() {
    lv_scr_load(screen);
}