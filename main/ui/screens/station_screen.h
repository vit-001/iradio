/**
 * @file station_screen.h
 * @brief Экран выбора радиостанции
 *
 * Отображает список доступных радиостанций.
 *
 * Управление:
 * - Поворот энкодера → переход по списку станций
 * - Короткое нажатие → выбор станции
 *
 * Долгое и двойное нажатия обрабатываются базовым классом.
 */

#pragma once

#include "screen_with_handlers.h"
#include "drivers/encoder/encoder.h"

/**
 * @class StationScreen
 * @brief Экран выбора радиостанции
 */
class StationScreen : public ScreenWithHandlers
{
public:
    /**
     * @brief Конструктор
     * @param manager указатель на ScreenManager
     * @param parent контейнер LVGL
     */
    explicit StationScreen(ScreenManager* manager, lv_obj_t* parent);

    virtual ~StationScreen() = default;

    // ==================== Жизненный цикл ====================

    /**
     * @brief Создание LVGL объектов экрана
     */
    lv_obj_t* create() override;

    /**
     * @brief Обновление экрана
     */
    void refresh() override;

    // ==================== События от AudioTask ====================

    /**
     * @brief Обработка сообщений от AudioTask
     */
    void handleAudioEvent(const AudioToUIMessage& msg) override;

    // ==================== События энкодеров ====================

    /**
     * @brief Обработка событий энкодеров
     */
    void handleEncoderEvent(const EncoderEvent& event) override;

    // ==================== Дополнительные методы ====================

    /**
     * @brief Установить текущую станцию
     * @param index индекс станции
     */
    void setCurrentStation(int index);

    /**
     * @brief Получить индекс текущей станции
     */
    int getCurrentStation() const
    {
        return m_currentStationIndex;
    }

private:

    // ==================== Вспомогательные методы ====================

    /**
     * @brief Обновить отображение станции
     */
    void updateStationDisplay();

    /**
     * @brief Отправить команду AudioTask на переключение станции
     * @param index индекс станции
     */
    void switchToStation(int index);

    // ==================== Данные ====================

    int m_currentStationIndex = 0;   ///< Текущая выбранная станция

    // ==================== LVGL объекты ====================

    lv_obj_t* m_modeLabel = nullptr;         ///< "STATION"
    lv_obj_t* m_stationNameLabel = nullptr;  ///< Текущая станция
    lv_obj_t* m_previousLabel = nullptr;     ///< Предыдущая станция
    lv_obj_t* m_nextLabel = nullptr;         ///< Следующая станция
};