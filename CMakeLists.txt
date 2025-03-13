# The following five lines of boilerplate have to be in your project's
# CMakeLists in this exact order for cmake to work correctly
cmake_minimum_required(VERSION 3.16)

include($ENV{IDF_PATH}/tools/cmake/project.cmake)

# register local components
idf_build_component("components/esp32-DHT11")
idf_build_component("components/esp32-telegram-bot-api-interface")

project(esp32-telegram-temperature-bot)
