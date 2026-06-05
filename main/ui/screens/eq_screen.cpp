/**
 * @file eq_screen.cpp
 * @brief Реализация экрана эквалайзера
 */

#include "eq_screen.h"
#include "ui/screen_manager.h"
// #include "drivers/audio/audio_manager.h"
#include "messages/audio_messages.h"
#include "drivers/nvs/nvs_manager.h"
#include "messages/audio_to_ui_messages.h"
#include "fonts/fonts.h"
#include "esp_log.h"

static const char* TAG = "EQ_SCREEN";

// ==================== Статические данные ====================

const char* EQScreen::BAND_NAMES[] = {"BASS", "MID", "TREBLE"};

// ==================== Конструктор ====================

EQScreen::EQScreen(ScreenManager* manager, lv_obj_t* parent) 
    : ScreenWithHandlers(manager, parent) {
}

// ==================== Жизненный цикл ====================

lv_obj_t* EQScreen::create() {
    ESP_LOGI(TAG, "Creating EQScreen");

    // Создаем базовый контейнер для контента
    main_cont = lv_obj_create(m_parent);
    lv_obj_set_size(main_cont, LV_PCT(100), LV_PCT(100));    
    lv_obj_set_style_bg_color(main_cont, lv_color_hex(0x000000), 0);
    lv_obj_set_style_bg_opa(main_cont, LV_OPA_TRANSP, 0); // Прозрачный фон 


    // Настраиваем Flex-сетку (в колонку, сверху вниз)
    lv_obj_set_layout(main_cont, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(main_cont, LV_FLEX_FLOW_COLUMN);
 
    // Выравнивание элементов по центру по горизонтали, старт — по вертикали
    lv_obj_set_flex_align(main_cont, 
                            LV_FLEX_ALIGN_SPACE_AROUND,   // Выравнивание по вертикали (main axis)
                            LV_FLEX_ALIGN_CENTER,  // Выравнивание по горизонтали (cross axis)
                            LV_FLEX_ALIGN_CENTER);  // Выравнивание строк (для multi-line)
    

    // 3. Сбрасываем дефолтные отступы, настраиваем зазор между метками
    lv_obj_set_style_pad_all(main_cont, 10, 0);     // Внутренний отступ от краев экрана
    lv_obj_set_style_pad_row(main_cont, 8, 0);      // Расстояние МЕЖДУ метками по вертикали
    lv_obj_set_style_border_width(main_cont, 0, 0); // Без рамок
      
    // ==================== Индикатор режима ====================
    m_modeLabel = lv_label_create(main_cont);
    lv_label_set_text(m_modeLabel, "EQ");
    lv_obj_set_style_text_color(m_modeLabel, lv_color_hex(0x00FFFF), 0);  // голубой
    lv_obj_set_style_pad_bottom(m_modeLabel, 20, 0);
    
    // ==================== Название текущей полосы ====================
    m_bandLabel = lv_label_create(main_cont);
    lv_obj_set_style_text_font(m_bandLabel, font_accent, 0);
    lv_obj_set_style_text_color(m_bandLabel, lv_color_hex(0xFFFF00), 0);  // жёлтый
    lv_obj_set_style_pad_bottom(m_bandLabel, 10, 0);
    
    // ==================== Текущее значение ====================
    m_valueLabel = lv_label_create(main_cont);
    lv_obj_set_style_text_font(m_valueLabel, font_accent, 0);
    lv_obj_set_style_pad_bottom(m_valueLabel, 20, 0);
    
    // ==================== Шкала с метками ====================
    // Создаём горизонтальный контейнер для меток
    lv_obj_t* scale_cont = lv_obj_create(main_cont);
    lv_obj_set_size(scale_cont, 200, 30);
    lv_obj_set_style_bg_color(scale_cont, lv_color_hex(0x000000), 0);
    lv_obj_set_style_border_width(scale_cont, 0, 0);
    lv_obj_set_style_pad_all(scale_cont, 0, 0);
    lv_obj_clear_flag(scale_cont, LV_OBJ_FLAG_SCROLLABLE);
    
    lv_obj_set_flex_flow(scale_cont, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(scale_cont, 
        LV_FLEX_ALIGN_SPACE_BETWEEN,
        LV_FLEX_ALIGN_CENTER,
        LV_FLEX_ALIGN_CENTER);
    
    m_minLabel = lv_label_create(scale_cont);
    lv_label_set_text(m_minLabel, "-40");
    lv_obj_set_style_text_font(m_minLabel, font_accent, 0);
    lv_obj_set_style_text_color(m_minLabel, lv_color_hex(0x888888), 0);
    
    m_zeroLabel = lv_label_create(scale_cont);
    lv_label_set_text(m_zeroLabel, "0");
    lv_obj_set_style_text_font(m_zeroLabel, font_accent, 0);
    lv_obj_set_style_text_color(m_zeroLabel, lv_color_hex(0xFFFFFF), 0);
    
    m_maxLabel = lv_label_create(scale_cont);
    lv_label_set_text(m_maxLabel, "+6");
    lv_obj_set_style_text_font(m_maxLabel, font_accent, 0);
    lv_obj_set_style_text_color(m_maxLabel, lv_color_hex(0x888888), 0);
    
    // ==================== Прогресс-бар ====================
    m_bar = lv_bar_create(main_cont);
    lv_obj_set_size(m_bar, 200, 20);
    lv_bar_set_range(m_bar, 0, 100);
    lv_obj_set_style_pad_bottom(m_bar, 20, 0);
    
    // ==================== Загрузка значений ====================
    // Загружаем сохранённые значения из NVS
    m_bass = NVSManager::getInstance().loadBass(0);
    m_mid = NVSManager::getInstance().loadMid(0);
    m_treble = NVSManager::getInstance().loadTreble(0);
    
    // Отправляем в AudioManager для применения
    // AudioManager::getInstance().setTone(m_bass, m_mid, m_treble);
    
    // Отправляем команду в AudioTask (через очередь)
    AudioMessage msg;
    msg.type = CMD_SET_TONE;
    msg.value1 = m_bass;
    msg.value2 = m_mid;
    msg.value3 = m_treble;
    xQueueSend(audioQueue, &msg, portMAX_DELAY);
    ESP_LOGI(TAG, "Command sent to AudioTask: CMD_SET_TONE");

    // Обновляем отображение
    updateDisplay();
    
    ESP_LOGI(TAG, "EQScreen created with values: B=%d, M=%d, T=%d", 
             m_bass, m_mid, m_treble);
    
    return main_cont;
}

// ==================== Обработка событий от AudioTask ====================

void EQScreen::handleAudioEvent(const AudioToUIMessage& msg) {
    switch (msg.type) {
        case EVENT_EQ_VALUES:
            // Обновляем значения
            m_bass = msg.data.eq.bass;
            m_mid = msg.data.eq.mid;
            m_treble = msg.data.eq.treble;
            updateDisplay();
            ESP_LOGD(TAG, "EQ values updated from audio: B=%d, M=%d, T=%d", 
                     m_bass, m_mid, m_treble);
            break;
            
        default:
            break;
    }
}

// ==================== Обработка событий энкодера ====================

void EQScreen::onTurnRight() {
    int newValue = getCurrentBandValue() + 1;
    if (newValue > MAX_VALUE) newValue = MAX_VALUE;
    setCurrentBandValue(newValue);
}

void EQScreen::onTurnLeft() {
    int newValue = getCurrentBandValue() - 1;
    if (newValue < MIN_VALUE) newValue = MIN_VALUE;
    setCurrentBandValue(newValue);
}

void EQScreen::onShortPress() {
    // Переключение на следующую полосу
    m_currentBand = (Band)((m_currentBand + 1) % BAND_COUNT);
    updateDisplay();
    ESP_LOGD(TAG, "Switched to band: %s", BAND_NAMES[m_currentBand]);
}

// ==================== Вспомогательные методы ====================

int EQScreen::getCurrentBandValue() const {
    switch (m_currentBand) {
        case BASS:   return m_bass;
        case MID:    return m_mid;
        case TREBLE: return m_treble;
        default:     return 0;
    }
}

void EQScreen::setCurrentBandValue(int value) {
    switch (m_currentBand) {
        case BASS:
            m_bass = value;
            break;
        case MID:
            m_mid = value;
            break;
        case TREBLE:
            m_treble = value;
            break;
        default:
            break;
    }
    
    // Обновляем отображение
    updateDisplay();
    
    // Отправляем в AudioManager
    sendEQToAudio();
    
    // Сохраняем в NVS
    saveToNVS();
}

void EQScreen::updateDisplay() {
    if (!m_bandLabel || !m_valueLabel || !m_bar) return;
    
    // Название текущей полосы
    lv_label_set_text(m_bandLabel, BAND_NAMES[m_currentBand]);
    
    // Текущее значение
    int value = getCurrentBandValue();
    char buf[32];
    snprintf(buf, sizeof(buf), "%+d dB", value);
    lv_label_set_text(m_valueLabel, buf);
    
    // Цвет значения
    lv_obj_set_style_text_color(m_valueLabel, lv_color_hex(getValueColor(value)), 0);
    
    // Прогресс-бар
    int percent = valueToPercent(value);
    lv_bar_set_value(m_bar, percent, LV_ANIM_ON);
}

void EQScreen::refresh() {
    ESP_LOGD(TAG, "Refreshing EQScreen");
    updateDisplay();  
}

void EQScreen::sendEQToAudio() {
    // AudioManager::getInstance().setTone(m_bass, m_mid, m_treble);
    // Отправляем команду в AudioTask (через очередь)
    AudioMessage msg;
    msg.type = CMD_SET_TONE;
    msg.value1 = m_bass;
    msg.value2 = m_mid;
    msg.value3 = m_treble;
    xQueueSend(audioQueue, &msg, portMAX_DELAY);

    ESP_LOGI(TAG, "Sent EQ to audio: B=%d, M=%d, T=%d", m_bass, m_mid, m_treble);
}

void EQScreen::saveToNVS() {
    NVSManager::getInstance().setBass(m_bass);
    NVSManager::getInstance().setMid(m_mid);
    NVSManager::getInstance().setTreble(m_treble);
}

int EQScreen::valueToPercent(int value) const {
    // Диапазон: -40 .. +6, всего 46 единиц
    int offset = value - MIN_VALUE;  // 0..46
    return (offset * 100) / (MAX_VALUE - MIN_VALUE);
}

uint32_t EQScreen::getValueColor(int value) const {
    if (value > 0) {
        return 0x00FF00;      // зелёный — усиление
    } else if (value < 0) {
        return 0xFF6600;      // оранжевый — ослабление
    } else {
        return 0xFFFFFF;      // белый — нейтрально
    }
}