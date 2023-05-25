# Set necessary paths for finding HAL and OD for CANopenNode-STM32
set(CANOPEN_OBJECT_DICTIONARY_PATH "${CMAKE_SOURCE_DIR}/app/qpc_adapter/ao_canopen")

# include universal CANopenNode-STM32 in the mix
include(${cmake-toolchains_SOURCE_DIR}/frameworks/lib_canopennode.cmake)
