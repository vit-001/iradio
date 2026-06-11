/**
 * @file volume_screen.h
 * @brief Экран управления громкостью и воспроизведением
 *
 * Отображает:
 * - Название текущей станции
 * - Название трека
 * - Прогресс-бар громкости
 * - Значение громкости
 * - Статус воспроизведения
 *
 * Управление:
 * - Энкодер №1:
 *      поворот → громкость
 *      короткое нажатие → Play/Pause
 *
 * - Остальные энкодеры пока игнорируются
 */

#pragma once

#include "screen_with_handlers.h"
#include "drivers/encoder/encoder.h"

/**
 * @class VolumeScreen
 * @brief Экран управления воспроизведением
 */
class VolumeScreen : public ScreenWithHandlers
{
public:
    /**
     * @brief Конструктор
     * @param manager менеджер экранов
     * @param parent контейнер LVGL
     */
    explicit VolumeScreen(ScreenManager* manager, lv_obj_t* parent);

    virtual ~VolumeScreen() = default;

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
     * @brief Обработка события энкодера
     */
    void handleEncoderEvent(const EncoderEvent& event) override;

private:

    // ==================== Вспомогательные методы ====================

    /**
     * @brief Обновить отображение громкости
     */
    void updateVolumeDisplay(int volume);

    /**
     * @brief Обновить отображение названия трека
     */
    void updateSongDisplay(const char* song);

    // ==================== LVGL объекты ====================

    lv_obj_t* m_modeLabel = nullptr;       ///< "VOLUME"
    lv_obj_t* m_stationLabel = nullptr;    ///< Название станции
    lv_obj_t* m_songLabel = nullptr;       ///< Название трека
    lv_obj_t* m_volumeBar = nullptr;       ///< Полоса громкости
    lv_obj_t* m_volumeLabel = nullptr;     ///< "VOL: 10"
    lv_obj_t* m_statusLabel = nullptr;     ///< PLAYING / PAUSED
};