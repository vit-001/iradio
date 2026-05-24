#include "ui_task.h"
#include "config.h"
#include "drivers/encoder/encoder.h"
#include "drivers/audio/audio_manager.h"
#include "drivers/display/display_driver.h"
#include "esp_log.h"
#include "lvgl.h"

static const char* TAG = "UI_TASK";
static TaskHandle_t s_uiTaskHandle = NULL;
static Encoder* s_encoder = nullptr;

// Режимы работы энкодера
enum EncoderMode {
    MODE_VOLUME = 0,
    MODE_STATION,
    MODE_BASS,
    MODE_TREBLE,
    MODE_COUNT
};

static const char* mode_names[] = {
    "VOLUME",
    "STATION",
    "BASS",
    "TREBLE"
};

static EncoderMode current_mode = MODE_VOLUME;
static int bass_value = 0;      // -40..+6
static int treble_value = 0;    // -40..+6
static int current_station = 0;

// Список радиостанций
static const char* stations[] = {
    "DFM",
    "Europa Plus", 
    "Hit FM"
};
static const char* station_urls[] = {
    "http://dfm.hostingradio.ru/dfm128.mp3",
    "http://ep128.streamr.ru",
    "http://hitfm.hostingradio.ru/hitfm128.mp3"
};

#define STATIONS_COUNT (sizeof(stations) / sizeof(stations[0]))

// LVGL объекты
static lv_obj_t* station_label = NULL;
static lv_obj_t* song_label = NULL;
static lv_obj_t* volume_bar = NULL;
static lv_obj_t* volume_label = NULL;
static lv_obj_t* status_label = NULL;
static lv_obj_t* mode_label = NULL;
static lv_obj_t* bass_label = NULL;
static lv_obj_t* treble_label = NULL;

// Предварительное объявление функции
static void updateModeDisplay();

