cmake_minimum_required(VERSION 3.15)
set(D74HC595_SRC "${CMAKE_CURRENT_SOURCE_DIR}/src/D74HC595Kernel.c")
set(D74HC595_Head "${CMAKE_CURRENT_SOURCE_DIR}/head")
set(D74HC595_Conf "${CMAKE_CURRENT_SOURCE_DIR}/conf")

add_library(D74HC595Lib "${D74HC595_SRC}")

target_include_directories(D74HC595Lib PUBLIC
    "${D74HC595_Head}"
    "${D74HC595_Conf}"
)

target_link_libraries(D74HC595Lib PRIVATE ${COMPONENT_LIB})