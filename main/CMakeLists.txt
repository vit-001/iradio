idf_component_register(
    SRCS "main.cpp"
    INCLUDE_DIRS "."
    REQUIRES Arduino mbedtls esp-tls esp-dsp wear_levelling 
)

target_include_directories(${COMPONENT_TARGET} PRIVATE
    ../components/ESP32-audioI2S/src
)

target_compile_options(${COMPONENT_TARGET} PRIVATE 
    -Wno-format 
    -Wno-error=format 
    -Wno-unused-variable
)