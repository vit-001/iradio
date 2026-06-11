/**
 * @file screen_manager.cpp
 * @brief Реализация менеджера экранов
 */

#include "screen_manager.h"
#include "esp_log.h"

static const char* TAG = "SCREEN_MGR";

// ==================== Singleton ====================

ScreenManager& ScreenManager::getInstance()
{
    static ScreenManager instance;
    return instance;
}

// ==================== Управление экранами ====================

void ScreenManager::addScreen(ScreenWithHandlers* screen)
{
    if (!screen)
    {
        ESP_LOGE(TAG, "Cannot add nullptr screen");
        return;
    }

    m_screens.push_back(screen);

    ESP_LOGI(TAG,
             "Screen added, total=%d",
             (int)m_screens.size());
}

void ScreenManager::switchTo(int index)
{
    if (index < 0 || index >= (int)m_screens.size())
    {
        ESP_LOGW(TAG,
                 "Invalid screen index %d (count=%d)",
                 index,
                 (int)m_screens.size());
        return;
    }

    // скрываем предыдущий экран
    if (m_currentScreen)
    {
        m_currentScreen->hide();
    }

    // переключаемся
    m_currentIndex = index;
    m_currentScreen = m_screens[index];

    // показываем новый экран
    if (m_currentScreen)
    {
        m_currentScreen->show();
        m_currentScreen->refresh();
    }

    ESP_LOGI(TAG,
             "Switched to screen %d",
             m_currentIndex);
}

void ScreenManager::next()
{
    if (m_screens.empty())
    {
        return;
    }

    int nextIndex = (m_currentIndex + 1) % m_screens.size();

    switchTo(nextIndex);
}

void ScreenManager::previous()
{
    if (m_screens.empty())
    {
        return;
    }

    int prevIndex =
        (m_currentIndex - 1 + m_screens.size()) %
        m_screens.size();

    switchTo(prevIndex);
}

ScreenWithHandlers* ScreenManager::getCurrentScreen() const
{
    return m_currentScreen;
}

// ==================== Audio события ====================

void ScreenManager::handleAudioEvent(const AudioToUIMessage& msg)
{
    // Рассылка всем экранам
    for (auto* screen : m_screens)
    {
        if (screen)
        {
            screen->handleAudioEvent(msg);
        }
    }

    // Верхняя панель
    if (m_topBar)
    {
        m_topBar->handleAudioEvent(msg);
    }

    // Нижняя панель
    if (m_bottomBar)
    {
        m_bottomBar->handleAudioEvent(msg);
    }
}