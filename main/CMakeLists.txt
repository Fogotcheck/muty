idf_component_register(SRCS "main.c" INCLUDE_DIRS ".")

add_subdirectory("${LIBS_PATH}/VirtualRegister" VirtualRegister)
add_subdirectory("${LIBS_PATH}/mqttClient" ClientKernel)
add_subdirectory("${LIBS_PATH}/ControlMotor" ControlMotor)
add_subdirectory("${LIBS_PATH}/Driver74CH595" D74HC595Lib)

target_link_libraries(${COMPONENT_TARGET} VirtualRegister)
target_link_libraries(${COMPONENT_TARGET} ClientKernel)
target_link_libraries(${COMPONENT_TARGET} CMotorKernel)
target_link_libraries(${COMPONENT_TARGET} D74HC595Lib)

