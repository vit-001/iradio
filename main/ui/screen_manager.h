/**
 * @file screen_manager.h
 * @brief Менеджер экранов
 *
 * Управляет списком экранов и переключением между ними.
 * Рассылает события энкодера текущему экрану.
 * Рассылает события AudioTask всем экранам.
 */

#pragma once

#include <vector>

#include "messages/audio_to_ui_messages.h"
#include "drivers/encoder/encoder.h"
#include "screens/screen_with_handlers.h"
#include "ui/status_bars/top_bar.h"
#include "ui/status_bars/bottom_bar.h"

/**
 * @class ScreenManager
 * @brief Синглтон для управления экранами
 */
class ScreenManager
{
public:
    /**
     * @brief Получить экземпляр менеджера
     */
    static ScreenManager& getInstance();

    // ==================== Управление экранами ====================

    /**
     * @brief Добавить экран
     */
    void addScreen(ScreenWithHandlers* screen);

    /**
     * @brief Следующий экран
     */
    void next();

    /**
     * @brief Предыдущий экран
     */
    void previous();

    /**
     * @brief Переключение на экран по индексу
     */
    void switchTo(int index);

    /**
     * @brief Получить текущий экран
     */
    ScreenWithHandlers* getCurrentScreen() const;

    /**
     * @brief Индекс текущего экрана
     */
    int getCurrentIndex() const
    {
        return m_currentIndex;
    }

    /**
     * @brief Количество экранов
     */
    int getScreenCount() const
    {
        return m_screens.size();
    }

    // ==================== Панели ====================

    void setTopBar(TopBar* topBar)
    {
        m_topBar = topBar;
    }

    void setBottomBar(BottomBar* bottomBar)
    {
        m_bottomBar = bottomBar;
    }

    TopBar* getTopBar() const
    {
        return m_topBar;
    }

    BottomBar* getBottomBar() const
    {
        return m_bottomBar;
    }

    // ==================== Audio события ====================

    /**
     * @brief Рассылка события AudioTask всем экранам
     */
    void handleAudioEvent(const AudioToUIMessage& msg);

    /**
     * @brief Обновить текущий экран
     */
    void updateCurrent()
    {
        if (m_currentScreen)
        {
            m_currentScreen->refresh();
        }
    }

    // ==================== Энкодер ====================

    /**
     * @brief Передать событие энкодера текущему экрану
     */
    void handleEncoderEvent(const EncoderEvent& event)
    {
        if (m_currentScreen)
        {
            m_currentScreen->handleEncoderEvent(event);
        }
    }

private:
    ScreenManager() = default;

    ScreenManager(const ScreenManager&) = delete;
    ScreenManager& operator=(const ScreenManager&) = delete;

private:
    std::vector<ScreenWithHandlers*> m_screens;

    int m_currentIndex = -1;
    ScreenWithHandlers* m_currentScreen = nullptr;

    TopBar* m_topBar = nullptr;
    BottomBar* m_bottomBar = nullptr;
};