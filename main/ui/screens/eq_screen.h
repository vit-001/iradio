/**
 * @file eq_screen.h
 * @brief Экран эквалайзера
 *
 * Позволяет регулировать три полосы эквалайзера:
 * - BASS
 * - MID
 * - TREBLE
 *
 * Управление:
 * - Поворот энкодера → изменение значения текущей полосы
 * - Короткое нажатие → переключение полосы
 *
 * Долгое и двойное нажатия обрабатываются базовым классом.
 */

#pragma once

#include "screen_with_handlers.h"

/**
 * @class EQScreen
 * @brief Экран настройки эквалайзера
 */
class EQScreen : public ScreenWithHandlers
{
public:
    /**
     * @brief Конструктор
     * @param manager указатель на менеджер экранов
     * @param parent контейнер LVGL
     */
    explicit EQScreen(ScreenManager* manager, lv_obj_t* parent);

    virtual ~EQScreen() = default;

    // ==================== Жизненный цикл ====================

    /**
     * @brief Создание LVGL объектов
     */
    lv_obj_t* create() override;

    /**
     * @brief Обновление экрана
     */
    void refresh() override;

    // ==================== События AudioTask ====================

    /**
     * @brief Обработка сообщений от AudioTask
     */
    void handleAudioEvent(const AudioToUIMessage& msg) override;

    // ==================== События энкодеров ====================

    /**
     * @brief Обработка событий энкодеров
     */
    void handleEncoderEvent(const EncoderEvent& event) override;

private:

    // ==================== Типы и константы ====================

    enum Band : uint8_t
    {
        BASS = 0,
        MID,
        TREBLE,
        BAND_COUNT
    };

    static const char* BAND_NAMES[BAND_COUNT];

    static constexpr int MIN_VALUE = -40;
    static constexpr int MAX_VALUE = 6;

    // ==================== Вспомогательные методы ====================

    /**
     * @brief Обновить отображение
     */
    void updateDisplay();

    /**
     * @brief Получить значение текущей полосы
     */
    int getCurrentBandValue() const;

    /**
     * @brief Установить значение текущей полосы
     */
    void setCurrentBandValue(int value);

    /**
     * @brief Отправить настройки в AudioTask
     */
    void sendEQToAudio();

    /**
     * @brief Сохранить настройки в NVS
     */
    void saveToNVS();

    /**
     * @brief Преобразование dB в проценты для progress bar
     */
    int valueToPercent(int value) const;

    /**
     * @brief Получить цвет значения
     */
    uint32_t getValueColor(int value) const;

    // ==================== Данные ====================

    Band m_currentBand = BASS;

    int m_bass = 0;
    int m_mid = 0;
    int m_treble = 0;

    // ==================== LVGL объекты ====================

    lv_obj_t* m_modeLabel = nullptr;   ///< "EQ"
    lv_obj_t* m_bandLabel = nullptr;   ///< BASS/MID/TREBLE
    lv_obj_t* m_valueLabel = nullptr;  ///< "+3 dB"
    lv_obj_t* m_bar = nullptr;

    lv_obj_t* m_minLabel = nullptr;
    lv_obj_t* m_maxLabel = nullptr;
    lv_obj_t* m_zeroLabel = nullptr;
};