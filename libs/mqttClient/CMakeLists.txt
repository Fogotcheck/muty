cmake_minimum_required(VERSION 3.5)

set(WifiKernelSRC "${CMAKE_CURRENT_SOURCE_DIR}/src/WiFiKernel.c")
set(MqttClientSRC "${CMAKE_CURRENT_SOURCE_DIR}/src/MqttClient.c")
set(WifiKernelHead "${CMAKE_CURRENT_SOURCE_DIR}/head")
set(WifiKernlConfig "${CMAKE_CURRENT_SOURCE_DIR}/config")

add_library(ClientKernel "${WifiKernelSRC}"
    "${MqttClientSRC}")

target_include_directories(ClientKernel PUBLIC
    "${WifiKernelHead}"
    "${WifiKernlConfig}")

target_link_libraries(ClientKernel PRIVATE ${COMPONENT_LIB})
target_link_libraries(ClientKernel INTERFACE VirtualRegister)