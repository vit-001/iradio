/**
 * @file screen_manager.cpp
 * @brief Реализация менеджера экранов
 */

#include "screen_manager.h"
#include "screens/screen.h"
#include "esp_log.h"

static const char* TAG = "SCREEN_MGR";

// Forward declaration
class Screen;

// ==================== Реализация синглтона ====================

ScreenManager& ScreenManager::getInstance() {
    static ScreenManager instance;
    return instance;
}

// ==================== Управление экранами ====================

void ScreenManager::addScreen(Screen* screen) {
    if (!screen) {
        ESP_LOGE(TAG, "Cannot add null screen");
        return;
    }
    m_screens.push_back(screen);
    ESP_LOGI(TAG, "Screen added, total: %d", (int)m_screens.size());
}

void ScreenManager::switchTo(int index) {
    // Проверка валидности индекса
    if (index < 0 || index >= (int)m_screens.size()) {
        ESP_LOGW(TAG, "Invalid screen index: %d (total: %d)", 
                 index, (int)m_screens.size());
        return;
    }
    
    // Скрываем текущий экран (если есть)
    if (m_currentScreen) {
        m_currentScreen->Hide();
    }
    
    // Переключаемся на новый экран
    m_currentIndex = index;
    m_currentScreen = m_screens[m_currentIndex];
    
    // Делаем контейнер видимым внутри центральной области
    m_currentScreen->Show();
    
    ESP_LOGI(TAG, "Switched to screen index %d", m_currentIndex);
}

void ScreenManager::next() {
    if (m_screens.empty()) {
        ESP_LOGW(TAG, "No screens available, cannot switch to next");
        return;
    }
    int newIndex = (m_currentIndex + 1) % m_screens.size();
    switchTo(newIndex);
}

void ScreenManager::previous() {
    if (m_screens.empty()) {
        ESP_LOGW(TAG, "No screens available, cannot switch to previous");
        return;
    }
    int newIndex = (m_currentIndex - 1 + m_screens.size()) % m_screens.size();
    switchTo(newIndex);
}

Screen* ScreenManager::getCurrentScreen() const {
    return m_currentScreen;
}

// ==================== События от AudioTask ====================

void ScreenManager::handleAudioEvent(const AudioToUIMessage& msg) {
    if (m_screens.empty()) {
        ESP_LOGW(TAG, "No screens to handle audio event");
        return;
    }
    
    // Рассылаем событие ВСЕМ экранам (включая неактивные)
    // Это гарантирует, что при переключении на любой экран данные уже актуальны
    for (auto* screen : m_screens) {
        if (screen) {
            screen->handleAudioEvent(msg);
        }
    }

}