cmake_minimum_required(VERSION 3.12...3.18)
set(CMAKE_CXX_STANDARD 11)

project(electrometer-edge)

find_package(Threads REQUIRED)
find_package(OpenSSL REQUIRED)
find_package(eclipse-paho-mqtt-c REQUIRED)
find_package(PahoMqttCpp REQUIRED)

include_directories(${PAHO-MQTT-C_INCLUDE_DIRS})

add_executable(async_pub ./temp/async_publish.cpp)
target_link_libraries(async_pub
    PahoMqttCpp::paho-mqttpp3
)

