// audio_manager.cpp
#include "audio_manager.h"
#include "audio_to_ui_messages.h"
#include "esp_log.h"

static const char* TAG = "AUDIO_MGR";

extern QueueHandle_t audioToUIQueue;

// -----------------------------------------------------------------------------
// Конструктор и деструктор
// -----------------------------------------------------------------------------
AudioManager::AudioManager() 
    : _volume(10),
      _isPlaying(false) 
      {
    _currentUrl[0] = '\0';
}

AudioManager::~AudioManager() {
}

// -----------------------------------------------------------------------------
// Singleton: доступ к единственному экземпляру AudioManager
// -----------------------------------------------------------------------------
AudioManager& AudioManager::getInstance() {
    static AudioManager instance;
    return instance;
}

// -----------------------------------------------------------------------------
// Инициализация аудиоподсистемы
// -----------------------------------------------------------------------------
void AudioManager::begin(int bckPin, int lrcPin, int doutPin) {
    ESP_LOGI(TAG, "Initializing I2S: BCLK=%d, LRC=%d, DOUT=%d", bckPin, lrcPin, doutPin);
    _audio.setPinout(bckPin, lrcPin, doutPin);
    _audio.setVolume(_volume);
    ESP_LOGI(TAG, "Audio manager initialized");
}

// -----------------------------------------------------------------------------
// Установка Wi-Fi соединения (блокирующая операция при старте)
// -----------------------------------------------------------------------------
bool AudioManager::connectWiFi() {
    if (!_ssid || !_password) return false;

    ESP_LOGI(TAG, "Connecting to WiFi SSID: %s", _ssid);
    WiFi.begin(_ssid, _password);

    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        attempts++;
        ESP_LOGD(TAG, "Waiting for WiFi... (%d sec)", attempts / 2);
        if (attempts > 30) {
            ESP_LOGW(TAG, "WiFi timeout, retrying...");
            attempts = 0;
            WiFi.reconnect();
        }
    }

    ESP_LOGI(TAG, "WiFi connected! IP: %s RSSI: %d",
             WiFi.localIP().toString().c_str(), WiFi.RSSI());
    return true;
}

// -----------------------------------------------------------------------------
// Установка SSID и пароля Wi-Fi
// -----------------------------------------------------------------------------
void AudioManager::setWiFiCredentials(const char* ssid, const char* password) {
    _ssid = ssid;
    _password = password;
}

// -----------------------------------------------------------------------------
// Основной цикл AudioManager
// 1. Обработка аудиопотока ESP32-audioI2S
// 2. Контроль состояния Wi-Fi
// 3. Контроль заполнения входного буфера
// 4. Выполнение state machine воспроизведения
// -----------------------------------------------------------------------------
void AudioManager::loop() {
    _audio.loop();

    uint32_t now = millis();

    // -------------------------------------------------------------------------
    // Проверка Wi-Fi с таймером
    // -------------------------------------------------------------------------
    if (now - _lastWiFiCheck >= WIFI_CHECK_INTERVAL) {
        _lastWiFiCheck = now;

        bool connected = (WiFi.status() == WL_CONNECTED);
        int rssi = connected ? WiFi.RSSI() : 0;

        // Если статус изменился, отправляем событие в UI
        if(abs(rssi - _wifiRSSI) >= 2 || connected != _wifiConnected){
            _wifiConnected = connected;
            _wifiRSSI = rssi;
            sendWiFiEvent();
        }

        // Если Wi-Fi пропал — инициируем переподключение
        if (!connected && (currentState == PlaybackState::Playing || currentState == PlaybackState::Reconnecting))  {
            ESP_LOGW(TAG, "Wi-Fi lost, reconnecting...");
            WiFi.reconnect();
        }
    }

    // -------------------------------------------------------------------------
    // Таймерная проверка заполнения буфера
    // -------------------------------------------------------------------------
    if (now - _lastBufferCheck >= BUFFER_CHECK_INTERVAL) {
        _lastBufferCheck = now;

        // Проверяем буфер только если поток воспроизводится
        if (currentState == PlaybackState::Playing) {
            uint32_t filled = _audio.inBufferFilled();
            ESP_LOGD(TAG, "Buffer status: filled=%u bytes", filled);

            // Если буфер опустел — переходим в Reconnecting
            if (filled == 0) {
                ESP_LOGW(TAG, "Buffer empty, starting reconnect...");
                setState(PlaybackState::Reconnecting);

                // Авто-переподключение убрано, теперь централизованно в processReconnecting
            }
        }
    }

    // -------------------------------------------------------------------------
    // State machine воспроизведения
    // -------------------------------------------------------------------------
    switch (currentState) {
        case PlaybackState::Idle:
            // Ничего не делаем, ждём действий пользователя
            break;

        case PlaybackState::Connecting:
            // Когда появится буфер — начинаем воспроизведение
            if (_audio.inBufferFilled() > 0) {
                setState(PlaybackState::Playing);
            }
            break;

        case PlaybackState::Playing:
            // Логика управления Playing уже в таймерной проверке буфера
            break;

        case PlaybackState::Paused:
            // Пока пользователь нажал pause — ничего не делаем
            break;

        case PlaybackState::Reconnecting:
            processReconnecting();
            break;

        default:
            break;
    }
}

