// encoder.cpp

#include "encoder.h"
#include "config.h"

#include <Arduino.h>

namespace {

// Таблица переходов квадратурного энкодера
constexpr int8_t ENC_STATES[16] = {
     0, -1,  1,  0,
     1,  0,  0, -1,
    -1,  0,  0,  1,
     0,  1, -1,  0
};

}

// -----------------------------------------------------------------------------
// Конструктор
// -----------------------------------------------------------------------------
Encoder::Encoder(
    int id,
    int pinA,
    int pinB,
    int pinBtn,
    bool reverse)
    : _id(id)
    , _pinA(pinA)
    , _pinB(pinB)
    , _pinBtn(pinBtn)
    , _reverse(reverse)
{
    pinMode(_pinA, INPUT_PULLUP);
    pinMode(_pinB, INPUT_PULLUP);
    pinMode(_pinBtn, INPUT_PULLUP);

    uint8_t a = digitalRead(_pinA);
    uint8_t b = digitalRead(_pinB);

    _oldAB = (a << 1) | b;
}

// -----------------------------------------------------------------------------
// Установка callback
// -----------------------------------------------------------------------------
void Encoder::setCallback(EncoderCallback callback)
{
    _callback = callback;
}

// -----------------------------------------------------------------------------
// Основной цикл
// -----------------------------------------------------------------------------
void Encoder::update()
{
    handleRotation();
    handleButton();
}

// -----------------------------------------------------------------------------
// Обработка вращения
// -----------------------------------------------------------------------------
void Encoder::handleRotation()
{
    uint8_t a = digitalRead(_pinA);
    uint8_t b = digitalRead(_pinB);

    uint8_t newAB = (a << 1) | b;

    uint8_t index = (_oldAB << 2) | newAB;

    _delta += ENC_STATES[index];

    _oldAB = newAB;

    // Один щелчок энкодера = 4 перехода
    if (_delta >= 4)
    {
        _delta = 0;

        if (_reverse)
            emitEvent(EncoderEventType::TurnLeft);
        else
            emitEvent(EncoderEventType::TurnRight);
    }
    else if (_delta <= -4)
    {
        _delta = 0;

        if (_reverse)
            emitEvent(EncoderEventType::TurnRight);
        else
            emitEvent(EncoderEventType::TurnLeft);
    }
}

// -----------------------------------------------------------------------------
// Обработка кнопки
// -----------------------------------------------------------------------------
void Encoder::handleButton()
{
    bool pressed = (digitalRead(_pinBtn) == LOW);

    uint32_t now = millis();

    // Нажатие
    if (pressed && !_buttonPressed)
    {
        _buttonPressed = true;
        _longPressSent = false;
        _pressStartTime = now;
    }

    // Удержание
    if (pressed && !_longPressSent)
    {
        if ((now - _pressStartTime) >= ENCODER_LONG_PRESS_TIME)
        {
            _longPressSent = true;
            emitEvent(EncoderEventType::ButtonLong);
        }
    }

    // Отпускание
    if (!pressed && _buttonPressed)
    {
        _buttonPressed = false;

        // После long ничего больше не генерируем
        if (_longPressSent)
            return;

        _clickCount++;
        _lastReleaseTime = now;
    }

    // Ожидание второго клика закончилось
    if (_clickCount > 0)
    {
        if ((now - _lastReleaseTime) >= ENCODER_DOUBLE_CLICK_TIME)
        {
            if (_clickCount == 1)
            {
                emitEvent(EncoderEventType::ButtonShort);
            }
            else
            {
                emitEvent(EncoderEventType::ButtonDouble);
            }

            _clickCount = 0;
        }
    }
}

// -----------------------------------------------------------------------------
// Отправка события
// -----------------------------------------------------------------------------
void Encoder::emitEvent(EncoderEventType type)
{
    if (_callback)
    {
        EncoderEvent event;

        event.encoderId = _id;
        event.type = type;

        _callback(event);
    }
}
