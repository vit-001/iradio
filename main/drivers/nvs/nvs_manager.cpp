#include "nvs_manager.h"
#include "esp_log.h"
#include "esp_timer.h"

static const char* TAG = "NVS";

NVSManager::NVSManager() 
    : m_handle(0)
    , m_initialized(false)
    , m_volume(10)
    , m_station(0)
    , m_bass(0)
    , m_mid(0)
    , m_treble(0)
    , m_pendingSave(false)
    , m_saveTimer(0) {
}

NVSManager::~NVSManager() {
    if (m_initialized) {
        nvs_close(m_handle);
    }
}

NVSManager& NVSManager::getInstance() {
    static NVSManager instance;
    return instance;
}

bool NVSManager::init() {
    if (m_initialized) return true;
    
    esp_err_t err = nvs_open("radio", NVS_READWRITE, &m_handle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to open NVS: %s", esp_err_to_name(err));
        return false;
    }
    
    m_initialized = true;
    loadAllFromNVS();
    ESP_LOGI(TAG, "NVS initialized");
    return true;
}

void NVSManager::loadAllFromNVS() {
    if (!m_initialized) return;
    
    int32_t value;
    
    if (nvs_get_i32(m_handle, "volume", &value) == ESP_OK) {
        m_volume = value;
    }
    if (nvs_get_i32(m_handle, "station", &value) == ESP_OK) {
        m_station = value;
    }
    if (nvs_get_i32(m_handle, "bass", &value) == ESP_OK) {
        m_bass = value;
    }
    if (nvs_get_i32(m_handle, "mid", &value) == ESP_OK) {
        m_mid = value;
    }
    if (nvs_get_i32(m_handle, "treble", &value) == ESP_OK) {
        m_treble = value;
    }
    
    ESP_LOGI(TAG, "Loaded: vol=%d, station=%d, bass=%d, mid=%d, treble=%d",
        m_volume, m_station, m_bass, m_mid, m_treble);
}

void NVSManager::saveAllToNVS() {
    if (!m_initialized) return;
    
    nvs_set_i32(m_handle, "volume", m_volume);
    nvs_set_i32(m_handle, "station", m_station);
    nvs_set_i32(m_handle, "bass", m_bass);
    nvs_set_i32(m_handle, "mid", m_mid);
    nvs_set_i32(m_handle, "treble", m_treble);
    nvs_commit(m_handle);
    
    ESP_LOGI(TAG, "Saved: vol=%d, station=%d, bass=%d, mid=%d, treble=%d",
        m_volume, m_station, m_bass, m_mid, m_treble);
}

int NVSManager::loadVolume(int defaultValue) {
    return m_volume;
}

int NVSManager::loadStation(int defaultValue) {
    return m_station;
}

int NVSManager::loadBass(int defaultValue) {
    return m_bass;
}

int NVSManager::loadMid(int defaultValue) {
    return m_mid;
}

int NVSManager::loadTreble(int defaultValue) {
    return m_treble;
}

void NVSManager::setVolume(int volume) {
    if (volume < 0) volume = 0;
    if (volume > 21) volume = 21;
    if (m_volume != volume) {
        m_volume = volume;
        m_pendingSave = true;
        m_saveTimer = esp_timer_get_time() / 1000 + 10000; // +10 секунд
        ESP_LOGD(TAG, "Volume changed to %d, pending save", volume);
    }
}

void NVSManager::setStation(int station) {
    if (m_station != station) {
        m_station = station;
        m_pendingSave = true;
        m_saveTimer = esp_timer_get_time() / 1000 + 10000;
        ESP_LOGI(TAG, "Station changed to %d, pending save", station);
    }
}

void NVSManager::setBass(int bass) {
    if (bass < -40) bass = -40;
    if (bass > 6) bass = 6;
    if (m_bass != bass) {
        m_bass = bass;
        m_pendingSave = true;
        m_saveTimer = esp_timer_get_time() / 1000 + 10000;
    }
}

void NVSManager::setMid(int mid) {
    if (mid < -40) mid = -40;
    if (mid > 6) mid = 6;
    if (m_mid != mid) {
        m_mid = mid;
        m_pendingSave = true;
        m_saveTimer = esp_timer_get_time() / 1000 + 10000;
    }
}

void NVSManager::setTreble(int treble) {
    if (treble < -40) treble = -40;
    if (treble > 6) treble = 6;
    if (m_treble != treble) {
        m_treble = treble;
        m_pendingSave = true;
        m_saveTimer = esp_timer_get_time() / 1000 + 10000;
    }
}

void NVSManager::commit() {
    if (m_pendingSave) {
        saveAllToNVS();
        m_pendingSave = false;
        ESP_LOGI(TAG, "Settings saved");
    }
}

void NVSManager::processDelayedSave() {
    if (m_pendingSave) {
        unsigned long now = esp_timer_get_time() / 1000;
        if (now >= m_saveTimer) {
            commit();
        }
    }
}