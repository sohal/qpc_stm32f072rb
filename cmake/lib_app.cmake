# include the qpc framework here
include(${cmake-toolchains_SOURCE_DIR}/frameworks/lib_QPC.cmake)

set(app_SOURCE_DIR ${CMAKE_SOURCE_DIR}/app/qpc_adapter)
set(app_SOURCES
    ${app_SOURCE_DIR}/qpc_app.c
    ${app_SOURCE_DIR}/ao_blinky/ao_blinky.c
    ${app_SOURCE_DIR}/ao_canopen/ao_canopennodestm32.c
    ${app_SOURCE_DIR}/qk/bsp.c
)

add_library(app STATIC)

target_sources(app
    PUBLIC
    ${app_SOURCES}
)

target_include_directories(app
    PRIVATE
    "$<$<AND:$<CONFIG:Debug>,$<COMPILE_LANGUAGE:C>>:${cmsis_SOURCE_DIR}/Device/ARM/ARMCM0/Include>"
    "$<$<AND:$<CONFIG:Debug>,$<COMPILE_LANGUAGE:C>>:${cmsis_SOURCE_DIR}/CMSIS/Core/Include>"
    "$<$<AND:$<CONFIG:Debug>,$<COMPILE_LANGUAGE:C>>:${PROJECT_SOURCE_DIR}/Drivers/CMSIS/Device/ST/STM32F0xx/Include>"
    PUBLIC
    ${app_SOURCE_DIR}
    ${app_SOURCE_DIR}/ao_blinky
    ${app_SOURCE_DIR}/ao_canopennode
    $<BUILD_INTERFACE:${app_SOURCE_DIR}>
)

set(libName app)
setTargetCompileOptions(libName)

target_link_libraries(app
    qpc
    canopennode-stm32
    canopennode-od
)
