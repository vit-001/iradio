#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_log.h>
#include <esp_heap_caps.h>
#include <esp_timer.h> // Обязательный заголовок для esp_timer_create
#include <driver/gpio.h>
#include <driver/spi_master.h>
#include <esp_lcd_panel_io.h>
#include <esp_lcd_panel_vendor.h>
#include <esp_lcd_panel_ops.h>

// Подключаем заголовки LVGL
#include "lvgl.h"

static const char *TAG = "LVGL_TEST";

// Безопасные пины дисплея (SPI2)
#define LCD_MOSI        6
#define LCD_SCLK        7
#define LCD_DC          5
#define LCD_CS          4
#define LCD_RST         8
#define LCD_BL          9

// Разрешение экрана (в альбомной ориентации для LVGL)
#define LCD_H_RES       320
#define LCD_V_RES       170

static esp_lcd_panel_handle_t panel_handle = NULL;

// Функция-колбэк: вызывается esp_lcd, когда DMA закончил передачу кадра
static bool notify_lvgl_flush_ready(esp_lcd_panel_io_handle_t panel_io, esp_lcd_panel_io_event_data_t *edata, void *user_ctx) {
    lv_disp_drv_t *disp_driver = (lv_disp_drv_t *)user_ctx;
    lv_disp_flush_ready(disp_driver);
    return false;
}

// Функция-колбэк: вызывается LVGL, когда нужно обновить область экрана
static void lvgl_flush_cb(lv_disp_drv_t *drv, const lv_area_t *area, lv_color_t *color_map) {
    esp_lcd_panel_handle_t panel_h = (esp_lcd_panel_handle_t)drv->user_data;
    int offsetx1 = area->x1;
    int offsetx2 = area->x2;
    int offsety1 = area->y1;
    int offsety2 = area->y2;
    
    esp_lcd_panel_draw_bitmap(panel_h, offsetx1, offsety1, offsetx2 + 1, offsety2 + 1, color_map);
}

// Функция тика для внутренних таймеров LVGL
static void lvgl_tick_task(void *arg) {
    (void) arg;
    lv_tick_inc(2); // Сообщаем LVGL, что прошло 2 мс
}

