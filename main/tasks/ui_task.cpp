/**
 * @file ui_task.cpp
 * @brief Задача пользовательского интерфейса
 * 
 * Отвечает за:
 * - Инициализацию дисплея и LVGL
 * - Создание и управление экранами через ScreenManager
 * - Обработку сообщений от AudioTask
 * - Обработку событий энкодера
 */

#include "ui_task.h"
#include "config.h"
#include "drivers/encoder/encoder.h"
#include "drivers/display/display_driver.h"
#include "drivers/nvs/nvs_manager.h"
#include "ui/screen_manager.h"
#include "ui/root_screen.h"
#include "ui/status_bars/top_bar.h"
#include "ui/status_bars/bottom_bar.h"
#include "ui/screens/volume_screen.h"
#include "ui/screens/eq_screen.h"
#include "ui/screens/station_screen.h"
#include "messages/audio_to_ui_messages.h"
#include "esp_log.h"

static const char* TAG = "UI_TASK";

static TaskHandle_t s_uiTaskHandle = NULL;
static Encoder* s_encoder = nullptr;
static Encoder* s_encoder_2 = nullptr;

static unsigned long s_lastActivity = 0;
static bool s_displayAwake = true;
static const unsigned long DISPLAY_SLEEP_TIMEOUT = DISPLAY_SLEEP_TIMEOUT_MS;

// Внешняя очередь для сообщений от AudioTask (объявлена в main.cpp)
extern QueueHandle_t audioToUIQueue;

// Указатели на объекты LVGL (хранятся здесь, так как менеджер не владеет памятью)
static RootScreen* s_rootScreen = nullptr;
static TopBar* s_topBar = nullptr;
static BottomBar* s_bottomBar = nullptr;
static VolumeScreen* s_volumeScreen = nullptr;
static EQScreen* s_eqScreen = nullptr;
static StationScreen* s_stationScreen = nullptr;

/**
 * @brief Инициализация всех экранов
 * @param mgr ссылка на менеджер экранов
 */
static void initScreens(ScreenManager& mgr) {
    // Создаём контейнеры и общие элементы интерфейса
    s_rootScreen = new RootScreen();

    s_topBar = new TopBar(s_rootScreen->getTopBarContainer());
    s_bottomBar = new BottomBar(s_rootScreen->getBottomBarContainer());
    s_rootScreen->Show();

    // Создаём экраны (передаём указатель на менеджер для навигации)
    s_volumeScreen = new VolumeScreen(&mgr, s_rootScreen->getContentContainer());
    s_stationScreen = new StationScreen(&mgr, s_rootScreen->getContentContainer());
    s_eqScreen = new EQScreen(&mgr, s_rootScreen->getContentContainer());
    
    // Добавляем экраны в менеджер (порядок определяет порядок переключения)
    mgr.addScreen(s_volumeScreen);
    mgr.addScreen(s_stationScreen);
    mgr.addScreen(s_eqScreen);

    // Устанавливаем указатели на панели в менеджере, чтобы экраны могли их обновлять
    mgr.setTopBar(s_topBar);
    mgr.setBottomBar(s_bottomBar);
    
    // Создаём LVGL объекты для каждого экрана, сразу их скрывая
    s_volumeScreen->create();
    s_volumeScreen->hide();

    s_stationScreen->create();
    s_stationScreen->hide();

    s_eqScreen->create();
    s_eqScreen->hide();
    
    ESP_LOGI(TAG, "All screens initialized");
}

/**
 * @brief Callback для событий энкодера
 * @param event тип события
 * @param userData пользовательские данные (не используются)
 */
static void onEncoderEvent(EncoderEvent event, void* userData) {
    ScreenManager& mgr = ScreenManager::getInstance();

    // Сбрасываем таймер при любом действии пользователя
    s_lastActivity = millis();
    
    // Если дисплей в режиме сна — будим
    if (!s_displayAwake) {
        display_wake();
        s_displayAwake = true;
        // Перерисовываем текущий экран
        mgr.updateCurrent();
    }
    
    switch (event) {
        case ENCODER_TURN_RIGHT:
            mgr.onTurnRight(1);
            break;
        case ENCODER_TURN_LEFT:
            mgr.onTurnLeft(1);
            break;
        case ENCODER_BUTTON_SHORT:
            mgr.onShortPress(1);
            break;
        case ENCODER_BUTTON_LONG:
            mgr.onLongPress(1);
            break;
        case ENCODER_BUTTON_DOUBLE:
            mgr.onDoublePress(1);
            break;
        default:
            break;
    }
}

/**
 * @brief Callback для событий энкодера №2
 * @param event тип события
 * @param userData пользовательские данные (не используются)
 */
