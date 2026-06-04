// bottom_bar.cpp
#include "bottom_bar.h"

BottomBar::BottomBar(lv_obj_t* parent) {
    container = lv_obj_create(parent);
    lv_obj_set_size(container, LV_PCT(100), LV_SIZE_CONTENT);
    lv_obj_set_flex_flow(container, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(container, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_bg_opa(container, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(container, 0, 0);
    lv_obj_set_style_pad_all(container, 4, 0);
    
    // Левая группа (битрейт)
    bitrate_label = lv_label_create(container);
    lv_label_set_text(bitrate_label, "0 kbps");
    
    // Центр (индикатор воспроизведения)
    play_icon = lv_label_create(container);
    lv_label_set_text(play_icon, LV_SYMBOL_PLAY);
    lv_obj_set_style_text_color(play_icon, lv_color_hex(0x00FF00), 0);
    
    // Правая группа (громкость)
    lv_obj_t* volume_group = lv_obj_create(container);
    lv_obj_set_flex_flow(volume_group, LV_FLEX_FLOW_ROW);
    lv_obj_set_style_bg_opa(volume_group, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(volume_group, 0, 0);
    lv_obj_set_style_pad_all(volume_group, 0, 0);
    
    volume_icon = lv_label_create(volume_group);
    lv_label_set_text(volume_icon, LV_SYMBOL_VOLUME_MID);
    
    volume_value = lv_label_create(volume_group);
    lv_label_set_text(volume_value, "50%");
}

void BottomBar::updateBitrate(int kbps) {
    if (kbps > 0) {
        lv_label_set_text_fmt(bitrate_label, "%d kbps", kbps);
    } else {
        lv_label_set_text(bitrate_label, "--- kbps");
    }
}

void BottomBar::updateVolume(int percent) {
    lv_label_set_text_fmt(volume_value, "%d%%", percent);
    
    // Меняем иконку громкости
    if (percent == 0) {
        lv_label_set_text(volume_icon, LV_SYMBOL_MUTE);
    } else if (percent < 40) {
        lv_label_set_text(volume_icon, LV_SYMBOL_MUTE);
    } else if (percent < 70) {
        lv_label_set_text(volume_icon, LV_SYMBOL_VOLUME_MID);
    } else {
        lv_label_set_text(volume_icon, LV_SYMBOL_VOLUME_MAX);
    }
}

void BottomBar::updatePlaybackState(bool is_playing) {
    if (is_playing) {
        lv_label_set_text(play_icon, LV_SYMBOL_PLAY);
        lv_obj_set_style_text_color(play_icon, lv_color_hex(0x00FF00), 0);
    } else {
        lv_label_set_text(play_icon, LV_SYMBOL_PAUSE);
        lv_obj_set_style_text_color(play_icon, lv_color_hex(0xFF6666), 0);
    }
}