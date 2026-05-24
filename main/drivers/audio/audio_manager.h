#ifndef AUDIO_MANAGER_H
#define AUDIO_MANAGER_H

#include "Audio.h"

class AudioManager {
public:
    static AudioManager& getInstance();
    
    void begin(int bckPin, int lrcPin, int doutPin);
    void loop();
    void setVolume(int vol);
    int getVolume() const { return _volume; }
    void volumeUp();
    void volumeDown();
    void play();
    void pause();
    void playPause();
    bool isPlaying() const { return _isPlaying; }
    void setTone(int8_t bass, int8_t mid, int8_t treble);
    void setDefaultVolume(int vol);
    void connectToStream(const char* url);
    
private:
    AudioManager();
    ~AudioManager();
    AudioManager(const AudioManager&) = delete;
    AudioManager& operator=(const AudioManager&) = delete;
    
    Audio _audio;
    int _volume;
    bool _isPlaying;
    const char* _currentUrl;
};

#endif // AUDIO_MANAGER_H