void display_task(void *pvParameters) {
    // 1. Инициализация подсветки
    gpio_config_t bk_gpio_config = {};
    bk_gpio_config.pin_bit_mask = 1ULL << LCD_BL;
    bk_gpio_config.mode = GPIO_MODE_OUTPUT;
    gpio_config(&bk_gpio_config);
    gpio_set_level((gpio_num_t)LCD_BL, 1);

    // 2. Инициализация SPI шины
    spi_bus_config_t buscfg = {};
    buscfg.mosi_io_num = LCD_MOSI;
    buscfg.miso_io_num = -1;
    buscfg.sclk_io_num = LCD_SCLK;
    buscfg.quadwp_io_num = -1;
    buscfg.quadhd_io_num = -1;
    buscfg.max_transfer_sz = LCD_H_RES * 10 * sizeof(uint16_t);
    ESP_ERROR_CHECK(spi_bus_initialize(SPI2_HOST, &buscfg, SPI_DMA_CH_AUTO));

    // 3. Установка Panel IO
    esp_lcd_panel_io_handle_t io_handle = NULL;
    esp_lcd_panel_io_spi_config_t io_config = {};
    io_config.dc_gpio_num = LCD_DC;
    io_config.cs_gpio_num = LCD_CS;
    io_config.pclk_hz = 40000000; 
    io_config.lcd_cmd_bits = 8;
    io_config.lcd_param_bits = 8;
    io_config.spi_mode = 0;
    io_config.trans_queue_depth = 10;
    io_config.on_color_trans_done = notify_lvgl_flush_ready;
    
    ESP_ERROR_CHECK(esp_lcd_new_panel_io_spi((spi_host_device_t)SPI2_HOST, &io_config, &io_handle));

    // 4. Установка драйвера ST7789
    esp_lcd_panel_dev_config_t panel_config = {};
    panel_config.reset_gpio_num = LCD_RST;
    panel_config.rgb_endian = LCD_RGB_ENDIAN_BGR;
    panel_config.data_endian = LCD_RGB_DATA_ENDIAN_BIG;
    panel_config.bits_per_pixel = 16;
    ESP_ERROR_CHECK(esp_lcd_new_panel_st7789(io_handle, &panel_config, &panel_handle));

    ESP_ERROR_CHECK(esp_lcd_panel_reset(panel_handle));
    ESP_ERROR_CHECK(esp_lcd_panel_init(panel_handle));
    ESP_ERROR_CHECK(esp_lcd_panel_invert_color(panel_handle, true));
    
    // Исправлено: Аппаратное зеркалирование и смещение для 170x320 в режиме Landscape
    esp_lcd_panel_set_gap(panel_handle, 0, 35);
    esp_lcd_panel_swap_xy(panel_handle, true);
    esp_lcd_panel_mirror(panel_handle, false, true); // Исправлено имя функции

    ESP_ERROR_CHECK(esp_lcd_panel_disp_on_off(panel_handle, true));
    ESP_LOGI(TAG, "Hardware ST7789 init OK.");

    // ============================================================================
    // ИНИЦИАЛИЗАЦИЯ LVGL
    // ============================================================================
    lv_init();

    // Настройка периодического таймера тиков (2 мс)
    esp_timer_create_args_t lvgl_tick_timer_args = {}; // Исправлено C++ объявление
    lvgl_tick_timer_args.callback = &lvgl_tick_task;
    lvgl_tick_timer_args.name = "lvgl_tick";
    
    esp_timer_handle_t lvgl_tick_timer = NULL;
    ESP_ERROR_CHECK(esp_timer_create(&lvgl_tick_timer_args, &lvgl_tick_timer));
    ESP_ERROR_CHECK(esp_timer_start_periodic(lvgl_tick_timer, 2000)); 

    // Выделяем буфер в PSRAM на 10 строк экрана
    size_t lv_buffer_size = LCD_H_RES * 10 * sizeof(lv_color_t);
    lv_color_t *buf1 = (lv_color_t *)heap_caps_malloc(lv_buffer_size, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
    assert(buf1 != NULL);

    // Исправлено: Правильный тип структуры буфера в LVGL v8
    static lv_disp_draw_buf_t disp_buf; 
    lv_disp_draw_buf_init(&disp_buf, buf1, NULL, LCD_H_RES * 10);

    // Настройка драйвера дисплея LVGL
    static lv_disp_drv_t disp_drv;
    lv_disp_drv_init(&disp_drv);
    disp_drv.hor_res = LCD_H_RES;
    disp_drv.ver_res = LCD_V_RES;
    disp_drv.flush_cb = lvgl_flush_cb;
    disp_drv.draw_buf = &disp_buf;
    disp_drv.user_data = panel_handle;
    
    // Исправлено: Структурированная регистрация колбэка DMA транзакций
    esp_lcd_panel_io_callbacks_t io_cbs = {};
    io_cbs.on_color_trans_done = notify_lvgl_flush_ready;
    esp_lcd_panel_io_register_event_callbacks(io_handle, &io_cbs, &disp_drv);
    
    // Регистрируем дисплей
    lv_disp_drv_register(&disp_drv);
    ESP_LOGI(TAG, "LVGL Init OK. Creating widgets...");

    // ============================================================================
    // ИНТЕРФЕЙС (Тестовая кнопка)
    // ============================================================================
    lv_obj_t *btn = lv_btn_create(lv_scr_act());
    lv_obj_align(btn, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_size(btn, 180, 50);

    lv_obj_t *label = lv_label_create(btn);
    lv_label_set_text(label, "LVGL S3 Working!");
    lv_obj_center(label);

    // Цикл обработки графики
    while (true) {
        vTaskDelay(pdMS_TO_TICKS(10));
        lv_timer_handler(); 
    }
}


void setup(void) {
    xTaskCreatePinnedToCore(display_task, "display_task", 8192, NULL, 5, NULL, 1);
}

void loop(void) {
    // Пустой цикл, так как вся логика выполняется в display_task
    vTaskDelay(1000);
}