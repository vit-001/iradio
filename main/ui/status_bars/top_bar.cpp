// top_bar.cpp
#include "top_bar.h"
#include "station/stations.h"
#include "fonts/fonts.h"

TopBar::TopBar(lv_obj_t* parent) {
    container = lv_obj_create(parent);

    lv_obj_set_size(container, LV_PCT(100), 30);

    lv_obj_set_flex_flow(container, LV_FLEX_FLOW_ROW);

    lv_obj_set_flex_align(
        container,
        LV_FLEX_ALIGN_START,
        LV_FLEX_ALIGN_CENTER,
        LV_FLEX_ALIGN_CENTER);

    lv_obj_set_style_bg_opa(container, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(container, 0, 0);

    lv_obj_set_style_pad_left(container, 4, 0);
    lv_obj_set_style_pad_right(container, 4, 0);

    lv_obj_set_style_pad_column(container, 12, 0);

    //
    // WiFi
    //
    wifi_icon = lv_label_create(container);
    lv_label_set_text(wifi_icon, LV_SYMBOL_WIFI);
    lv_obj_set_style_text_color(wifi_icon, lv_color_hex(0xFFFFFF), 0);

    //
    // Название станции
    //
    station_name = lv_label_create(container);
    lv_label_set_text(station_name, "iRadio");
    lv_obj_set_style_text_font(station_name, font_accent, 0);
    lv_obj_set_style_text_color(station_name, lv_color_hex(0xFFFFFF), 0);

    //
    // Батарея
    //
    battery_icon = lv_label_create(container);
    lv_label_set_text(battery_icon, LV_SYMBOL_BATTERY_FULL);
    lv_obj_set_style_text_color(battery_icon, lv_color_hex(0xFFFFFF), 0);

    battery_label = lv_label_create(container);
    lv_label_set_text(battery_label, "100%");
    lv_obj_set_style_text_font(battery_label, font_accent, 0);
    lv_obj_set_style_text_color(battery_label, lv_color_hex(0xFFFFFF), 0);
}

void TopBar::handleAudioEvent(const AudioToUIMessage& msg) {
    switch (msg.type) {
        case EVENT_WIFI_STATUS:
            updateWiFi(msg.data.wifi.is_connected);
            break;
        
        case EVENT_STATION_CHANGED:{
            int index=StationsManager::getInstance().findIndexByUrl(msg.data.url);
            // lv_label_set_text(station_name, StationsManager::getInstance().getName(index));
            updateStationName(StationsManager::getInstance().getName(index));
            break;
            }
        default:
            // Игнорируем остальные события (громкость, битрейт и т.д.)
            break;      
        }
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