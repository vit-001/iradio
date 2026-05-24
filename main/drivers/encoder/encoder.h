#ifndef ENCODER_H
#define ENCODER_H

#include <Arduino.h>

// Тип события энкодера
enum EncoderEvent {
    ENCODER_TURN_LEFT = -1,
    ENCODER_TURN_RIGHT = 1,
    ENCODER_BUTTON_SHORT = 2,
    ENCODER_BUTTON_LONG = 3,
    ENCODER_BUTTON_DOUBLE = 4,
    ENCODER_IDLE = 0
};

// Callback тип для событий энкодера
typedef void (*EncoderCallback)(EncoderEvent event, void* userData);

class Encoder {
public:
    // Конструктор
    Encoder(int pinA, int pinB, int pinBtn);
    
    // Инициализация
    void begin();
    
    // Обновление состояния (вызывать в цикле)
    void update();
    
    // Установка callback
    void setCallback(EncoderCallback callback, void* userData = nullptr);
    
    // Получение последнего события (без callback)
    EncoderEvent getLastEvent();
    
    // Настройка времени для длинного нажатия (мс)
    void setLongPressTime(unsigned long ms);
    
    // Настройка времени для двойного нажатия (мс)
    void setDoubleClickTime(unsigned long ms);
    
    // Получить текущее значение поворота (счётчик)
    int getPosition();
    
    // Сбросить позицию
    void resetPosition();
    
    // Получить состояние кнопки (raw)
    bool isButtonPressed();

private:
    // Пины
    int _pinA;
    int _pinB;
    int _pinBtn;
    
    // Состояния
    int _lastA;
    int _lastB;
    int _position;
    EncoderEvent _lastEvent;
    
    // Callback
    EncoderCallback _callback;
    void* _callbackData;
    
    // Антидребезг
    unsigned long _lastTurnTime;
    unsigned long _lastButtonTime;
    unsigned long _buttonPressStartTime;
    int _buttonPressCount;

    // Флаги
    bool _buttonProcessed;

    // Настройки времени
    unsigned long _longPressTime;
    unsigned long _doubleClickTime;
       
    // Внутренние методы
    void handleTurn();
    void handleButton();
};

#endif // ENCODER_H