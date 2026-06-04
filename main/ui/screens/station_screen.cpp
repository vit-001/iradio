/**
 * @file station_screen.cpp
 * @brief Реализация экрана выбора радиостанции
 */

#include "station_screen.h"
#include "ui/screen_manager.h"
// #include "drivers/audio/audio_manager.h"
#include "station/stations.h"
#include "drivers/nvs/nvs_manager.h"
#include "messages/audio_messages.h"
#include "messages/audio_to_ui_messages.h"
#include "fonts/fonts.h"
#include "esp_log.h"

static const char* TAG = "STATION_SCREEN";

// extern QueueHandle_t audioQueue;


// ==================== Конструктор ====================

StationScreen::StationScreen(ScreenManager* manager) 
    : Screen(manager) {
}

// ==================== Жизненный цикл ====================

lv_obj_t* StationScreen::create() {
    ESP_LOGI(TAG, "Creating StationScreen");
    
    // ==================== Создание LVGL экрана ====================
    m_screen = lv_obj_create(NULL);
    lv_obj_set_style_bg_color(m_screen, lv_color_hex(0x000000), 0);
    
    // ==================== Главный flex-контейнер ====================
    lv_obj_t* main_cont = lv_obj_create(m_screen);
    lv_obj_set_size(main_cont, LV_HOR_RES, LV_VER_RES);
    lv_obj_set_style_bg_color(main_cont, lv_color_hex(0x000000), 0);
    lv_obj_set_style_border_width(main_cont, 0, 0);
    lv_obj_clear_flag(main_cont, LV_OBJ_FLAG_SCROLLABLE);
    
    lv_obj_set_flex_flow(main_cont, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(main_cont, 
        LV_FLEX_ALIGN_SPACE_AROUND,
        LV_FLEX_ALIGN_CENTER,
        LV_FLEX_ALIGN_CENTER);
    
    // ==================== Индикатор режима ====================
    m_modeLabel = lv_label_create(main_cont);
    setModeIndicator(m_modeLabel, "STATION", 0xFFFF00);  // жёлтый
    lv_obj_set_style_pad_bottom(m_modeLabel, 20, 0);
    
    // ==================== Предыдущая станция (подсказка сверху) ====================
    m_previousLabel = lv_label_create(main_cont);
    lv_obj_set_style_text_font(m_previousLabel, font_text_medium, 0);
    lv_obj_set_style_text_color(m_previousLabel, lv_color_hex(0x888888), 0);  // серый
    lv_obj_set_style_pad_bottom(m_previousLabel, 10, 0);
    
    // ==================== Текущая станция (крупно) ====================
    m_stationNameLabel = lv_label_create(main_cont);
    lv_obj_set_style_text_font(m_stationNameLabel, font_text_medium, 0);
    lv_obj_set_style_text_color(m_stationNameLabel, lv_color_hex(0xFFFF00), 0);  // жёлтый
    lv_obj_set_style_pad_bottom(m_stationNameLabel, 10, 0);
    
    // ==================== Следующая станция (подсказка снизу) ====================
    m_nextLabel = lv_label_create(main_cont);
    lv_obj_set_style_text_font(m_nextLabel, font_text_medium, 0);
    lv_obj_set_style_text_color(m_nextLabel, lv_color_hex(0x888888), 0);  // серый
    lv_obj_set_style_pad_bottom(m_nextLabel, 20, 0);
    
    // ==================== Подсказка ====================
    m_hintLabel = lv_label_create(main_cont);
    lv_label_set_text(m_hintLabel, "PRESS TO SELECT");
    lv_obj_set_style_text_font(m_hintLabel, font_text_small, 0);
    lv_obj_set_style_text_color(m_hintLabel, lv_color_hex(0x00FF00), 0);  // зелёный
    lv_obj_set_style_pad_bottom(m_hintLabel, 10, 0);
    
    // ==================== Индикатор воспроизведения ====================
    m_playingIndicator = lv_label_create(main_cont);
    lv_label_set_text(m_playingIndicator, "▶ NOW PLAYING");
    lv_obj_set_style_text_font(m_playingIndicator, font_text_small, 0);
    lv_obj_set_style_text_color(m_playingIndicator, lv_color_hex(0x00FF00), 0);
    lv_obj_add_flag(m_playingIndicator, LV_OBJ_FLAG_HIDDEN);  // скрыт по умолчанию
    
    // ==================== Загрузка начальных значений ====================
    // Загружаем сохранённую станцию из NVS
    int saved_station = NVSManager::getInstance().loadStation(0);
    switchToStation(saved_station);
    
    ESP_LOGI(TAG, "StationScreen created");
    return m_screen;
}

// // Вспомогательная функция для получения индекса текущей станции из информации
// static int getCurrentStationIndexFromInfo(const AudioToUIMessage& msg) {
//     // Здесь нужно сравнить msg.data.playback.station_name со списком stations[]
//     // Пока возвращаем 0
//     (void)msg;
//     return 0;
// }


// ==================== Обработка событий от AudioTask ====================

void StationScreen::handleAudioEvent(const AudioToUIMessage& msg) {
    
    switch (msg.type) {
        case EVENT_PLAYBACK_INFO:
            // // Если текущая станция совпадает с играющей, показываем индикатор
            // if (m_currentStationIndex == getCurrentStationIndexFromInfo(msg)) {
            //     lv_obj_clear_flag(m_playingIndicator, LV_OBJ_FLAG_HIDDEN);
            // } else {
            //     lv_obj_add_flag(m_playingIndicator, LV_OBJ_FLAG_HIDDEN);
            // }
            // // Обновляем название текущей станции из информации
            // if (strlen(msg.data.playback.station_name) > 0) {
            //     // Можно обновить отображение, но лучше сохранить согласованность
            // }
            break;

        case EVENT_STATION_CHANGED:{
            int index=StationsManager::getInstance().findIndexByUrl(msg.data.url);
            m_currentStationIndex = index;
            updateStationDisplay();
            break;
        }
            
        default:
            break;
    }
}

// ==================== Обработка событий энкодера ====================

void StationScreen::onTurnRight() {
    // Следующая станция
    int newIndex = m_currentStationIndex + 1;
    if (newIndex >= STATIONS_COUNT) {
        newIndex = 0;  // циклический переход
    }
    setCurrentStation(newIndex);
}

void StationScreen::onTurnLeft() {
    // Предыдущая станция
    int newIndex = m_currentStationIndex - 1;
    if (newIndex < 0) {
        newIndex = STATIONS_COUNT - 1;  // циклический переход
    }
    setCurrentStation(newIndex);
}

void StationScreen::onShortPress() {

    // Немедленное сохранение в NVS
    NVSManager::getInstance().commit();  // немедленное сохранение
    
    // Возвращаемся на первый экран (VolumeScreen)
    if (m_manager) {
        m_manager->switchTo(0);
    }
}

void StationScreen::refresh() {
    ESP_LOGD(TAG, "Refreshing StationScreen");
    updateStationDisplay();
}

// ==================== Вспомогательные методы ====================

void StationScreen::setCurrentStation(int index) {
    if (index < 0) index = 0;
    if (index >= STATIONS_COUNT) index = STATIONS_COUNT - 1;
    
    m_currentStationIndex = index;

    // Сохраняем выбранную станцию в NVS
    NVSManager::getInstance().setStation(m_currentStationIndex);

    switchToStation(m_currentStationIndex);
    // updateStationDisplay();
}

void StationScreen::updateStationDisplay() {
    if (!m_stationNameLabel) return;
    
    // Получаем названия станций из stations
    StationsManager& stations=StationsManager::getInstance();

    const char* current = stations.getName(m_currentStationIndex);
    const char* prev = (m_currentStationIndex > 0) 
        ? stations.getName(m_currentStationIndex - 1) 
        : stations.getName(stations.getCount() - 1);
    const char* next = (m_currentStationIndex < stations.getCount() - 1) 
        ? stations.getName(m_currentStationIndex + 1) 
        : stations.getName(0);
    
    // Обновляем метки
    lv_label_set_text(m_stationNameLabel, current);
    
    char prevBuf[64];
    char nextBuf[64];
    snprintf(prevBuf, sizeof(prevBuf), "▲ %s", prev);
    snprintf(nextBuf, sizeof(nextBuf), "▼ %s", next);
    
    lv_label_set_text(m_previousLabel, prevBuf);
    lv_label_set_text(m_nextLabel, nextBuf);
    
    ESP_LOGD(TAG, "Station display updated: %s", current);
}

void StationScreen::switchToStation(int index) {
    StationsManager::getInstance().getCount(); 
    
    if (index < 0) index=0;
    if (index >= StationsManager::getInstance().getCount()) index=StationsManager::getInstance().getCount()-1;

    const char* url = StationsManager::getInstance().getUrl(index);
    ESP_LOGI(TAG, "Switching to station %d: %s", index, url);
    
    // Отправляем команду в AudioTask (через очередь)
    AudioMessage msg;
    msg.type = CMD_PLAY_URL;
    strncpy(msg.url, url, sizeof(msg.url) - 1);
    msg.url[sizeof(msg.url) - 1] = '\0';
    msg.value1 = 0;
    msg.value2 = 0;
    msg.value3 = 0;
    xQueueSend(audioQueue, &msg, portMAX_DELAY);
    // UI обновится через EVENT_STATION_CHANGED от AudioTask

    ESP_LOGI(TAG, "Command sent to AudioTask: CMD_PLAY_URL");

}

