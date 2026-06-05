// top_bar.h
#pragma once
#include "lvgl.h"
#include "ui/iaudio_event_handler.h"

class TopBar : public IAudioEventHandler{
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

    // Реализация метода из IAudioEventHandler для обработки событий от AudioTask
    void handleAudioEvent(const AudioToUIMessage& msg) override;
};