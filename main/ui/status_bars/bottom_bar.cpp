// bottom_bar.cpp
#include "bottom_bar.h"
#include "config.h"

BottomBar::BottomBar(lv_obj_t* parent) {
    container = lv_obj_create(parent);

    lv_obj_set_size(container, LV_PCT(100), 25);

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
    lv_obj_set_style_pad_top(container, 0, 0);
    lv_obj_set_style_pad_bottom(container, 0, 0);

    // Расстояние между элементами
    lv_obj_set_style_pad_column(container, 30, 0);

    //
    // Битрейт
    //
    bitrate_label = lv_label_create(container);
    lv_label_set_text(bitrate_label, "-- kbps");
    lv_obj_set_style_text_color(
        bitrate_label,
        lv_color_hex(0xFFFFFF),
        0);

    //
    // Индикатор воспроизведения
    //
    play_icon = lv_label_create(container);
    lv_label_set_text(play_icon, LV_SYMBOL_PLAY);
    lv_obj_set_style_text_color(
        play_icon,
        lv_color_hex(0xFFFFFF),
        0);

    //
    // Иконка громкости
    //
    volume_icon = lv_label_create(container);
    lv_label_set_text(volume_icon, LV_SYMBOL_VOLUME_MID);
    lv_obj_set_style_text_color(
        volume_icon,
        lv_color_hex(0xFFFFFF),
        0);

    //
    // Значение громкости
    //
    volume_value = lv_label_create(container);
    lv_label_set_text(volume_value, "50%");
    lv_obj_set_style_text_color(
        volume_value,
        lv_color_hex(0xFFFFFF),
        0);
}

void BottomBar::handleAudioEvent(const AudioToUIMessage& msg){
    switch (msg.type) {
        case EVENT_BITRATE_CHANGED:
            updateBitrate(msg.data.bitrate/1024); // Конвертируем в kbps
            break;
        case EVENT_VOLUME_CHANGED:
            updateVolume(msg.data.volume*100/(MAX_VOLUME-MIN_VOLUME)); // Конвертируем в проценты
            break;
        case EVENT_PLAYBACK_INFO:
            updatePlaybackState(msg.data.playbackState.is_playing);
            break;
        default:
            break;
    }
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
    } else if (percent < 60) {
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