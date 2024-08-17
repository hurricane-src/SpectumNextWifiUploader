cmake_minimum_required(VERSION 3.18)
project(SpectrumNextWifiUploader)
enable_language(ASM)
set(CMAKE_SYSTEM_NAME "SpectrumNext")
set(CMAKE_ZXASM_COMPILER "/usr/local/bin/sjasmplus" CACHE STRING "Path to the sjasmplus compiler")
set(CMAKE_ZXASM_FLAGS --zxnext
    --outprefix=${CMAKE_BINARY_DIR}
    --sld=${CMAKE_BINARY_DIR}/wifiupld.sld
    --sym=${CMAKE_BINARY_DIR}/wifiupld.labels
    --lst=${CMAKE_BINARY_DIR}/wifiupld.lst
    --inc=${CMAKE_SOURCE_DIR}
    --outprefix="${CMAKE_CURRENT_BINARY_DIR}/"
    --fullpath)
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(SOURCES
    main.asm
    esp.inc
    esp-state-machine.inc
    font.inc
    macros.inc
    next_board_feature_control.inc
    tools.inc
    uart.inc
    ula_text.inc)
add_custom_target(server ALL
                    COMMAND ${CMAKE_ZXASM_COMPILER} ${CMAKE_ZXASM_FLAGS} ${CMAKE_CURRENT_SOURCE_DIR}/main.asm
                    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
                    SOURCES ${SOURCES}
                    )