// Функция переключения станции
static void switchStation(int direction) {
    current_station = (current_station + direction + STATIONS_COUNT) % STATIONS_COUNT;
    AudioManager& audio = AudioManager::getInstance();
    audio.pause();  // вместо stopSong
    audio.connectToStream(station_urls[current_station]);
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

// Функция обновления дисплея при смене режима
static void updateModeDisplay() {
    if (mode_label) {
        lv_label_set_text(mode_label, mode_names[current_mode]);
    }
    
    AudioManager& audio = AudioManager::getInstance();
    
    // Сначала скрываем все
    if (volume_bar) lv_obj_add_flag(volume_bar, LV_OBJ_FLAG_HIDDEN);
    if (volume_label) lv_obj_add_flag(volume_label, LV_OBJ_FLAG_HIDDEN);
    if (bass_label) lv_obj_add_flag(bass_label, LV_OBJ_FLAG_HIDDEN);
    if (treble_label) lv_obj_add_flag(treble_label, LV_OBJ_FLAG_HIDDEN);
    
    // Показываем нужные элементы в зависимости от режима
    switch (current_mode) {
        case MODE_VOLUME:
            if (volume_bar) lv_obj_clear_flag(volume_bar, LV_OBJ_FLAG_HIDDEN);
            if (volume_label) lv_obj_clear_flag(volume_label, LV_OBJ_FLAG_HIDDEN);
            break;
        case MODE_STATION:
            // В режиме станций показываем подсказку
            if (song_label) {
                lv_label_set_text(song_label, "Turn to change station\nPress to select");
                lv_obj_align(song_label, LV_ALIGN_CENTER, 0, -20);
            }
            break;
        case MODE_BASS:
            if (bass_label) {
                lv_obj_clear_flag(bass_label, LV_OBJ_FLAG_HIDDEN);
                char buf[32];
                snprintf(buf, sizeof(buf), "BASS: %+d dB", bass_value);
                lv_label_set_text(bass_label, buf);
                lv_obj_align(bass_label, LV_ALIGN_CENTER, 0, 0);
            }
            break;
        case MODE_TREBLE:
            if (treble_label) {
                lv_obj_clear_flag(treble_label, LV_OBJ_FLAG_HIDDEN);
                char buf[32];
                snprintf(buf, sizeof(buf), "TREBLE: %+d dB", treble_value);
                lv_label_set_text(treble_label, buf);
                lv_obj_align(treble_label, LV_ALIGN_CENTER, 0, 0);
            }
            break;
        default:
            break;
    }
}

// Callback для энкодера
static void onEncoderEvent(EncoderEvent event, void* userData) {
    AudioManager& audio = AudioManager::getInstance();
    
    switch (event) {
        case ENCODER_TURN_RIGHT:
            switch (current_mode) {
                case MODE_VOLUME:
                    audio.volumeUp();
                    if (volume_bar) lv_bar_set_value(volume_bar, audio.getVolume(), LV_ANIM_ON);
                    if (volume_label) {
                        char buf[32];
                        snprintf(buf, sizeof(buf), "VOLUME: %d/21", audio.getVolume());
                        lv_label_set_text(volume_label, buf);
                    }
                    break;
                case MODE_STATION:
                    switchStation(1);
                    break;
                case MODE_BASS:
                    bass_value++;
                    if (bass_value > 6) bass_value = 6;
                    audio.setTone(bass_value, 0, treble_value);
                    if (bass_label) {
                        char buf[32];
                        snprintf(buf, sizeof(buf), "BASS: %+d dB", bass_value);
                        lv_label_set_text(bass_label, buf);
                    }
                    break;
                case MODE_TREBLE:
                    treble_value++;
                    if (treble_value > 6) treble_value = 6;
                    audio.setTone(bass_value, 0, treble_value);
                    if (treble_label) {
                        char buf[32];
                        snprintf(buf, sizeof(buf), "TREBLE: %+d dB", treble_value);
                        lv_label_set_text(treble_label, buf);
                    }
                    break;
                default:
                    break;
            }
            break;
            
        case ENCODER_TURN_LEFT:
            switch (current_mode) {
                case MODE_VOLUME:
                    audio.volumeDown();
                    if (volume_bar) lv_bar_set_value(volume_bar, audio.getVolume(), LV_ANIM_ON);
                    if (volume_label) {
                        char buf[32];
                        snprintf(buf, sizeof(buf), "VOLUME: %d/21", audio.getVolume());
                        lv_label_set_text(volume_label, buf);
                    }
                    break;
                case MODE_STATION:
                    switchStation(-1);
                    break;
                case MODE_BASS:
                    bass_value--;
                    if (bass_value < -40) bass_value = -40;
                    audio.setTone(bass_value, 0, treble_value);
                    if (bass_label) {
                        char buf[32];
                        snprintf(buf, sizeof(buf), "BASS: %+d dB", bass_value);
                        lv_label_set_text(bass_label, buf);
                    }
                    break;
                case MODE_TREBLE:
                    treble_value--;
                    if (treble_value < -40) treble_value = -40;
                    audio.setTone(bass_value, 0, treble_value);
                    if (treble_label) {
                        char buf[32];
                        snprintf(buf, sizeof(buf), "TREBLE: %+d dB", treble_value);
                        lv_label_set_text(treble_label, buf);
                    }
                    break;
                default:
                    break;
            }
            break;
            
        case ENCODER_BUTTON_SHORT:
            if (current_mode == MODE_VOLUME) {
                audio.playPause();
                if (status_label) {
                    lv_label_set_text(status_label, audio.isPlaying() ? "▶ PLAYING" : "⏸ PAUSED");
                    lv_obj_set_style_text_color(status_label, 
                        audio.isPlaying() ? lv_color_hex(0x00FF00) : lv_color_hex(0xFF0000), 0);
                }
            } else if (current_mode == MODE_STATION) {
                // Выбор текущей станции (уже играет)
                ESP_LOGI(TAG, "Station selected: %s", stations[current_station]);
                // Возвращаемся в режим громкости
                returnToVolumeMode();
            } else if (current_mode == MODE_BASS) {
                // Сброс басов в 0
                bass_value = 0;
                audio.setTone(bass_value, 0, treble_value);
                if (bass_label) {
                    char buf[32];
                    snprintf(buf, sizeof(buf), "BASS: %+d dB", bass_value);
                    lv_label_set_text(bass_label, buf);
                }
            } else if (current_mode == MODE_TREBLE) {
                // Сброс высоких в 0
                treble_value = 0;
                audio.setTone(bass_value, 0, treble_value);
                if (treble_label) {
                    char buf[32];
                    snprintf(buf, sizeof(buf), "TREBLE: %+d dB", treble_value);
                    lv_label_set_text(treble_label, buf);
                }
            }
            break;
            
        case ENCODER_BUTTON_DOUBLE:
            // Двойное нажатие — возврат в режим громкости из любого режима
            returnToVolumeMode();
            break;
            
        case ENCODER_BUTTON_LONG:
            // Переключение режима (долгое нажатие)
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
    lv_obj_t* scr = display_get_scr_act();
    lv_obj_set_style_bg_color(scr, lv_color_hex(0x000000), 0);
    
    // Режим (отображается в правом верхнем углу)
    mode_label = lv_label_create(scr);
    lv_label_set_text(mode_label, mode_names[current_mode]);
    lv_obj_set_style_text_font(mode_label, &lv_font_montserrat_14, 0);
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
    lv_obj_set_style_text_font(song_label, &lv_font_montserrat_14, 0);
    lv_obj_set_style_text_color(song_label, lv_color_hex(0xFFFFFF), 0);
    lv_obj_align(song_label, LV_ALIGN_CENTER, 0, -20);
    
    // Прогресс-бар громкости
    volume_bar = lv_bar_create(scr);
    lv_obj_set_size(volume_bar, 200, 15);
    lv_bar_set_range(volume_bar, 0, 21);
    AudioManager& audio = AudioManager::getInstance();
    lv_bar_set_value(volume_bar, audio.getVolume(), LV_ANIM_OFF);
    lv_obj_align(volume_bar, LV_ALIGN_BOTTOM_MID, 0, -55);
    
    // Текст громкости
    volume_label = lv_label_create(scr);
    char vol_buf[32];
    snprintf(vol_buf, sizeof(vol_buf), "VOLUME: %d/21", audio.getVolume());
    lv_label_set_text(volume_label, vol_buf);
    lv_obj_set_style_text_font(volume_label, &lv_font_montserrat_14, 0);
    lv_obj_set_style_text_color(volume_label, lv_color_hex(0xFFFFFF), 0);
    lv_obj_align_to(volume_label, volume_bar, LV_ALIGN_OUT_TOP_MID, 0, -5);
    
    // Статус воспроизведения
    status_label = lv_label_create(scr);
    lv_label_set_text(status_label, audio.isPlaying() ? "▶ PLAYING" : "⏸ PAUSED");
    lv_obj_set_style_text_font(status_label, &lv_font_montserrat_14, 0);
    lv_obj_set_style_text_color(status_label, audio.isPlaying() ? lv_color_hex(0x00FF00) : lv_color_hex(0xFF0000), 0);
    lv_obj_align(status_label, LV_ALIGN_BOTTOM_MID, 0, -20);
    
    // Басы (скрыты по умолчанию)
    bass_label = lv_label_create(scr);
    lv_label_set_text(bass_label, "BASS: 0 dB");
    lv_obj_set_style_text_font(bass_label, &lv_font_montserrat_14, 0);
    lv_obj_set_style_text_color(bass_label, lv_color_hex(0xFFFFFF), 0);
    lv_obj_align(bass_label, LV_ALIGN_CENTER, 0, 0);
    lv_obj_add_flag(bass_label, LV_OBJ_FLAG_HIDDEN);
    
    // Высокие (скрыты по умолчанию)
    treble_label = lv_label_create(scr);
    lv_label_set_text(treble_label, "TREBLE: 0 dB");
    lv_obj_set_style_text_font(treble_label, &lv_font_montserrat_14, 0);
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
    
    // Запуск первой станции
    AudioManager::getInstance().connectToStream(station_urls[current_station]);
    
    ESP_LOGI(TAG, "UI task ready");
    
    while (true) {
        s_encoder->update();
        display_update();
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