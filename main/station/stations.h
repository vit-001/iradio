/**
 * @file stations.h
 * @brief Класс для управления списком радиостанций
 */

#ifndef STATIONS_H
#define STATIONS_H

#include <cstring>

// ==================== Статические данные станций ====================
static const char* STATION_NAMES[] = {
    "DFM",
    "Europa Plus", 
    "Hit FM",
    "Relax FM",
    "Energy FM",
    "Maximum",
    "Like FM",
    "Radio Record",
    "Наше радио",
    "Monte Carlo",
    "Love Radio",
    "Comedy Radio",
    "Rock FM",
    "Studio 21",
    "Piter FM",
    "Серебряный дождь",
    "Русское радио"
};

static const char* STATION_URLS[] = {
    "http://dfm.hostingradio.ru/dfm128.mp3",
    "http://ep128.streamr.ru",
    "http://hitfm.hostingradio.ru/hitfm128.mp3",
    "https://pub0201.101.ru/stream/trust/mp3/128/24",
    "http://23.105.238.4/gpm-energyfm495.aacp",
    "https://maximum.hostingradio.ru/maximum128.mp3",
    "http://23.105.238.4/gpm-likefm495.aacp",
    "https://radiorecord.hostingradio.ru/rr_spb",
    "http://nashe1.hostingradio.ru/nashespb128.mp3",
    "http://montecarloreg.hostingradio.ru/spb.montecarlo160.aacp",
    "http://microit.n340.com:9000/VgMv0WV17ZVx1uuo_12_love_128_reg_45",
    "http://23.105.238.4/gpm-comedyradio495.aacp",
    "http://nashe1.hostingradio.ru/rock-256",
    "http://78.140.197.238:9877/SPB_studio21",
    "http://78.140.208.20:8801/pfm_spb",
    "http://s0.radioheart.ru:8000/ird",
    "http://23.105.238.4/spb.rusradio128.mp3"
};

#define STATIONS_COUNT (sizeof(STATION_NAMES) / sizeof(STATION_NAMES[0]))

/**
 * @class StationsManager
 * @brief Синглтон для управления списком радиостанций
 */
class StationsManager {
public:
    /**
     * @brief Получить единственный экземпляр менеджера
     */
    static StationsManager& getInstance();

    /**
     * @brief Получить название станции по индексу
     * @param index индекс станции (0..getCount()-1)
     * @return указатель на строку с названием, или nullptr если индекс неверный
     */
    const char* getName(int index) const;

    /**
     * @brief Получить URL станции по индексу
     * @param index индекс станции (0..getCount()-1)
     * @return указатель на строку с URL, или nullptr если индекс неверный
     */
    const char* getUrl(int index) const;

    /**
     * @brief Найти индекс станции по URL
     * @param url URL станции
     * @return индекс станции (0..getCount()-1), или -1 если не найдено
     */
    int findIndexByUrl(const char* url) const;

    /**
     * @brief Найти индекс станции по названию
     * @param name название станции
     * @return индекс станции (0..getCount()-1), или -1 если не найдено
     */
    int findIndexByName(const char* name) const;

    /**
     * @brief Найти индекс станции по частичному совпадению названия
     * @param name часть названия
     * @return индекс станции, или -1 если не найдено
     */
    int findIndexByNamePartial(const char* name) const;

    /**
     * @brief Получить количество станций в списке
     */
    int getCount() const;

    /**
     * @brief Проверить, валидный ли индекс
     */
    bool isValidIndex(int index) const;

    // Для будущего расширения
    // bool addStation(const char* name, const char* url);
    // bool removeStation(int index);
    // void saveToNVS();
    // void loadFromNVS();

private:
    // Приватный конструктор для синглтона
    StationsManager() = default;
    
    // Запрещаем копирование
    StationsManager(const StationsManager&) = delete;
    StationsManager& operator=(const StationsManager&) = delete;
};

#endif // STATIONS_H