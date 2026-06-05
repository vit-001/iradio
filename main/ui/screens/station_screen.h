/**
 * @file station_screen.h
 * @brief Экран выбора радиостанции
 * 
 * Отображает список доступных радиостанций.
 * Пользователь может переключаться между станциями и выбирать текущую.
 * 
 * Управление:
 * - Поворот энкодера → переключение между станциями
 * - Короткое нажатие → выбор текущей станции и возврат на главный экран
 * - Долгое/двойное нажатие → переключение на следующий экран (унаследовано от Screen)
 */

#ifndef UI_SCREENS_STATION_SCREEN_H
#define UI_SCREENS_STATION_SCREEN_H

#include "screen_with_handlers.h"

/**
 * @class StationScreen
 * @brief Экран для выбора радиостанции из списка
 */
class StationScreen : public ScreenWithHandlers {
public:
    /**
     * @brief Конструктор
     * @param manager указатель на менеджер экранов
     * @param container указатель на контейнер, в котором будет размещён экран
     */
    explicit StationScreen(ScreenManager* manager, lv_obj_t* parent);
    
    virtual ~StationScreen() = default;
    
    // ==================== Жизненный цикл ====================
    
    /**
     * @brief Создание LVGL экрана и всех виджетов
     * @return указатель на LVGL объект экрана
     */
    lv_obj_t* create() override;
    
    // ==================== События от AudioTask ====================
    
    /**
     * @brief Обработка событий от AudioTask
     * @param msg сообщение из очереди audioToUIQueue
     * 
     * Реагирует на:
     * - EVENT_STATION_CHANGED: обновление текущей станции
     */
    void handleAudioEvent(const AudioToUIMessage& msg) override;
    
    // ==================== События энкодера ====================
    
    /**
     * @brief Поворот вправо → следующая станция
     */
    void onTurnRight() override;
    
    /**
     * @brief Поворот влево → предыдущая станция
     */
    void onTurnLeft() override;
    
    /**
     * @brief Короткое нажатие → выбрать станцию, вернуться на главный экран
     */
    void onShortPress() override;
    
    // onLongPress и onDoublePress оставляем по умолчанию (переключение экрана)
    
    virtual void refresh() override;

    // ==================== Дополнительные методы ====================
    
    /**
     * @brief Установить текущую станцию по индексу
     * @param index индекс станции (0..STATIONS_COUNT-1)
     */
    void setCurrentStation(int index);
    
    /**
     * @brief Получить индекс текущей станции
     */
    int getCurrentStation() const { return m_currentStationIndex; }
    
private:
    // ==================== Вспомогательные методы ====================
    
    /**
     * @brief Обновить отображение текущей станции
     */
    void updateStationDisplay();
    
    /**
     * @brief Отправить команду в AudioTask для переключения на станцию
     * @param index индекс станции
     */
    void switchToStation(int index);
    
    // ==================== Данные экрана ====================
    
    int m_currentStationIndex = 0;        ///< Индекс текущей выбранной станции
    
    // ==================== LVGL виджеты ====================
    
    lv_obj_t* m_modeLabel = nullptr;      ///< Индикатор "STATION" (жёлтый)
    lv_obj_t* m_stationNameLabel = nullptr;   ///< Название текущей станции (крупно)
    lv_obj_t* m_previousLabel = nullptr;  ///< Предыдущая станция (мелко, сверху)
    lv_obj_t* m_nextLabel = nullptr;      ///< Следующая станция (мелко, снизу)
};

#endif // UI_SCREENS_STATION_SCREEN_H