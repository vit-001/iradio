#ifndef AUDIO_MESSAGES_H
#define AUDIO_MESSAGES_H

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"

// Типы команд
enum AudioCommandType {
    CMD_SET_VOLUME = 0,
    CMD_SET_TONE,
    CMD_PLAY_URL,
    CMD_PLAY_PAUSE,
    CMD_NONE
};

// Структура сообщения
struct AudioMessage {
    AudioCommandType type;
    int value1;   // для volume, для tone: bass
    int value2;   // для tone: mid
    int value3;   // для tone: treble
    char url[256]; // для play_url
};

// Глобальная очередь (объявляем в main.cpp)
extern QueueHandle_t audioQueue;

#endif // AUDIO_MESSAGES_H