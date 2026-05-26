idf_component_register(
    SRCS 
        main.cpp
        config.cpp
        my_font_16.c
        tasks/audio_task.cpp
        tasks/ui_task.cpp
        drivers/encoder/encoder.cpp
        drivers/audio/audio_manager.cpp
        drivers/display/display_driver.cpp
        drivers/nvs/nvs_manager.cpp
    INCLUDE_DIRS 
        "."
        "tasks"
        "drivers/encoder"
        "drivers/audio"
        "drivers/display"
        "drivers/nvs"
    REQUIRES 
        Arduino 
        mbedtls 
        esp-tls 
        esp-dsp 
        wear_levelling 
        freertos
        esp_lcd
        driver 
        lvgl
        nvs_flash
)

target_include_directories(${COMPONENT_TARGET} PRIVATE
    ../components/ESP32-audioI2S/src

)

target_compile_options(${COMPONENT_TARGET} PRIVATE 
    -Wno-format 
    -Wno-error=format 
    -Wno-unused-variable
    -Wno-reorder
    -fpermissive
)