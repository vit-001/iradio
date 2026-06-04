/**
 * @file volume_screen.cpp
 * @brief Реализация экрана громкости
 */

#include "volume_screen.h"
#include "config.h"
#include "ui/screen_manager.h"
//#include "drivers/audio/audio_manager.h"
#include "station/stations.h"
#include "messages/audio_messages.h"
#include "drivers/nvs/nvs_manager.h"
#include "messages/audio_to_ui_messages.h"
#include "esp_log.h"
#include "fonts/fonts.h"

static const char* TAG = "VOLUME_SCREEN";

// ==================== Конструктор ====================

VolumeScreen::VolumeScreen(ScreenManager* manager) 
    : Screen(manager) {
}

// ==================== Жизненный цикл ====================

lv_obj_t* VolumeScreen::create() {
    ESP_LOGI(TAG, "Creating VolumeScreen");
    
    // ==================== Создание LVGL экрана ====================
    m_screen = lv_obj_create(NULL);
    lv_obj_set_style_bg_color(m_screen, lv_color_hex(0x000000), 0);
    
    // ==================== Главный flex-контейнер ====================
    // Вертикальное расположение, центрирование по обеим осям
    lv_obj_t* main_cont = lv_obj_create(m_screen);
    lv_obj_set_size(main_cont, LV_HOR_RES, LV_VER_RES);
    lv_obj_set_style_bg_color(main_cont, lv_color_hex(0x000000), 0);
    lv_obj_set_style_border_width(main_cont, 0, 0);
    lv_obj_clear_flag(main_cont, LV_OBJ_FLAG_SCROLLABLE);
    
    lv_obj_set_flex_flow(main_cont, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(main_cont, 
        LV_FLEX_ALIGN_SPACE_AROUND,  // главная ось (вертикаль)
        LV_FLEX_ALIGN_CENTER,  // поперечная ось (горизонталь)
        LV_FLEX_ALIGN_CENTER); // распределение
    
    // ==================== Индикатор режима ====================
    m_modeLabel = lv_label_create(main_cont);
    setModeIndicator(m_modeLabel, "VOLUME", 0x00FF00);  // зелёный
    lv_obj_set_style_pad_bottom(m_modeLabel, 20, 0);
    
    // ==================== Название станции ====================
    m_stationLabel = lv_label_create(main_cont);
    lv_label_set_text(m_stationLabel, "DFM");
    lv_obj_set_style_text_font(m_stationLabel, font_text_medium, 0);
    lv_obj_set_style_text_color(m_stationLabel, lv_color_hex(0xFFFF00), 0);  // жёлтый
    lv_obj_set_style_pad_bottom(m_stationLabel, 20, 0);
    
    // ==================== Название трека ====================
    m_songLabel = lv_label_create(main_cont);
    lv_label_set_text(m_songLabel, "Loading...");
    lv_obj_set_style_text_font(m_songLabel, font_text_small, 0);
    lv_obj_set_style_text_color(m_songLabel, lv_color_hex(0xFFFFFF), 0);  // белый
    lv_obj_set_style_pad_bottom(m_songLabel, 40, 0);
    
    // ==================== Прогресс-бар громкости ====================
    m_volumeBar = lv_bar_create(main_cont);
    lv_obj_set_size(m_volumeBar, 200, 15);
    lv_bar_set_range(m_volumeBar, MIN_VOLUME, MAX_VOLUME);  // диапазон громкости
    lv_obj_set_style_pad_bottom(m_volumeBar, 10, 0);
    
    // ==================== Текст громкости ====================
    m_volumeLabel = lv_label_create(main_cont);
    lv_obj_set_style_text_font(m_volumeLabel, font_accent, 0);
    lv_obj_set_style_text_color(m_volumeLabel, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_pad_bottom(m_volumeLabel, 20, 0);
    
    // ==================== Статус воспроизведения ====================
    m_statusLabel = lv_label_create(main_cont);
    lv_obj_set_style_text_font(m_statusLabel, font_mono_small, 0);
    
    // ==================== Загрузка начальных значений ====================
    // Загружаем сохранённую громкость из NVS
    int savedVolume = NVSManager::getInstance().loadVolume(10);
    // Отправляем команду в AudioTask (через очередь)
    AudioMessage msg;
    msg.type = CMD_SET_VOLUME;
    msg.value1 = savedVolume;
    xQueueSend(audioQueue, &msg, portMAX_DELAY);
    ESP_LOGI(TAG, "Command sent to AudioTask: CMD_SET_VOLUME");
    // UI обновится через EVENT_VOLUME_CHANGED от AudioTask
    // updateVolumeDisplay(AudioManager::getInstance().getVolume());
    
    ESP_LOGI(TAG, "VolumeScreen created");
    return m_screen;
}

// ==================== Обработка событий от AudioTask ====================

void VolumeScreen::handleAudioEvent(const AudioToUIMessage& msg) {
    switch (msg.type) {
        case EVENT_PLAYBACK_INFO: // Обновляем информацию о станции и треке
            // updateSongDisplay(
            //     msg.data.playback.station_name,
            //     msg.data.playback.song_title
            // );
            // // Обновляем статус воспроизведения
            // updateStatusDisplay(msg.data.playback.is_playing);
            // // Громкость тоже может быть в этом сообщении
            // // updateVolumeDisplay(msg.data.playback.volume);
            break;
            
        case EVENT_VOLUME_CHANGED: // Событие изменения громкости
            NVSManager::getInstance().setVolume(msg.data.volume);
            updateVolumeDisplay(msg.data.volume);
            break;

        case EVENT_STATION_CHANGED:{
            int index=StationsManager::getInstance().findIndexByUrl(msg.data.url);
            lv_label_set_text(m_stationLabel, StationsManager::getInstance().getName(index));
            break;
        }
            
        case EVENT_WIFI_STATUS:
            // Опционально: отображать статус WiFi на экране
            // Пока просто логируем
            ESP_LOGD(TAG, "WiFi RSSI: %d dBm", msg.data.wifi.rssi);
            break;
            
        default:
            // Игнорируем остальные события
            break;
    }
}

// ==================== Обработка событий энкодера ====================

void VolumeScreen::onTurnRight() { // Увеличиваем громкость
    // Отправляем команду в AudioTask (через очередь)
    AudioMessage msg;
    msg.type = CMD_VOLUME_UP;
    xQueueSend(audioQueue, &msg, portMAX_DELAY);
    ESP_LOGI(TAG, "Command sent to AudioTask: CMD_VOLUME_UP");
    // UI обновится через EVENT_VOLUME_CHANGED от AudioTask
}

void VolumeScreen::onTurnLeft() {  // Уменьшаем громкость
    // Отправляем команду в AudioTask (через очередь)
    AudioMessage msg;
    msg.type = CMD_VOLUME_DOWN;
    xQueueSend(audioQueue, &msg, portMAX_DELAY);
    ESP_LOGI(TAG, "Command sent to AudioTask: CMD_VOLUME_DOWN");
    // UI обновится через EVENT_VOLUME_CHANGED от AudioTask
}

void VolumeScreen::onShortPress() { // Play/Pause
    // Отправляем команду в AudioTask (через очередь)
    AudioMessage msg;
    msg.type = CMD_PLAY_PAUSE;
    xQueueSend(audioQueue, &msg, portMAX_DELAY);
    ESP_LOGI(TAG, "Command sent to AudioTask: CMD_PLAY_PAUSE");
    // UI обновится через EVENT_PLAYBACK_INFO от AudioTask
}

void VolumeScreen::refresh() {
    ESP_LOGD(TAG, "Refreshing VolumeScreen");
    
    // Принудительно обновляем отображение текущих данных
    // пока ничего не делаем, так как при пробуждении дисплея из сна данные должны быть актуальными
    
    
}

// ==================== Вспомогательные методы ====================

void VolumeScreen::updateVolumeDisplay(int volume) {
    if (!m_volumeBar || !m_volumeLabel) return;
    
    // Ограничиваем значение
    if (volume < MIN_VOLUME) volume = MIN_VOLUME;
    if (volume > MAX_VOLUME) volume = MAX_VOLUME;
    
    // Обновляем прогресс-бар
    lv_bar_set_value(m_volumeBar, volume, LV_ANIM_ON);
    
    // Обновляем текстовую метку
    char buf[32];
    snprintf(buf, sizeof(buf), "VOLUME: %d/%d", volume, MAX_VOLUME);
    lv_label_set_text(m_volumeLabel, buf);
}

void VolumeScreen::updateStatusDisplay(bool is_playing) {
    if (!m_statusLabel) return;
    
    if (is_playing) {
        lv_label_set_text(m_statusLabel, "▶ PLAYING");
        lv_obj_set_style_text_color(m_statusLabel, lv_color_hex(0x00FF00), 0);  // зелёный
    } else {
        lv_label_set_text(m_statusLabel, "⏸ PAUSED");
        lv_obj_set_style_text_color(m_statusLabel, lv_color_hex(0xFF0000), 0);  // красный
    }
}

void VolumeScreen::updateSongDisplay(const char* station, const char* song) {
    if (m_stationLabel && station && strlen(station) > 0) {
        lv_label_set_text(m_stationLabel, station);
    }
    
    if (m_songLabel && song && strlen(song) > 0) {
        lv_label_set_text(m_songLabel, song);
    }
}