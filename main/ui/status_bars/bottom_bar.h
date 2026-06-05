// bottom_bar.h
#pragma once
#include "lvgl.h"
#include "ui/iaudio_event_handler.h"

class BottomBar : public IAudioEventHandler {
private:
    lv_obj_t* container;
    lv_obj_t* left_container;
    lv_obj_t* center_container;
    lv_obj_t* right_container;
    lv_obj_t* bitrate_label;
    lv_obj_t* volume_icon;
    lv_obj_t* volume_value;
    lv_obj_t* play_icon;
    
public:
    BottomBar(lv_obj_t* parent);
    
    void updateBitrate(int kbps);
    void updateVolume(int percent);
    void updatePlaybackState(bool is_playing);

    // Реализация метода из IAudioEventHandler для обработки событий от AudioTask
    void handleAudioEvent(const AudioToUIMessage& msg) override;
};