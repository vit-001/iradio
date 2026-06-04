/**
 * @file screen_manager.h
 * @brief Менеджер экранов
 * 
 * Управляет списком экранов и переключением между ними.
 * Делегирует события энкодера текущему экрану.
 * Рассылает события от AudioTask всем экранам.
 * 
 * Менеджер не знает ничего о внутреннем устройстве экранов —
 * он только вызывает их методы и переключает видимость LVGL объектов.
 */

#ifndef UI_SCREEN_MANAGER_H
#define UI_SCREEN_MANAGER_H

#include "messages/audio_to_ui_messages.h"
#include "screens/screen.h"
#include "ui/status_bars/top_bar.h"
#include "ui/status_bars/bottom_bar.h"
#include <vector>

/**
 * @class ScreenManager
 * @brief Синглтон для управления экранами
 * 
 * Использование:
 * 1. Получить экземпляр: ScreenManager::getInstance()
 * 2. Добавить экраны: addScreen(new VolumeScreen(&mgr))
 * 3. Создать LVGL объекты: каждый экран->create()
 * 4. Запустить: switchTo(0)
 * 5. В цикле UI вызывать updateCurrent() и обрабатывать события энкодера
 */
class ScreenManager {
public:
    /**
     * @brief Получить единственный экземпляр менеджера
     * @return ссылка на экземпляр ScreenManager
     */
    static ScreenManager& getInstance();
    
    // ==================== Управление экранами ====================
    
    /**
     * @brief Добавить экран в конец списка
     * @param screen указатель на экран (менеджер не владеет памятью, не удаляет)
     * 
     * @note Экраны добавляются в порядке, соответствующем циклическому переключению.
     *       Первый добавленный экран будет показан при switchTo(0).
     */
    void addScreen(Screen* screen);
    
    /**
     * @brief Переключиться на следующий экран (по кругу)
     * 
     * Вызывается по долгому или двойному нажатию по умолчанию.
     */
    void next();

    /**
     * @brief установить указатель на верхнюю панель
     * @param topBar указатель на верхнюю панель
     */
    void setTopBar(TopBar* topBar) { m_topBar = topBar; }

    
    /**
     * @brief установить указатель на нижнюю панель
     * @param bottomBar указатель на нижнюю панель
     */
    void setBottomBar(BottomBar* bottomBar) { m_bottomBar = bottomBar; }    
    
    /**
     * @brief Переключиться на предыдущий экран (по кругу)
     */
    void previous();
    
    /**
     * @brief Переключиться на экран с указанным индексом
     * @param index индекс экрана (0..count-1)
     * 
     * @note Автоматически скрывает текущий экран, показывает новый,
     *       вызывает onShow/onHide и обновляет LVGL.
     */
    void switchTo(int index);
    
    /**
     * @brief Получить текущий экран
     * @return указатель на текущий экран, или nullptr если нет экранов
     */
    Screen* getCurrentScreen() const;
    
    /**
     * @brief Получить индекс текущего экрана
     * @return индекс, или -1 если нет экранов
     */
    int getCurrentIndex() const { return m_currentIndex; }
    
    /**
     * @brief Получить количество экранов
     */
    int getScreenCount() const { return m_screens.size(); }

    /**
     * @brief Получить указатель на верхнюю панель
     * @return указатель на верхнюю панель, или nullptr если нет экранов
     */
    TopBar* getTopBar() const { return m_topBar; }

        /**
     * @brief Получить указатель на нижнюю панель
     * @return указатель на нижнюю панель, или nullptr если нет экранов
     */
    BottomBar* getBottomBar() const { return m_bottomBar; }

    // ==================== События от AudioTask ====================
    
    /**
     * @brief Обработка события от AudioTask
     * @param msg сообщение из очереди audioToUIQueue
     * 
     * Рассылает событие ВСЕМ экранам (включая неактивные).
     * Это позволяет всем экранам всегда иметь актуальные данные,
     * даже если они не отображаются в данный момент.
     * 
     * @note Неактивные экраны обновляют свои данные, но остаются скрытыми.
     */
    void handleAudioEvent(const AudioToUIMessage& msg);
    
    /**
     * @brief Обновить текущий экран
     * 
     * Вызывается в цикле UI задачи.
     * Нужно только для экранов, которые требуют постоянного обновления
     * (например, анимации, визуализация спектра).
     * 
     * @note Обычно не требуется, так как данные обновляются через handleAudioEvent.
     */
   void updateCurrent() { if (m_currentScreen) m_currentScreen->refresh(); }
    
    // ==================== Делегирование событий энкодера ====================
    
    /**
     * @brief Делегировать поворот вправо текущему экрану
     */
    void onTurnRight()   { if (m_currentScreen) m_currentScreen->onTurnRight(); }
    
    /**
     * @brief Делегировать поворот влево текущему экрану
     */
    void onTurnLeft()    { if (m_currentScreen) m_currentScreen->onTurnLeft(); }
    
    /**
     * @brief Делегировать короткое нажатие текущему экрану
     */
    void onShortPress()  { if (m_currentScreen) m_currentScreen->onShortPress(); }
    
    /**
     * @brief Делегировать долгое нажатие текущему экрану
     */
    void onLongPress()   { if (m_currentScreen) m_currentScreen->onLongPress(); }
    
    /**
     * @brief Делегировать двойное нажатие текущему экрану
     */
    void onDoublePress() { if (m_currentScreen) m_currentScreen->onDoublePress(); }
    
private:
    // Приватный конструктор для синглтона
    ScreenManager() = default;
    
    // Запрещаем копирование
    ScreenManager(const ScreenManager&) = delete;
    ScreenManager& operator=(const ScreenManager&) = delete;
    
    std::vector<Screen*> m_screens;      ///< Список всех экранов
    int m_currentIndex = -1;             ///< Индекс текущего экрана
    Screen* m_currentScreen = nullptr;   ///< Указатель на текущий экран (кэш)

    TopBar* m_topBar = nullptr;          ///< Указатель на верхнюю панель
    BottomBar* m_bottomBar = nullptr;    ///< Указатель на нижнюю панель

};

#endif // UI_SCREEN_MANAGER_H