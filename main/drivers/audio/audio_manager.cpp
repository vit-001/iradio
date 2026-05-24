#include "audio_manager.h"
#include "esp_log.h"

static const char* TAG = "AUDIO_MGR";

AudioManager::AudioManager() 
    : _volume(10), _isPlaying(false), _currentUrl(nullptr) {
}

AudioManager::~AudioManager() {
}

AudioManager& AudioManager::getInstance() {
    static AudioManager instance;
    return instance;
}

void AudioManager::loop() {
    _audio.loop();
}

void AudioManager::setDefaultVolume(int vol) {
    if (vol < 0) vol = 0;
    if (vol > 21) vol = 21;
    _volume = vol;
    ESP_LOGI(TAG, "Default volume set to %d", _volume);
}

void AudioManager::begin(int bckPin, int lrcPin, int doutPin) {
    ESP_LOGI(TAG, "Initializing I2S: BCLK=%d, LRC=%d, DOUT=%d", bckPin, lrcPin, doutPin);
    _audio.setPinout(bckPin, lrcPin, doutPin);
    _audio.setVolume(_volume);
    ESP_LOGI(TAG, "Audio manager initialized");
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

void AudioManager::play() {
    if (!_isPlaying && _currentUrl) {
        ESP_LOGI(TAG, "Starting playback");
        _audio.connecttohost(_currentUrl);
        _isPlaying = true;
    }
}

void AudioManager::pause() {
    if (_isPlaying) {
        ESP_LOGI(TAG, "Pausing playback");
        _audio.stopSong();
        _isPlaying = false;
    }
}

void AudioManager::playPause() {
    if (_isPlaying) {
        pause();
    } else {
        play();
    }
}

void AudioManager::connectToStream(const char* url) {
    ESP_LOGI(TAG, "Connecting to stream: %s", url);
    _currentUrl = url;
    _audio.connecttohost(url);
    _isPlaying = true;
}