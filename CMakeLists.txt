
cmake_minimum_required(VERSION 3.20)

set(DEVKITPRO "$ENV{DEVKITPRO}")
if(NOT DEVKITPRO)
    set(DEVKITPRO "/opt/devkitpro")
endif()

set(DEVKITARM "${DEVKITPRO}/devkitARM")

set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_C_COMPILER "${DEVKITARM}/bin/arm-none-eabi-gcc")
set(CMAKE_CXX_COMPILER "${DEVKITARM}/bin/arm-none-eabi-g++")
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

project(DeltaruneGBA LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(DeltaruneGBA
        src/main.cpp
)

set_target_properties(DeltaruneGBA PROPERTIES
        SUFFIX ".elf"
)

target_include_directories(DeltaruneGBA PRIVATE
        "${DEVKITPRO}/libgba/include"
)

target_compile_options(DeltaruneGBA PRIVATE
        -mcpu=arm7tdmi
        -mthumb
        -mthumb-interwork
        -O2
        -ffunction-sections
        -fdata-sections
)

target_link_options(DeltaruneGBA PRIVATE
        -specs=gba.specs
        -mthumb
        -mthumb-interwork
        -Wl,--gc-sections
)

target_link_libraries(DeltaruneGBA PRIVATE
        "${DEVKITPRO}/libgba/lib/libgba.a"
)

add_custom_command(TARGET DeltaruneGBA POST_BUILD
        COMMAND "${DEVKITARM}/bin/arm-none-eabi-objcopy"
        -O binary
        "$<TARGET_FILE:DeltaruneGBA>"
        "${CMAKE_CURRENT_BINARY_DIR}/DeltaruneGBA.gba"
        COMMAND "${DEVKITPRO}/tools/bin/gbafix"
        "${CMAKE_CURRENT_BINARY_DIR}/DeltaruneGBA.gba"
        -tDeltaruneGBA
        -cDLTR
        -m00
        VERBATIM
)
