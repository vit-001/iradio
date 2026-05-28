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

// ===== ФУНКЦИИ ДЛЯ УПРАВЛЕНИЯ ПИТАНИЕМ =====

/**
 * @brief Перевести дисплей в режим сна (Sleep In)
 * 
 * Отключает внутренние осцилляторы и драйверы дисплея.
 * Потребление дисплея снижается до ~50-100 мкА.
 * 
 * @note После вызова этого метода дисплей не отображает информацию.
 *       Для восстановления работы нужно вызвать display_wake().
 */
void display_sleep(void);

/**
 * @brief Разбудить дисплей из режима сна (Sleep Out)
 * 
 * Восстанавливает нормальную работу дисплея.
 * Требуется время ~120 мс для стабилизации.
 * 
 * @note После пробуждения необходимо обновить содержимое экрана,
 *       так как данные в ОЗУ дисплея могут быть потеряны.
 */
void display_wake(void);

/**
 * @brief Проверить, находится ли дисплей в режиме сна
 * @return true если в режиме сна, false если активен
 */
bool display_is_asleep(void);

#endif // DISPLAY_DRIVER_H