#include "Arduino.h"
#include "Audio.h"
#include "config.h"

Audio audio;

// Переменные энкодера
int volume = 10;
unsigned long lastTurnTime = 0;
const int debounceDelay = DEBOUNCE_DELAY;

void setVolume(int vol) {
    if (vol < 0) vol = 0;
    if (vol > 21) vol = 21;
    volume = vol;
    audio.setVolume(volume);
    Serial.print("Volume: ");
    Serial.println(volume);
}

void setup() {
    Serial.begin(115200);
    Serial.println("\nStarting Internet Radio...");
    
    // Настройка пинов энкодера
    pinMode(ENC_A, INPUT_PULLUP);
    pinMode(ENC_B, INPUT_PULLUP);
    pinMode(ENC_BTN, INPUT_PULLUP);
  
    // Подключение к WiFi
    WiFi.begin(ssid, password);
    Serial.print("Connecting to WiFi");
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("\nWiFi connected!");
    Serial.print("IP: ");
    Serial.println(WiFi.localIP());

    // Настройка I2S для PCM5102A
    audio.setPinout(I2S_BCLK, I2S_LRC, I2S_DOUT);
    audio.setTone(TONE_BASS, TONE_MID, TONE_TREBLE);
    setVolume(volume);  // Устанавливаем начальную громкость

    // Запуск воспроизведения
    audio.connecttohost(radio_url);
    Serial.println("Playing DFM Radio...");
}

void loop() {
    // Энкодер с антидребезгом
    static int lastA = HIGH;
    int a = digitalRead(ENC_A);
    int b = digitalRead(ENC_B);
    
    if (a != lastA && a == LOW && (millis() - lastTurnTime) > debounceDelay) {
        lastTurnTime = millis();
        
        if (b == HIGH) {
            setVolume(volume - 1);  // Влево - уменьшение
            Serial.println("<- Volume Down");
        } else {
            setVolume(volume + 1);  // Вправо - увеличение
            Serial.println("Volume Up ->");
        }
    }
    lastA = a;

    // Кнопка: Play/Pause с защитой от дребезга
    static unsigned long lastButtonTime = 0;
    if (digitalRead(ENC_BTN) == LOW && (millis() - lastButtonTime) > 200) {
        lastButtonTime = millis();
        
        if (audio.isRunning()) {
            audio.stopSong();
            Serial.println("⏸ Paused");
        } else {
            audio.connecttohost(radio_url);
            Serial.println("▶ Resumed");
        }
        while (digitalRead(ENC_BTN) == LOW) delay(10);
    }

    audio.loop();
    delay(5);
}

// Callback для информации о потоке
void audio_info(const char* info) {
    Serial.printf("Info: %s\n", info);
}

// Callback для метаданных (название трека)
void audio_id3data(const char* info) {
    Serial.printf("Now playing: %s\n", info);
}

// Callback для ошибок
void audio_eof_mp3(const char* info) {
    Serial.printf("End of stream: %s\n", info);
}