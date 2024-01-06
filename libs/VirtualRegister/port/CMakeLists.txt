cmake_minimum_required(VERSION 3.15)

add_library(VirtualPort "${CMAKE_CURRENT_SOURCE_DIR}/VRPort.c")
target_link_libraries(VirtualPort INTERFACE VirtualRegister)

target_include_directories(VirtualPort PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}" "${VisiblePublicHead}")
