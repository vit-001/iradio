// bottom_bar.h
#pragma once
#include "lvgl.h"

class BottomBar {
private:
    lv_obj_t* container;
    lv_obj_t* bitrate_label;
    lv_obj_t* volume_icon;
    lv_obj_t* volume_value;
    lv_obj_t* play_icon;
    
public:
    BottomBar(lv_obj_t* parent);
    
    void updateBitrate(int kbps);
    void updateVolume(int percent);
    void updatePlaybackState(bool is_playing);
};