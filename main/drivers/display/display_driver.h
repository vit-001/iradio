#ifndef DISPLAY_DRIVER_H
#define DISPLAY_DRIVER_H

#include "lvgl.h"

// Инициализация дисплея и LVGL
bool display_driver_init(void);

// Получить указатель на текущий экран LVGL
lv_obj_t* display_get_scr_act(void);

// Обновить LVGL (вызывать в цикле)
void display_update(void);

// Управление подсветкой
void display_backlight(bool on);
void display_set_brightness(uint8_t percent);

#endif // DISPLAY_DRIVER_H