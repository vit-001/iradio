#include "ui_task.h"
#include "config.h"
#include "stations.h"
#include "drivers/encoder/encoder.h"
// #include "drivers/audio/audio_manager.h"
#include "drivers/display/display_driver.h"
#include "drivers/nvs/nvs_manager.h"
#include "messages/audio_messages.h"
#include "esp_log.h"
#include "lvgl.h"

LV_FONT_DECLARE(my_font_16);

static const char* TAG = "UI_TASK";
static TaskHandle_t s_uiTaskHandle = NULL;
static Encoder* s_encoder = nullptr;

extern QueueHandle_t audioQueue;

// Режимы работы энкодера
enum EncoderMode {
    MODE_VOLUME = 0,
    MODE_STATION,
    MODE_BASS,
    MODE_MID,
    MODE_TREBLE,
    MODE_COUNT
};

static const char* mode_names[] = {
    "VOLUME",
    "STATION",
    "BASS",
    "MID",
    "TREBLE"
};

static EncoderMode current_mode = MODE_VOLUME;
static int audioVolume = 10;
static int bass_value = TONE_BASS;      // -40..+6
static int mid_value = TONE_MID;       // -40..+6
static int treble_value = TONE_TREBLE;    // -40..+6
static int current_station = 0;

// // Список радиостанций
// static const char* stations[] = {
//     "DFM",
//     "Europa Plus", 
//     "Hit FM"
// };
// static const char* station_urls[] = {
//     "http://dfm.hostingradio.ru/dfm128.mp3",
//     "http://ep128.streamr.ru",
//     "http://hitfm.hostingradio.ru/hitfm128.mp3"
// };

// #define STATIONS_COUNT (sizeof(stations) / sizeof(stations[0]))

// LVGL объекты
static lv_obj_t* station_label = NULL;
static lv_obj_t* song_label = NULL;
static lv_obj_t* volume_bar = NULL;
static lv_obj_t* volume_label = NULL;
static lv_obj_t* status_label = NULL;
static lv_obj_t* mode_label = NULL;
static lv_obj_t* bass_label = NULL;
static lv_obj_t* mid_label = NULL;
static lv_obj_t* treble_label = NULL;

// Предварительное объявление функции
static void updateModeDisplay();

// Функция отправки команды в AudioTask
static void sendAudioCommand(AudioCommandType type, int v1=0, int v2=0, int v3=0, const char* url=nullptr) {
    if (audioQueue == NULL) {
        ESP_LOGE(TAG, "Audio queue not initialized!");
        return;
    }
    
    AudioMessage msg;
    msg.type = type;
    msg.value1 = v1;
    msg.value2 = v2;
    msg.value3 = v3;
    if (url) {
        strncpy(msg.url, url, sizeof(msg.url) - 1);
        msg.url[sizeof(msg.url) - 1] = '\0';
    } else {
        msg.url[0] = '\0';
    }
    
    xQueueSend(audioQueue, &msg, portMAX_DELAY);
    ESP_LOGD(TAG, "Command sent: %d", type);
}

// Функция переключения станции
static void switchStation(int direction) {
    current_station = (current_station + direction + STATIONS_COUNT) % STATIONS_COUNT;
    sendAudioCommand(CMD_PLAY_URL, 0, 0, 0, station_urls[current_station]);
    NVSManager::getInstance().setStation(current_station);
    if (station_label) {
        lv_label_set_text(station_label, stations[current_station]);
    }
    ESP_LOGI(TAG, "Switched to station: %s", stations[current_station]);
}

// Функция возврата в режим громкости
static void returnToVolumeMode() {
    if (current_mode != MODE_VOLUME) {
        current_mode = MODE_VOLUME;
        updateModeDisplay();
        ESP_LOGI(TAG, "Returned to VOLUME mode");
    }
}

