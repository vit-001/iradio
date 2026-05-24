#include "encoder.h"
#include "esp_log.h"

static const char* TAG = "ENCODER";

Encoder::Encoder(int pinA, int pinB, int pinBtn)
    : _pinA(pinA), _pinB(pinB), _pinBtn(pinBtn)
    , _lastA(HIGH), _lastB(HIGH), _position(0), _lastEvent(ENCODER_IDLE)
    , _callback(nullptr), _callbackData(nullptr)
    , _lastTurnTime(0), _lastButtonTime(0), _buttonPressStartTime(0)
    , _buttonPressCount(0), _buttonProcessed(false)
    , _longPressTime(1000), _doubleClickTime(300) {
}

void Encoder::begin() {
    pinMode(_pinA, INPUT_PULLUP);
    pinMode(_pinB, INPUT_PULLUP);
    pinMode(_pinBtn, INPUT_PULLUP);
    _lastA = digitalRead(_pinA);
    _lastB = digitalRead(_pinB);
    ESP_LOGI(TAG, "Initialized on pins A=%d, B=%d, BTN=%d", _pinA, _pinB, _pinBtn);
}

void Encoder::update() {
    handleTurn();
    handleButton();
}

void Encoder::setCallback(EncoderCallback callback, void* userData) {
    _callback = callback;
    _callbackData = userData;
}

EncoderEvent Encoder::getLastEvent() {
    EncoderEvent event = _lastEvent;
    _lastEvent = ENCODER_IDLE;
    return event;
}

void Encoder::setLongPressTime(unsigned long ms) {
    _longPressTime = ms;
}

void Encoder::setDoubleClickTime(unsigned long ms) {
    _doubleClickTime = ms;
}

int Encoder::getPosition() {
    return _position;
}

void Encoder::resetPosition() {
    _position = 0;
}

bool Encoder::isButtonPressed() {
    return digitalRead(_pinBtn) == LOW;
}

void Encoder::handleTurn() {
    int a = digitalRead(_pinA);
    int b = digitalRead(_pinB);
    
    if (a != _lastA && a == LOW && (millis() - _lastTurnTime) > 5) {
        _lastTurnTime = millis();
        if (b == HIGH) {
            _position--;
            _lastEvent = ENCODER_TURN_LEFT;
            ESP_LOGD(TAG, "Turn LEFT, position=%d", _position);
        } else {
            _position++;
            _lastEvent = ENCODER_TURN_RIGHT;
            ESP_LOGD(TAG, "Turn RIGHT, position=%d", _position);
        }
        if (_callback) {
            _callback(_lastEvent, _callbackData);
        }
    }
    _lastA = a;
    _lastB = b;
}

void Encoder::handleButton() {
    bool pressed = (digitalRead(_pinBtn) == LOW);
    unsigned long now = millis();
    
    if (pressed && !_buttonProcessed) {
        if (_buttonPressStartTime == 0) {
            _buttonPressStartTime = now;
        }
        if (!_buttonProcessed && (now - _buttonPressStartTime) > _longPressTime) {
            _buttonProcessed = true;
            _lastEvent = ENCODER_BUTTON_LONG;
            ESP_LOGI(TAG, "Button LONG press");
            if (_callback) {
                _callback(ENCODER_BUTTON_LONG, _callbackData);
            }
        }
    }
    
    if (!pressed && _buttonPressStartTime > 0) {
        unsigned long pressDuration = now - _buttonPressStartTime;
        
        if (!_buttonProcessed && pressDuration < _longPressTime) {
            _buttonPressCount++;
            _lastButtonTime = now;
        }
        
        _buttonPressStartTime = 0;
        _buttonProcessed = false;
        
        static unsigned long lastPressEnd = 0;
        if (now - lastPressEnd > _doubleClickTime) {
            if (_buttonPressCount == 1) {
                _lastEvent = ENCODER_BUTTON_SHORT;
                ESP_LOGI(TAG, "Button SHORT press");
                if (_callback) {
                    _callback(ENCODER_BUTTON_SHORT, _callbackData);
                }
            } else if (_buttonPressCount >= 2) {
                _lastEvent = ENCODER_BUTTON_DOUBLE;
                ESP_LOGI(TAG, "Button DOUBLE press");
                if (_callback) {
                    _callback(ENCODER_BUTTON_DOUBLE, _callbackData);
                }
            }
            _buttonPressCount = 0;
        }
        lastPressEnd = now;
    }
}