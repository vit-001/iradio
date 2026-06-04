#include "stations.h"

StationsManager& StationsManager::getInstance() {
    static StationsManager instance;
    return instance;
}

const char* StationsManager::getName(int index) const {
    if (!isValidIndex(index)) return nullptr;
    return STATION_NAMES[index];
}

const char* StationsManager::getUrl(int index) const {
    if (!isValidIndex(index)) return nullptr;
    return STATION_URLS[index];
}

int StationsManager::findIndexByUrl(const char* url) const {
    if (url == nullptr) return -1;
    
    for (int i = 0; i < STATIONS_COUNT; i++) {
        if (strcmp(STATION_URLS[i], url) == 0) {
            return i;
        }
    }
    return -1;
}

int StationsManager::findIndexByName(const char* name) const {
    if (name == nullptr) return -1;
    
    for (int i = 0; i < STATIONS_COUNT; i++) {
        if (strcmp(STATION_NAMES[i], name) == 0) {
            return i;
        }
    }
    return -1;
}

int StationsManager::findIndexByNamePartial(const char* name) const {
    if (name == nullptr) return -1;
    
    for (int i = 0; i < STATIONS_COUNT; i++) {
        if (strstr(STATION_NAMES[i], name) != nullptr) {
            return i;
        }
    }
    return -1;
}

int StationsManager::getCount() const {
    return STATIONS_COUNT;
}

bool StationsManager::isValidIndex(int index) const {
    return (index >= 0 && index < STATIONS_COUNT);
}