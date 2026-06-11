#pragma once

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

// Запуск аудио задачи на указанном ядре
void startAudioTask(int core, int priority, int stackSize);

// Получение handle задачи (для мониторинга)
TaskHandle_t getAudioTaskHandle();

// Остановка аудио задачи
void stopAudioTask();

