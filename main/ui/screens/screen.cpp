#include "screen.h"
#include "drivers/nvs/nvs_manager.h"
#include "screen_manager.h"  

void Screen::handleAudioEvent(const AudioToUIMessage& msg) {
    (void)msg;
}

void Screen::onLongPress() {

    // Сохраняем все настройки перед переключением
    NVSManager::getInstance().commit();
    
    if (m_manager) {
        m_manager->next();
    }
}

void Screen::onDoublePress() {
    
    // Сохраняем все настройки перед переключением
    NVSManager::getInstance().commit();
    
    if (m_manager) {
        m_manager->next();
    }
}