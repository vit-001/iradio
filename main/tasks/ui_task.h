#ifndef UI_TASK_H
#define UI_TASK_H

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

void startUiTask(int core, int priority, int stackSize);
TaskHandle_t getUiTaskHandle();

// Функции для обновления UI из других задач
void ui_update_station(const char* station);
void ui_update_song(const char* song);
void ui_update_volume(int volume);
void ui_update_status(bool is_playing);

#endif // UI_TASK_H