// Функция обновления отображения режима
static void updateModeDisplay() {
    if (mode_label) {
        lv_label_set_text(mode_label, mode_names[current_mode]);
        
        uint32_t color;
        switch (current_mode) {
            case MODE_VOLUME:  color = 0x00FF00; break;
            case MODE_STATION: color = 0xFFFF00; break;
            case MODE_BASS:    color = 0xFF6600; break;
            case MODE_MID:     color = 0x00FFFF; break;
            case MODE_TREBLE:  color = 0xFF00FF; break;
            default:           color = 0xFFFFFF; break;
        }
        lv_obj_set_style_text_color(mode_label, lv_color_hex(color), 0);
    }
    
    // Скрываем все элементы
    if (volume_bar) lv_obj_add_flag(volume_bar, LV_OBJ_FLAG_HIDDEN);
    if (volume_label) lv_obj_add_flag(volume_label, LV_OBJ_FLAG_HIDDEN);
    if (bass_label) lv_obj_add_flag(bass_label, LV_OBJ_FLAG_HIDDEN);
    if (mid_label) lv_obj_add_flag(mid_label, LV_OBJ_FLAG_HIDDEN);
    if (treble_label) lv_obj_add_flag(treble_label, LV_OBJ_FLAG_HIDDEN);
    
    // Показываем нужные
    switch (current_mode) {
        case MODE_VOLUME:
            if (volume_bar) lv_obj_clear_flag(volume_bar, LV_OBJ_FLAG_HIDDEN);
            if (volume_label) lv_obj_clear_flag(volume_label, LV_OBJ_FLAG_HIDDEN);
            break;
        case MODE_BASS:
            if (bass_label) lv_obj_clear_flag(bass_label, LV_OBJ_FLAG_HIDDEN);
            break;
        case MODE_MID:
            if (mid_label) lv_obj_clear_flag(mid_label, LV_OBJ_FLAG_HIDDEN);
            break;
        case MODE_TREBLE:
            if (treble_label) lv_obj_clear_flag(treble_label, LV_OBJ_FLAG_HIDDEN);
            break;
        default:
            break;
    }
}

