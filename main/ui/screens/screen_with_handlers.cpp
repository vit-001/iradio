#include "screen_with_handlers.h"
#include "drivers/nvs/nvs_manager.h"
#include "screen_manager.h"  

void ScreenWithHandlers::onLongPress()  {

    // Сохраняем все настройки перед переключением
    NVSManager::getInstance().commit();
    
    if (m_manager) {
        m_manager->next();
    }
}

void ScreenWithHandlers::onDoublePress()  {
    
    // Сохраняем все настройки перед переключением
    NVSManager::getInstance().commit();
    
    if (m_manager) {
        m_manager->next();
    }
}