/**
 * @file volume_screen.h
 * @brief Экран управления громкостью и воспроизведением
 * 
 * Отображает:
 * - Название текущей станции
 * - Название трека (из RDS/ICY метаданных)
 * - Прогресс-бар громкости
 * - Текст громкости
 * - Статус воспроизведения (Playing/Paused)
 * 
 * Управление:
 * - Поворот энкодера → изменение громкости
 * - Короткое нажатие → Play/Pause
 * - Долгое/двойное нажатие → переключение на следующий экран (унаследовано от Screen)
 */

#ifndef UI_SCREENS_VOLUME_SCREEN_H
#define UI_SCREENS_VOLUME_SCREEN_H

#include "screen_with_handlers.h"

/**
 * @class VolumeScreen
 * @brief Экран для отображения и управления воспроизведением
 */
class VolumeScreen : public ScreenWithHandlers {
public:
    /**
     * @brief Конструктор
     * @param manager указатель на менеджер экранов
     * @param container указатель на контейнер, в котором будет размещён экран
     */
    explicit VolumeScreen(ScreenManager* manager, lv_obj_t* parent);
    
    virtual ~VolumeScreen() = default;
    
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
     * - EVENT_PLAYBACK_INFO: обновление станции, трека, статуса
     * - EVENT_VOLUME_CHANGED: обновление громкости
     * - EVENT_WIFI_STATUS: обновление статуса WiFi (опционально)
     */
    void handleAudioEvent(const AudioToUIMessage& msg) override;
    
    // ==================== События энкодера ====================
    
    /**
     * @brief Поворот вправо → увеличение громкости
     */
    void onTurnRight(int enc_no) override;
    
    /**
     * @brief Поворот влево → уменьшение громкости
     */
    void onTurnLeft(int enc_no) override;
    
    /**
     * @brief Короткое нажатие → Play/Pause
     */
    void onShortPress(int enc_no) override;
    
    // onLongPress и onDoublePress оставляем по умолчанию (переключение экрана)

    virtual void refresh() override;

private:
    // ==================== Вспомогательные методы ====================
    
    /**
     * @brief Обновить отображение громкости
     * @param volume новое значение громкости (0-21)
     */
    void updateVolumeDisplay(int volume);
    
    /**
     * @brief Обновить отображение названия трека
     * @param song название трека
     */
    void updateSongDisplay(const char* song);
    
    // ==================== LVGL виджеты ====================
    
    lv_obj_t* m_modeLabel = nullptr;      ///< Индикатор "VOLUME" (зелёный)
    lv_obj_t* m_stationLabel = nullptr;   ///< Название радиостанции (жёлтый, крупный)
    lv_obj_t* m_songLabel = nullptr;      ///< Название трека (белый, средний)
    lv_obj_t* m_volumeBar = nullptr;      ///< Прогресс-бар громкости
    lv_obj_t* m_volumeLabel = nullptr;    ///< Текст громкости (например "VOLUME: 15/21")
    lv_obj_t* m_statusLabel = nullptr;    ///< Статус воспроизведения (PLAYING/PAUSED)
};

#endif // UI_SCREENS_VOLUME_SCREEN_H