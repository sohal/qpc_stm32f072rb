# Set necessary paths for finding HAL and OD for CANopenNode-STM32
set(CANOPEN_OBJECT_DICTIONARY_PATH "${CMAKE_SOURCE_DIR}/app/qpc_adapter/ao_canopen")
set(STM32CUBEMX_HAL_PATH ${hal_SOURCE_DIR})
# include universal CANopenNode-STM32 in the mix
include(${cmake-toolchains_SOURCE_DIR}/frameworks/lib_canopennode.cmake)