static void onEncoder2Event(EncoderEvent event, void* userData) {
    ScreenManager& mgr = ScreenManager::getInstance();

    // Сбрасываем таймер при любом действии пользователя
    s_lastActivity = millis();
    
    // Если дисплей в режиме сна — будим
    if (!s_displayAwake) {
        display_wake();
        s_displayAwake = true;
        // Перерисовываем текущий экран
        mgr.updateCurrent();
    }
    
    switch (event) {
        case ENCODER_TURN_RIGHT:
            mgr.onTurnRight(2);
            break;
        case ENCODER_TURN_LEFT:
            mgr.onTurnLeft(2);
            break;
        case ENCODER_BUTTON_SHORT:
            mgr.onShortPress(2);
            break;
        case ENCODER_BUTTON_LONG:
            mgr.onLongPress(2);
            break;
        case ENCODER_BUTTON_DOUBLE:
            mgr.onDoublePress(2);
            break;
        default:
            break;
    }
}

/**
 * @brief Главная функция задачи UI
 */
void uiTaskFunction(void* parameter) {
    ESP_LOGI(TAG, "Started on core %d", xPortGetCoreID());
    
    // ==================== Инициализация дисплея ====================
    if (!display_driver_init()) {
        ESP_LOGE(TAG, "Failed to init display!");
        while (1) vTaskDelay(pdMS_TO_TICKS(1000));
    }
    
    // ==================== Инициализация экранов ====================
    ScreenManager& mgr = ScreenManager::getInstance();
    initScreens(mgr);
    
    // Показываем первый экран (VolumeScreen)
    mgr.switchTo(0);
    
    // ==================== Инициализация энкодера ====================
    s_encoder = new Encoder(ENC_A, ENC_B, ENC_BTN);
    s_encoder->begin();
    s_encoder->setCallback(onEncoderEvent);
    s_encoder->setLongPressTime(800);      // 800 мс для долгого нажатия
    s_encoder->setDoubleClickTime(300);    // 300 мс между нажатиями для двойного
  
    // ==================== Инициализация энкодера 2====================
    s_encoder_2 = new Encoder(ENC_2_A, ENC_2_B, ENC_2_BTN);
    s_encoder_2->begin();
    s_encoder_2->setCallback(onEncoder2Event);
    s_encoder_2->setLongPressTime(800);      // 800 мс для долгого нажатия
    s_encoder_2->setDoubleClickTime(300);    // 300 мс между нажатиями для двойного    

    s_lastActivity = millis();
    s_displayAwake = true;

    ESP_LOGI(TAG, "UI task ready, waiting for audio events");
    
    // ==================== Основной цикл ====================
    while (true) {
        // Обработка сообщений от AudioTask
        AudioToUIMessage msg;
        if (xQueueReceive(audioToUIQueue, &msg, 0) == pdTRUE) {
            // Рассылаем событие всем экранам через менеджер
            mgr.handleAudioEvent(msg);
            
            // Логируем полученные события (для отладки)
            switch (msg.type) {
                // case EVENT_PLAYBACK_INFO:
                //     ESP_LOGD(TAG, "Received playback info: station='%s', song='%s', vol=%d, playing=%d",
                //              msg.data.playback.station_name,
                //              msg.data.playback.song_title,
                //              msg.data.playback.volume,
                //              msg.data.playback.is_playing);
                //     break;
                    
                case EVENT_VOLUME_CHANGED:
                    ESP_LOGD(TAG, "Received volume changed: %d", msg.data.volume);
                    break;
                      
                case EVENT_WIFI_STATUS:
                    ESP_LOGD(TAG, "Received WiFi status: rssi=%d, connected=%b",
                             msg.data.wifi.rssi, msg.data.wifi.is_connected);
                    break;
                case EVENT_EQ_VALUES:
                    ESP_LOGD(TAG, "Received EQ values: B=%d, M=%d, T=%d",
                             msg.data.eq.bass, msg.data.eq.mid, msg.data.eq.treble);
                    break;
                default:
                    break;
            }
        }
        
        // Обновление энкодера
        s_encoder->update();
        s_encoder_2->update();

        // Проверка таймера бездействия
        if (DISPLAY_SLEEP_TIMEOUT > 0 && s_displayAwake && (millis() - s_lastActivity) > DISPLAY_SLEEP_TIMEOUT) {
            display_sleep();
            s_displayAwake = false;
            ESP_LOGI(TAG, "Display put to sleep due to inactivity");
        }        
        
        // Обновление LVGL (обработка таймеров, анимаций и т.д.)
        display_update();

        // Отработка задержки записи в NVS
        NVSManager::getInstance().processDelayedSave();
        
        // Небольшая задержка для стабильности
        vTaskDelay(pdMS_TO_TICKS(5));
    }
}

// ==================== Публичные функции ====================

void startUiTask(int core, int priority, int stackSize) {
    xTaskCreatePinnedToCore(uiTaskFunction, "UITask", stackSize, NULL, priority, &s_uiTaskHandle, core);
}

TaskHandle_t getUiTaskHandle() {
    return s_uiTaskHandle;
}