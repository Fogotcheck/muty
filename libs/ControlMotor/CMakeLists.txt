cmake_minimum_required(VERSION 3.5)

set(CMotorSRC "${CMAKE_CURRENT_SOURCE_DIR}/src/CMotor.c")
set(CMotorHead "${CMAKE_CURRENT_SOURCE_DIR}/head")
set(CMotorConfig "${CMAKE_CURRENT_SOURCE_DIR}/config")

add_library(CMotorKernel ${CMotorSRC})

target_include_directories(CMotorKernel PUBLIC
    "${CMotorHead}"
    "${CMotorConfig}")

target_link_libraries(CMotorKernel PRIVATE ${COMPONENT_LIB})