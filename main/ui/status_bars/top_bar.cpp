// top_bar.cpp
#include "top_bar.h"

TopBar::TopBar(lv_obj_t* parent) {
    container = lv_obj_create(parent);
    lv_obj_set_size(container, LV_PCT(100), LV_SIZE_CONTENT);
    lv_obj_set_flex_flow(container, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(container, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_bg_opa(container, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(container, 0, 0);
    lv_obj_set_style_pad_all(container, 4, 0);
    
    // Левая группа (Wi-Fi)
    wifi_icon = lv_label_create(container);
    lv_label_set_text(wifi_icon, LV_SYMBOL_WIFI);
    
    // Центр (название станции, бегущая строка)
    station_name = lv_label_create(container);
    lv_label_set_text(station_name, "iRadio");
    lv_obj_set_flex_grow(station_name, 1);
    lv_obj_set_style_text_align(station_name, LV_TEXT_ALIGN_CENTER, 0);
    
    // Правая группа (батарея)
    lv_obj_t* battery_group = lv_obj_create(container);
    lv_obj_set_flex_flow(battery_group, LV_FLEX_FLOW_ROW);
    lv_obj_set_style_bg_opa(battery_group, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(battery_group, 0, 0);
    lv_obj_set_style_pad_all(battery_group, 0, 0);
    
    battery_icon = lv_label_create(battery_group);
    lv_label_set_text(battery_icon, LV_SYMBOL_BATTERY_FULL);
    
    battery_label = lv_label_create(battery_group);
    lv_label_set_text(battery_label, "100%");
}

void TopBar::updateWiFi(bool connected) {
    if (connected) {
        lv_label_set_text(wifi_icon, LV_SYMBOL_WIFI);
    } else {
        lv_label_set_text(wifi_icon, LV_SYMBOL_CLOSE);
    }
}

void TopBar::updateStationName(const char* name) {
    lv_label_set_text(station_name, name);
    // TODO: добавить бегущую строку, если текст длиннее 20 символов
}

void TopBar::updateBattery(int percent) {
    lv_label_set_text_fmt(battery_label, "%d%%", percent);
    
    // Меняем иконку в зависимости от уровня заряда
    if (percent >= 90) {
        lv_label_set_text(battery_icon, LV_SYMBOL_BATTERY_FULL);
    } else if (percent >= 60) {
        lv_label_set_text(battery_icon, LV_SYMBOL_BATTERY_3);
    } else if (percent >= 30) {
        lv_label_set_text(battery_icon, LV_SYMBOL_BATTERY_2);
    } else if (percent >= 10) {
        lv_label_set_text(battery_icon, LV_SYMBOL_BATTERY_1);
    } else {
        lv_label_set_text(battery_icon, LV_SYMBOL_BATTERY_EMPTY);
    }
}