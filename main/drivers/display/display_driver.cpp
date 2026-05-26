#include "display_driver.h"
#include "config.h"
#include "esp_log.h"
#include "esp_heap_caps.h"
#include "esp_timer.h"
#include "driver/gpio.h"
#include "driver/spi_master.h"
#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_vendor.h"
#include "esp_lcd_panel_ops.h"

static const char* TAG = "DISPLAY_DRV";
static esp_lcd_panel_handle_t panel_handle = NULL;
static lv_disp_t* lvgl_disp = NULL;

// Callback для LVGL — когда нужно обновить экран
static void lvgl_flush_cb(lv_disp_drv_t *drv, const lv_area_t *area, lv_color_t *color_map) {
    esp_lcd_panel_handle_t panel = (esp_lcd_panel_handle_t)drv->user_data;
    esp_lcd_panel_draw_bitmap(panel, area->x1, area->y1, area->x2 + 1, area->y2 + 1, color_map);
    lv_disp_flush_ready(drv);
}

// Callback для DMA
static bool notify_lvgl_flush_ready(esp_lcd_panel_io_handle_t panel_io, 
                                    esp_lcd_panel_io_event_data_t *edata, 
                                    void *user_ctx) {
    lv_disp_drv_t *disp_driver = (lv_disp_drv_t *)user_ctx;
    lv_disp_flush_ready(disp_driver);
    return false;
}

// Callback для таймера LVGL
static void lvgl_tick_task(void *arg) {
    lv_tick_inc(2);
}

bool display_driver_init(void) {
    ESP_LOGI(TAG, "Initializing display driver...");
    
    // 1. Подсветка
    gpio_config_t bk_cfg = {};
    bk_cfg.pin_bit_mask = (1ULL << LCD_BL);
    bk_cfg.mode = GPIO_MODE_OUTPUT;
    gpio_config(&bk_cfg);
    gpio_set_level((gpio_num_t)LCD_BL, 1);
    
    // 2. SPI шина
    spi_bus_config_t buscfg = {};
    buscfg.mosi_io_num = LCD_MOSI;
    buscfg.miso_io_num = -1;
    buscfg.sclk_io_num = LCD_SCLK;
    buscfg.max_transfer_sz = LCD_H_RES * 10 * sizeof(uint16_t);
    ESP_ERROR_CHECK(spi_bus_initialize(SPI2_HOST, &buscfg, SPI_DMA_CH_AUTO));
    
    // 3. Panel IO
    esp_lcd_panel_io_handle_t io_handle = NULL;
    esp_lcd_panel_io_spi_config_t io_config = {};
    io_config.cs_gpio_num = LCD_CS;
    io_config.dc_gpio_num = LCD_DC;
    io_config.spi_mode = 0;
    io_config.pclk_hz = 40000000;
    io_config.trans_queue_depth = 10;
    io_config.lcd_cmd_bits = 8;
    io_config.lcd_param_bits = 8;
    // io_config.flags не трогаем — оставляем как есть
    ESP_ERROR_CHECK(esp_lcd_new_panel_io_spi(SPI2_HOST, &io_config, &io_handle));
    
    // 4. Панель ST7789
    esp_lcd_panel_dev_config_t panel_config = {};
    panel_config.reset_gpio_num = LCD_RST;
    panel_config.rgb_endian = LCD_RGB_ENDIAN_RGB;
    panel_config.bits_per_pixel = 16;
    // panel_config.flags не трогаем
    ESP_ERROR_CHECK(esp_lcd_new_panel_st7789(io_handle, &panel_config, &panel_handle));
    
    ESP_ERROR_CHECK(esp_lcd_panel_reset(panel_handle));
    ESP_ERROR_CHECK(esp_lcd_panel_init(panel_handle));
    ESP_ERROR_CHECK(esp_lcd_panel_invert_color(panel_handle, true));
    
    // Настройки для 170x320 в альбомной ориентации
    ESP_ERROR_CHECK(esp_lcd_panel_set_gap(panel_handle, LCD_GAP_X, LCD_GAP_Y));
    ESP_ERROR_CHECK(esp_lcd_panel_swap_xy(panel_handle, true));
    ESP_ERROR_CHECK(esp_lcd_panel_mirror(panel_handle, false, true));
    ESP_ERROR_CHECK(esp_lcd_panel_disp_on_off(panel_handle, true));
    
    // 5. LVGL инициализация
    lv_init();
    
    // Таймер для LVGL тиков
    esp_timer_handle_t lvgl_tick_timer;
    esp_timer_create_args_t tick_args = {};
    tick_args.callback = lvgl_tick_task;
    tick_args.name = "lvgl_tick";
    ESP_ERROR_CHECK(esp_timer_create(&tick_args, &lvgl_tick_timer));
    ESP_ERROR_CHECK(esp_timer_start_periodic(lvgl_tick_timer, 2000));
    
    // Буфер дисплея (10 строк в PSRAM)
    size_t buf_size = LCD_H_RES * 10 * sizeof(lv_color_t);
    lv_color_t* buf1 = (lv_color_t*)heap_caps_malloc(buf_size, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
    if (!buf1) {
        ESP_LOGE(TAG, "Failed to allocate LVGL buffer");
        return false;
    }
    
    static lv_disp_draw_buf_t draw_buf;
    lv_disp_draw_buf_init(&draw_buf, buf1, NULL, LCD_H_RES * 10);
    
    static lv_disp_drv_t disp_drv;
    lv_disp_drv_init(&disp_drv);
    disp_drv.hor_res = LCD_H_RES;
    disp_drv.ver_res = LCD_V_RES;
    disp_drv.flush_cb = lvgl_flush_cb;
    disp_drv.draw_buf = &draw_buf;
    disp_drv.user_data = panel_handle;
    
    lvgl_disp = lv_disp_drv_register(&disp_drv);
    
    // Регистрация callback для DMA
    esp_lcd_panel_io_callbacks_t io_cbs = {};
    io_cbs.on_color_trans_done = notify_lvgl_flush_ready;
    esp_lcd_panel_io_register_event_callbacks(io_handle, &io_cbs, &disp_drv);
    
    ESP_LOGI(TAG, "Display driver ready! Resolution: %dx%d", LCD_H_RES, LCD_V_RES);
    return true;
}

lv_obj_t* display_get_scr_act(void) {
    return lv_scr_act();
}

void display_update(void) {
    lv_timer_handler();
}

void display_backlight(bool on) {
    gpio_set_level((gpio_num_t)LCD_BL, on ? 1 : 0);
}

void display_set_brightness(uint8_t percent) {
    gpio_set_level((gpio_num_t)LCD_BL, percent > 0 ? 1 : 0);
}