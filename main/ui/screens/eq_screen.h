/**
 * @file eq_screen.h
 * @brief Экран эквалайзера
 * 
 * Позволяет регулировать три полосы эквалайзера:
 * - BASS (низкие частоты)
 * - MID (средние частоты)
 * - TREBLE (высокие частоты)
 * 
 * Управление:
 * - Поворот энкодера → изменение значения текущей полосы
 * - Короткое нажатие → переключение между полосами (BASS → MID → TREBLE → BASS)
 * - Долгое/двойное нажатие → переключение на следующий экран (унаследовано от Screen)
 * 
 * Диапазон значений: от -40 до +6 децибел.
 * Шаг изменения: 1 dB.
 */

#ifndef UI_SCREENS_EQ_SCREEN_H
#define UI_SCREENS_EQ_SCREEN_H

#include "screen_with_handlers.h"

/**
 * @class EQScreen
 * @brief Экран для настройки эквалайзера
 */
class EQScreen : public ScreenWithHandlers {
public:
    /**
     * @brief Конструктор
     * @param manager указатель на менеджер экранов
     * @param parent указатель на контейнер, в котором будет размещён экран
     */
    explicit EQScreen(ScreenManager* manager, lv_obj_t* parent);
    
    virtual ~EQScreen() = default;
    
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
     * - EVENT_EQ_VALUES: обновление значений эквалайзера
     */
    void handleAudioEvent(const AudioToUIMessage& msg) override;
    
    // ==================== События энкодера ====================
    
    /**
     * @brief Поворот вправо → увеличить значение текущей полосы
     */
    void onTurnRight() override;
    
    /**
     * @brief Поворот влево → уменьшить значение текущей полосы
     */
    void onTurnLeft() override;
    
    /**
     * @brief Короткое нажатие → переключить полосу
     */
    void onShortPress() override;
    
    // onLongPress и onDoublePress оставляем по умолчанию (переключение экрана)
    
    virtual void refresh() override;

private:
    // ==================== Типы и константы ====================
    
    /**
     * @enum Band
     * @brief Полосы эквалайзера
     */
    enum Band : uint8_t {
        BASS = 0,      ///< Низкие частоты
        MID,           ///< Средние частоты
        TREBLE,        ///< Высокие частоты
        BAND_COUNT     ///< Количество полос
    };
    
    static const char* BAND_NAMES[BAND_COUNT];   ///< Названия полос
    static const int MIN_VALUE = -40;              ///< Минимальное значение (dB)
    static const int MAX_VALUE = 6;               ///< Максимальное значение (dB)
    
    // ==================== Вспомогательные методы ====================
    
    /**
     * @brief Обновить отображение текущей полосы и её значения
     */
    void updateDisplay();
    
    /**
     * @brief Получить значение текущей полосы
     * @return текущее значение в dB
     */
    int getCurrentBandValue() const;
    
    /**
     * @brief Установить значение текущей полосы
     * @param value новое значение
     */
    void setCurrentBandValue(int value);
    
    /**
     * @brief Отправить текущие значения эквалайзера в AudioTask
     */
    void sendEQToAudio();
    
    /**
     * @brief Сохранить текущие значения в NVS
     */
    void saveToNVS();
    
    /**
     * @brief Преобразовать значение dB в процент для прогресс-бара
     * @param value значение в dB (-40..+6)
     * @return процент (0-100)
     */
    int valueToPercent(int value) const;
    
    /**
     * @brief Получить цвет для значения
     * @param value значение в dB
     * @return цвет в формате RGB565
     */
    uint32_t getValueColor(int value) const;
    
    // ==================== Данные экрана ====================
    
    Band m_currentBand = BASS;    ///< Текущая выбранная полоса
    int m_bass = 0;               ///< Значение басов
    int m_mid = 0;                ///< Значение средних
    int m_treble = 0;             ///< Значение высоких
    
    // ==================== LVGL виджеты ====================
    
    lv_obj_t* m_modeLabel = nullptr;      ///< Индикатор "EQ" (голубой)
    lv_obj_t* m_bandLabel = nullptr;      ///< Название текущей полосы (BASS/MID/TREBLE)
    lv_obj_t* m_valueLabel = nullptr;     ///< Текущее значение (например "+3 dB")
    lv_obj_t* m_bar = nullptr;            ///< Прогресс-бар для визуализации
    lv_obj_t* m_minLabel = nullptr;       ///< Метка минимального значения "-40"
    lv_obj_t* m_maxLabel = nullptr;       ///< Метка максимального значения "+6"
    lv_obj_t* m_zeroLabel = nullptr;      ///< Метка нуля "0"
};

#endif // UI_SCREENS_EQ_SCREEN_H