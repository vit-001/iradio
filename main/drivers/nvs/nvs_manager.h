#pragma once

#include "nvs_flash.h"
#include "nvs.h"

class NVSManager {
public:
    static NVSManager& getInstance();
    
    bool init();
    
    // Загрузка настроек
    int loadVolume(int defaultValue = 10);
    int loadStation(int defaultValue = 0);
    int loadBass(int defaultValue = 0);
    int loadMid(int defaultValue = 0);
    int loadTreble(int defaultValue = 0);
    
    // Установка значений (без сохранения)
    void setVolume(int volume);
    void setStation(int station);
    void setBass(int bass);
    void setMid(int mid);
    void setTreble(int treble);
    
    // Получение текущих значений
    int getVolume() const { return m_volume; }
    int getStation() const { return m_station; }
    int getBass() const { return m_bass; }
    int getMid() const { return m_mid; }
    int getTreble() const { return m_treble; }
    
    // Сохранение всех настроек во flash
    void commit();
    
    // Отложенное сохранение (вызывать в цикле)
    void processDelayedSave();
    
    // Проверка, есть ли изменения для сохранения
    bool hasPendingChanges() const { return m_pendingSave; }

private:
    NVSManager();
    ~NVSManager();
    NVSManager(const NVSManager&) = delete;
    NVSManager& operator=(const NVSManager&) = delete;
    
    nvs_handle_t m_handle;
    bool m_initialized;
    
    // Кэшированные значения настроек
    int m_volume;
    int m_station;
    int m_bass;
    int m_mid;
    int m_treble;
    
    // Флаг отложенного сохранения
    bool m_pendingSave;
    unsigned long m_saveTimer;
    
    void saveAllToNVS();
    void loadAllFromNVS();
};