// Callback для энкодера
static void onEncoderEvent(EncoderEvent event, void* userData) {
    switch (event) {
        // ========== ВРАЩЕНИЕ ВПРАВО ==========
        case ENCODER_TURN_RIGHT:
            switch (current_mode) {
                case MODE_VOLUME:
                    audioVolume++;
                    if (audioVolume > 21) audioVolume = 21;
                    // Отправляем команду в AudioTask
                    sendAudioCommand(CMD_SET_VOLUME, audioVolume);
                    // Сохраняем в NVS (отложенно)
                    NVSManager::getInstance().setVolume(audioVolume);
                    // Обновляем UI
                    lv_bar_set_value(volume_bar, audioVolume, LV_ANIM_ON);
                    char vol_buf[32];
                    snprintf(vol_buf, sizeof(vol_buf), "VOLUME: %d/21", audioVolume);
                    lv_label_set_text(volume_label, vol_buf);
                    ESP_LOGI(TAG, "Volume: %d", audioVolume);
                    break;
                    
                case MODE_STATION:
                    current_station++;
                    if (current_station >= STATIONS_COUNT) current_station = 0;
                    // Отправляем команду в AudioTask
                    sendAudioCommand(CMD_PLAY_URL, 0, 0, 0, station_urls[current_station]);
                    // Сохраняем в NVS (отложенно)
                    NVSManager::getInstance().setStation(current_station);
                    // Обновляем UI
                    lv_label_set_text(station_label, stations[current_station]);
                    ESP_LOGI(TAG, "Station: %s", stations[current_station]);
                    break;
                    
                case MODE_BASS:
                    bass_value++;
                    if (bass_value > 6) bass_value = 6;
                    // Отправляем команду в AudioTask
                    sendAudioCommand(CMD_SET_TONE, bass_value, mid_value, treble_value);
                    // Сохраняем в NVS (отложенно)
                    NVSManager::getInstance().setBass(bass_value);
                    // Обновляем UI
                    if (bass_label) {
                        char buf[32];
                        snprintf(buf, sizeof(buf), "BASS: %+d dB", bass_value);
                        lv_label_set_text(bass_label, buf);
                    }
                    ESP_LOGI(TAG, "Bass: %d", bass_value);
                    break;
                    
                case MODE_MID:
                    mid_value++;
                    if (mid_value > 6) mid_value = 6;
                    // Отправляем команду в AudioTask
                    sendAudioCommand(CMD_SET_TONE, bass_value, mid_value, treble_value);
                    // Сохраняем в NVS (отложенно)
                    NVSManager::getInstance().setMid(mid_value);
                    // Обновляем UI
                    if (mid_label) {
                        char buf[32];
                        snprintf(buf, sizeof(buf), "MID: %+d dB", mid_value);
                        lv_label_set_text(mid_label, buf);
                    }
                    ESP_LOGI(TAG, "Mid: %d", mid_value);
                    break;
                    
                case MODE_TREBLE:
                    treble_value++;
                    if (treble_value > 6) treble_value = 6;
                    // Отправляем команду в AudioTask
                    sendAudioCommand(CMD_SET_TONE, bass_value, mid_value, treble_value);
                    // Сохраняем в NVS (отложенно)
                    NVSManager::getInstance().setTreble(treble_value);
                    // Обновляем UI
                    if (treble_label) {
                        char buf[32];
                        snprintf(buf, sizeof(buf), "TREBLE: %+d dB", treble_value);
                        lv_label_set_text(treble_label, buf);
                    }
                    ESP_LOGI(TAG, "Treble: %d", treble_value);
                    break;
                    
                default:
                    break;
            }
            break;
            
        // ========== ВРАЩЕНИЕ ВЛЕВО ==========
        case ENCODER_TURN_LEFT:
            switch (current_mode) {
                case MODE_VOLUME:
                    audioVolume--;
                    if (audioVolume < 0) audioVolume = 0;
                    // Отправляем команду в AudioTask
                    sendAudioCommand(CMD_SET_VOLUME, audioVolume);
                    // Сохраняем в NVS (отложенно)
                    NVSManager::getInstance().setVolume(audioVolume);
                    // Обновляем UI
                    lv_bar_set_value(volume_bar, audioVolume, LV_ANIM_ON);
                    char vol_buf[32];
                    snprintf(vol_buf, sizeof(vol_buf), "VOLUME: %d/21", audioVolume);
                    lv_label_set_text(volume_label, vol_buf);
                    ESP_LOGI(TAG, "Volume: %d", audioVolume);
                    break;
                    
                case MODE_STATION:
                    current_station--;
                    if (current_station < 0) current_station = STATIONS_COUNT - 1;
                    // Отправляем команду в AudioTask
                    sendAudioCommand(CMD_PLAY_URL, 0, 0, 0, station_urls[current_station]);
                    // Сохраняем в NVS (отложенно)
                    NVSManager::getInstance().setStation(current_station);
                    // Обновляем UI
                    lv_label_set_text(station_label, stations[current_station]);
                    ESP_LOGI(TAG, "Station: %s", stations[current_station]);
                    break;
                    
                case MODE_BASS:
                    bass_value--;
                    if (bass_value < -40) bass_value = -40;
                    // Отправляем команду в AudioTask
                    sendAudioCommand(CMD_SET_TONE, bass_value, mid_value, treble_value);
                    // Сохраняем в NVS (отложенно)
                    NVSManager::getInstance().setBass(bass_value);
                    // Обновляем UI
                    if (bass_label) {
                        char buf[32];
                        snprintf(buf, sizeof(buf), "BASS: %+d dB", bass_value);
                        lv_label_set_text(bass_label, buf);
                    }
                    ESP_LOGI(TAG, "Bass: %d", bass_value);
                    break;
                    
                case MODE_MID:
                    mid_value--;
                    if (mid_value < -40) mid_value = -40;
                    // Отправляем команду в AudioTask
                    sendAudioCommand(CMD_SET_TONE, bass_value, mid_value, treble_value);
                    // Сохраняем в NVS (отложенно)
                    NVSManager::getInstance().setMid(mid_value);
                    // Обновляем UI
                    if (mid_label) {
                        char buf[32];
                        snprintf(buf, sizeof(buf), "MID: %+d dB", mid_value);
                        lv_label_set_text(mid_label, buf);
                    }
                    ESP_LOGI(TAG, "Mid: %d", mid_value);
                    break;
                    
                case MODE_TREBLE:
                    treble_value--;
                    if (treble_value < -40) treble_value = -40;
                    // Отправляем команду в AudioTask
                    sendAudioCommand(CMD_SET_TONE, bass_value, mid_value, treble_value);
                    // Сохраняем в NVS (отложенно)
                    NVSManager::getInstance().setTreble(treble_value);
                    // Обновляем UI
                    if (treble_label) {
                        char buf[32];
                        snprintf(buf, sizeof(buf), "TREBLE: %+d dB", treble_value);
                        lv_label_set_text(treble_label, buf);
                    }
                    ESP_LOGI(TAG, "Treble: %d", treble_value);
                    break;
                    
                default:
                    break;
            }
            break;
            
        // ========== КОРОТКОЕ НАЖАТИЕ ==========
        case ENCODER_BUTTON_SHORT:
            switch (current_mode) {
                case MODE_VOLUME:
                    // Play/Pause
                    sendAudioCommand(CMD_PLAY_PAUSE);
                    // Обновляем UI статуса
                    if (status_label) {
                        bool isPlaying = (strcmp(lv_label_get_text(status_label), "⏸ PAUSED") == 0);
                        lv_label_set_text(status_label, isPlaying ? "▶ PLAYING" : "⏸ PAUSED");
                        lv_obj_set_style_text_color(status_label, 
                            isPlaying ? lv_color_hex(0x00FF00) : lv_color_hex(0xFF0000), 0);
                    }
                    ESP_LOGI(TAG, "Play/Pause toggled");
                    break;
                    
                case MODE_STATION:
                    // Выбор станции — сохраняем и возвращаемся в режим громкости
                    NVSManager::getInstance().commit();  // немедленно сохраняем
                    current_mode = MODE_VOLUME;
                    updateModeDisplay();
                    ESP_LOGI(TAG, "Station selected, returning to VOLUME mode");
                    break;
                    
                case MODE_BASS:
                    // Сброс басов в 0
                    bass_value = 0;
                    sendAudioCommand(CMD_SET_TONE, bass_value, mid_value, treble_value);
                    NVSManager::getInstance().setBass(bass_value);
                    if (bass_label) {
                        char buf[32];
                        snprintf(buf, sizeof(buf), "BASS: %+d dB", bass_value);
                        lv_label_set_text(bass_label, buf);
                    }
                    ESP_LOGI(TAG, "Bass reset to 0");
                    break;
                    
                case MODE_MID:
                    // Сброс средних в 0
                    mid_value = 0;
                    sendAudioCommand(CMD_SET_TONE, bass_value, mid_value, treble_value);
                    NVSManager::getInstance().setMid(mid_value);
                    if (mid_label) {
                        char buf[32];
                        snprintf(buf, sizeof(buf), "MID: %+d dB", mid_value);
                        lv_label_set_text(mid_label, buf);
                    }
                    ESP_LOGI(TAG, "Mid reset to 0");
                    break;
                    
                case MODE_TREBLE:
                    // Сброс высоких в 0
                    treble_value = 0;
                    sendAudioCommand(CMD_SET_TONE, bass_value, mid_value, treble_value);
                    NVSManager::getInstance().setTreble(treble_value);
                    if (treble_label) {
                        char buf[32];
                        snprintf(buf, sizeof(buf), "TREBLE: %+d dB", treble_value);
                        lv_label_set_text(treble_label, buf);
                    }
                    ESP_LOGI(TAG, "Treble reset to 0");
                    break;
                    
                default:
                    break;
            }
            break;
            
        // ========== ДВОЙНОЕ НАЖАТИЕ (возврат в VOLUME) ==========
        case ENCODER_BUTTON_DOUBLE:
            // Сохраняем все настройки
            NVSManager::getInstance().commit();
            // Возвращаемся в режим громкости
            if (current_mode != MODE_VOLUME) {
                current_mode = MODE_VOLUME;
                updateModeDisplay();
                ESP_LOGI(TAG, "Double click: returned to VOLUME mode");
            }
            break;
            
        // ========== ДОЛГОЕ НАЖАТИЕ (переключение режимов) ==========
        case ENCODER_BUTTON_LONG:
            // Сохраняем все настройки перед сменой режима
            NVSManager::getInstance().commit();
            // Переключаем режим
            current_mode = (EncoderMode)((current_mode + 1) % MODE_COUNT);
            updateModeDisplay();
            ESP_LOGI(TAG, "Mode switched to: %s", mode_names[current_mode]);
            break;
            
        default:
            break;
    }
}