// -----------------------------------------------------------------------------
// Обработка Reconnecting
// -----------------------------------------------------------------------------
void AudioManager::processReconnecting() {
    // Если нет Wi-Fi — пробуем восстановить соединение
    if (!_wifiConnected) {
        WiFi.reconnect();
        return;
    }

    // Если Wi-Fi есть и URL известен — переподключаемся к станции
    if (_currentUrl[0] != '\0') {
        ESP_LOGI(TAG, "Reconnecting stream: %s", _currentUrl);
        _audio.connecttohost(_currentUrl);
        setState(PlaybackState::Connecting);
    }
}

// -----------------------------------------------------------------------------
// Отправка состояния воспроизведения в UI
// -----------------------------------------------------------------------------
void AudioManager::sendPlaybackStateEvent() {
    AudioToUIMessage msg;

    msg.type = EVENT_PLAYBACK_STATE_CHANGED;
    msg.data.playbackState.state = currentState;
    msg.data.playbackState.is_playing = (currentState == PlaybackState::Playing);

    xQueueSend(audioToUIQueue, &msg, 0);
}

// -----------------------------------------------------------------------------
// Отправка статуса Wi-Fi в UI
// -----------------------------------------------------------------------------
void AudioManager::sendWiFiEvent() {
    AudioToUIMessage msg;

    msg.type = EVENT_WIFI_STATUS;
    msg.data.wifi.is_connected = _wifiConnected;
    msg.data.wifi.rssi = _wifiRSSI;

    xQueueSend(audioToUIQueue,&msg,0);
}

// -----------------------------------------------------------------------------
// Управление громкостью
// -----------------------------------------------------------------------------
void AudioManager::setDefaultVolume(int vol) {
    if (vol < 0) vol = 0;
    if (vol > 21) vol = 21;
    _volume = vol;
    ESP_LOGI(TAG, "Default volume set to %d", _volume);
}

void AudioManager::setTone(int8_t bass, int8_t mid, int8_t treble) {
    ESP_LOGI(TAG, "Setting tone: BASS=%d, MID=%d, TREBLE=%d", bass, mid, treble);
    _audio.setTone(bass, mid, treble);
}

void AudioManager::setVolume(int vol) {
    if (vol < 0) vol = 0;
    if (vol > 21) vol = 21;
    _volume = vol;
    _audio.setVolume(_volume);
    ESP_LOGI(TAG, "Volume set to %d", _volume);
}

void AudioManager::volumeUp() {
    setVolume(_volume + 1);
}

void AudioManager::volumeDown() {
    setVolume(_volume - 1);
}

// -----------------------------------------------------------------------------
// Управление воспроизведением
// -----------------------------------------------------------------------------
void AudioManager::play() {
    if (!_isPlaying && _currentUrl[0] != '\0') {
        ESP_LOGI(TAG, "Starting playback");
        setState(PlaybackState::Connecting);
        _audio.pauseResume();
        _isPlaying = true;
    }
}

void AudioManager::pause() {
    if (_isPlaying) {
        ESP_LOGI(TAG, "Pausing playback");
        _audio.pauseResume();
        _isPlaying = false;
        setState(PlaybackState::Idle);
    }
}

void AudioManager::playPause() {
    if (_isPlaying) {
        pause();
    } else {
        play();
    }
}

// -----------------------------------------------------------------------------
// Подключение к новой станции
// -----------------------------------------------------------------------------
void AudioManager::connectToStream(const char* url) {
    strncpy(_currentUrl, url, sizeof(_currentUrl) - 1);
    _currentUrl[sizeof(_currentUrl) - 1] = '\0';

    ESP_LOGI(TAG, "Connecting to stream: %s", _currentUrl);

    setState(PlaybackState::Connecting);
    _audio.connecttohost(_currentUrl);
    _isPlaying = true;
}

// -----------------------------------------------------------------------------
// Получение текущего состояния
// -----------------------------------------------------------------------------
PlaybackState AudioManager::getState() const {
    return currentState;
}

// -----------------------------------------------------------------------------
// Преобразование состояния в строку для логирования
// -----------------------------------------------------------------------------
static const char* playbackStateToString(PlaybackState state) {
    switch (state)
    {
        case PlaybackState::Idle:
            return "Idle";
        case PlaybackState::Connecting:
            return "Connecting";
        case PlaybackState::Playing:
            return "Playing";
        case PlaybackState::Reconnecting:
            return "Reconnecting";
        case PlaybackState::Paused:
            return "Paused";
        default:
            return "Unknown";
    }
}

// -----------------------------------------------------------------------------
// Изменение состояния воспроизведения
// -----------------------------------------------------------------------------
void AudioManager::setState(PlaybackState state) {
    if (currentState == state)
        return;

    currentState = state;

    ESP_LOGI(TAG,
             "Playback state -> %s",
             playbackStateToString(state));

    // Уведомляем UI о новом состоянии
    sendPlaybackStateEvent();
}

// -----------------------------------------------------------------------------
// Отладочная функция вывода статуса буфера
// -----------------------------------------------------------------------------
void AudioManager::inBufferStatus() {
    _audio.inBufferStatus();
}