add_executable(blinky)

target_sources(blinky
    PRIVATE
    ${hal_SOURCE_DIR}/demo/__startup/__vectors.c
)

target_link_libraries(blinky
    app
)

target_include_directories(blinky
    PUBLIC
    ${cmsis_DEVICE_INCLUDE_DIR}
)

set(targetName blinky)

set(${targetName}_LINKER_PATH "${hal_SOURCE_DIR}/demo/__linker")
set(${targetName}_LINKER_SCRIPT "gcc_arm.ld")
set(${targetName}_SCATTER_PATH "${hal_SOURCE_DIR}/demo/__linker")
set(${targetName}_SCATTER_FILE "ac6_arm.sct")
set(${targetName}_LLVM_LINKER_PATH "${hal_SOURCE_DIR}/demo/__linker")
set(${targetName}_LLVM_LINKER_SCRIPT "llvm_arm.ld")
setTargetCompileOptions(targetName)
setTargetLinkOptions(targetName)

convertELF_BIN_HEX (${targetName})
