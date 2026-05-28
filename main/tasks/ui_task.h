/**
 * @file ui_task.h
 * @brief Задача пользовательского интерфейса
 * 
 * Отвечает за инициализацию дисплея, экранов и обработку UI.
 */

#ifndef UI_TASK_H
#define UI_TASK_H

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

/**
 * @brief Запуск задачи UI на указанном ядре
 * @param core номер ядра (0 или 1)
 * @param priority приоритет задачи
 * @param stackSize размер стека в байтах
 */
void startUiTask(int core, int priority, int stackSize);

/**
 * @brief Получить handle задачи UI
 * @return handle задачи, или NULL если задача не создана
 */
TaskHandle_t getUiTaskHandle();

#endif // UI_TASK_H