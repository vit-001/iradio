// top_bar.h
#pragma once
#include "lvgl.h"

class TopBar {
private:
    lv_obj_t* container;
    lv_obj_t* wifi_icon;
    lv_obj_t* station_name;
    lv_obj_t* battery_icon;
    lv_obj_t* battery_label;
    
public:
    TopBar(lv_obj_t* parent);
    
    void updateWiFi(bool connected);
    void updateStationName(const char* name);
    void updateBattery(int percent);
};