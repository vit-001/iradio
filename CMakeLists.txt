cmake_minimum_required(VERSION 3.16)

set(EXTRA_COMPONENT_DIRS 
    components/Arduino
    components/ESP32-audioI2S
)

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(radio)