// Создание UI элементов
static void create_ui(void) {
    // Загружаем сохранённые настройки
    int savedVolume = NVSManager::getInstance().loadVolume(DEFAULT_VOLUME);
    bass_value = NVSManager::getInstance().loadBass(TONE_BASS);
    mid_value = NVSManager::getInstance().loadMid(TONE_MID);
    treble_value = NVSManager::getInstance().loadTreble(TONE_TREBLE);
    current_station = NVSManager::getInstance().loadStation(0);
    
    // ===== Отправляем команды в AudioTask =====
    sendAudioCommand(CMD_SET_VOLUME, savedVolume);
    sendAudioCommand(CMD_SET_TONE, bass_value, mid_value, treble_value);
    sendAudioCommand(CMD_PLAY_URL, 0, 0, 0, station_urls[current_station]);


    lv_obj_t* scr = display_get_scr_act();
    lv_obj_set_style_bg_color(scr, lv_color_hex(0x000000), 0);
    
    // Режим (отображается в правом верхнем углу)
    mode_label = lv_label_create(scr);
    lv_label_set_text(mode_label, mode_names[current_mode]);
    lv_obj_set_style_text_font(mode_label, &my_font_16, 0);
    lv_obj_set_style_text_color(mode_label, lv_color_hex(0x00FFFF), 0);
    lv_obj_align(mode_label, LV_ALIGN_TOP_RIGHT, -10, 10);
    
    // Название станции
    station_label = lv_label_create(scr);
    lv_label_set_text(station_label, stations[current_station]);
    lv_obj_set_style_text_font(station_label, &lv_font_montserrat_20, 0);
    lv_obj_set_style_text_color(station_label, lv_color_hex(0xFFFF00), 0);
    lv_obj_align(station_label, LV_ALIGN_TOP_MID, 0, 10);
    
    // Название трека
    song_label = lv_label_create(scr);
    lv_label_set_text(song_label, "Loading...");
    lv_obj_set_style_text_font(song_label, &my_font_16, 0);
    lv_obj_set_style_text_color(song_label, lv_color_hex(0xFFFFFF), 0);
    lv_obj_align(song_label, LV_ALIGN_CENTER, 0, -20);
    
    // Прогресс-бар громкости
    volume_bar = lv_bar_create(scr);
    lv_obj_set_size(volume_bar, 200, 15);
    lv_bar_set_range(volume_bar, 0, 21);
    lv_bar_set_value(volume_bar, audioVolume, LV_ANIM_OFF);
    lv_obj_align(volume_bar, LV_ALIGN_BOTTOM_MID, 0, -55);
    
    // Текст громкости
    volume_label = lv_label_create(scr);
    char vol_buf[32];
    snprintf(vol_buf, sizeof(vol_buf), "VOLUME: %d/21", audioVolume);
    lv_label_set_text(volume_label, vol_buf);
    lv_obj_set_style_text_font(volume_label, &my_font_16, 0);
    lv_obj_set_style_text_color(volume_label, lv_color_hex(0xFFFFFF), 0);
    lv_obj_align_to(volume_label, volume_bar, LV_ALIGN_OUT_TOP_MID, 0, -5);
    
    // Статус воспроизведения
    status_label = lv_label_create(scr);
    lv_label_set_text(status_label, "▶ PLAYING");
    lv_obj_set_style_text_font(status_label, &my_font_16, 0);
    lv_obj_set_style_text_color(status_label, lv_color_hex(0x00FF00), 0);
    lv_obj_align(status_label, LV_ALIGN_BOTTOM_MID, 0, -20);
    
    // Басы (скрыты по умолчанию)
    bass_label = lv_label_create(scr);
    lv_label_set_text(bass_label, "BASS: 0 dB");
    lv_obj_set_style_text_font(bass_label, &my_font_16, 0);
    lv_obj_set_style_text_color(bass_label, lv_color_hex(0xFFFFFF), 0);
    lv_obj_align(bass_label, LV_ALIGN_CENTER, 0, 0);
    lv_obj_add_flag(bass_label, LV_OBJ_FLAG_HIDDEN);
 
    // Средние (скрыты по умолчанию)
    mid_label = lv_label_create(scr);
    lv_label_set_text(mid_label, "MID: 0 dB");
    lv_obj_set_style_text_font(mid_label, &my_font_16, 0);
    lv_obj_set_style_text_color(mid_label, lv_color_hex(0xFFFFFF), 0);
    lv_obj_align(mid_label, LV_ALIGN_CENTER, 0, 0);
    lv_obj_add_flag(mid_label, LV_OBJ_FLAG_HIDDEN);

    // Высокие (скрыты по умолчанию)
    treble_label = lv_label_create(scr);
    lv_label_set_text(treble_label, "TREBLE: 0 dB");
    lv_obj_set_style_text_font(treble_label, &my_font_16, 0);
    lv_obj_set_style_text_color(treble_label, lv_color_hex(0xFFFFFF), 0);
    lv_obj_align(treble_label, LV_ALIGN_CENTER, 0, 0);
    lv_obj_add_flag(treble_label, LV_OBJ_FLAG_HIDDEN);
}

