# Copyright (c) Vector Informatik GmbH. All rights reserved.

add_executable(SilKitAdapterGPIO
    "SilKitAdapterGPIO.cpp"
    "GpioChip.cpp" 
    "ChipDatas.cpp")

add_library(gpiodcxx STATIC IMPORTED)

set_target_properties(gpiodcxx PROPERTIES
  IMPORTED_LOCATION "${CMAKE_SOURCE_DIR}/third_party/gpiod/lib/libgpiodcxx.so"
  INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_SOURCE_DIR}/third_party/gpiod/include/"
)

target_include_directories(SilKitAdapterGPIO PUBLIC 
    ${CMAKE_CURRENT_SOURCE_DIR} 
    "${CMAKE_SOURCE_DIR}/third_party/gpiod/include")

target_link_libraries(SilKitAdapterGPIO
    PRIVATE
        gpiodcxx
        SilKit::SilKit
)

set_target_properties(SilKitAdapterGPIO 
	PROPERTIES
    #ensure SilKit shared libraries can be loaded
    INSTALL_RPATH "$ORIGIN"
    BUILD_RPATH "$ORIGIN"
    RUNTIME_OUTPUT_DIRECTORY $<$<BOOL:ALWAYS>:${SILKIT_ADAPTERS_GPIO_OUTPUT_DIRECTORY}>
)

target_compile_options(SilKitAdapterGPIO
    PRIVATE
        #-g -O0 # Compile options for valgrind
        -pedantic -Wall -Wextra
        -Wcast-align -Wformat=2
        -Wshadow -Wsign-conversion -Wsign-promo -Wstrict-overflow=5
        -Wno-unused
        -Wpacked
        -Werror -Wfatal-errors
)

add_custom_command(
    TARGET SilKitAdapterGPIO
    POST_BUILD
    COMMAND ${CMAKE_COMMAND}
    ARGS -E copy $<TARGET_FILE:SilKit::SilKit> ${SILKIT_ADAPTERS_GPIO_OUTPUT_DIRECTORY}
)