void uiTaskFunction(void* parameter) {
    ESP_LOGI(TAG, "Started on core %d", xPortGetCoreID());
    
    // Инициализация дисплея
    if (!display_driver_init()) {
        ESP_LOGE(TAG, "Failed to init display!");
        while (1) vTaskDelay(1000);
    }
    
    // Создание UI
    create_ui();
    
    // Инициализация энкодера
    s_encoder = new Encoder(ENC_A, ENC_B, ENC_BTN);
    s_encoder->begin();
    s_encoder->setCallback(onEncoderEvent);
    s_encoder->setLongPressTime(800);      // 800 мс для долгого нажатия
    s_encoder->setDoubleClickTime(300);    // 300 мс между нажатиями для двойного
    
    ESP_LOGI(TAG, "UI task ready");
    
    while (true) {
        s_encoder->update();
        display_update();
        NVSManager::getInstance().processDelayedSave(); 
        vTaskDelay(pdMS_TO_TICKS(5));
    }
}

void startUiTask(int core, int priority, int stackSize) {
    xTaskCreatePinnedToCore(uiTaskFunction, "UITask", stackSize, NULL, priority, &s_uiTaskHandle, core);
}

TaskHandle_t getUiTaskHandle() {
    return s_uiTaskHandle;
}

// Функция обновления названия трека (вызывается из audio_task)
void ui_update_song(const char* song) {
    if (song_label) {
        lv_label_set_text(song_label, song);
        // Восстанавливаем позицию текста трека
        lv_obj_align(song_label, LV_ALIGN_CENTER, 0, -20);
    }
}