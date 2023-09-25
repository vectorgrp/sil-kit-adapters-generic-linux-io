add_executable(SilKitDemoDisplayGPIODevice
    "SilKitDemoDisplayGPIODevice.cpp"
    "${CMAKE_SOURCE_DIR}/gpio/adapter/ChipDatas.cpp"
 )

target_include_directories(SilKitDemoDisplayGPIODevice
    PUBLIC
    "${CMAKE_SOURCE_DIR}/gpio/adapter/")

target_link_libraries(SilKitDemoDisplayGPIODevice
    PRIVATE
        SilKit::SilKit
)
set_target_properties(SilKitDemoDisplayGPIODevice 
	PROPERTIES
	#ensure SilKit shared libraries can be loaded
    INSTALL_RPATH "$ORIGIN"
    BUILD_RPATH "$ORIGIN"
    RUNTIME_OUTPUT_DIRECTORY $<$<BOOL:ALWAYS>:${SILKIT_ADAPTERS_GPIO_OUTPUT_DIRECTORY}>
)

target_compile_options(SilKitDemoDisplayGPIODevice
    PRIVATE
        #-g -O0 # Compile options for valgrind
        -pedantic -Wall -Wextra
        -Wcast-align -Wformat=2
        -Wshadow -Wsign-conversion -Wsign-promo -Wstrict-overflow=5
        -Wno-unused
        -Wpacked
        -Werror -Wfatal-errors
)


 add_executable(SilKitDemoModifyGPIODevice
    "SilKitDemoModifyGPIODevice.cpp"
    "${CMAKE_SOURCE_DIR}/gpio/adapter/ChipDatas.cpp"
 )

 target_include_directories(SilKitDemoModifyGPIODevice
    PUBLIC
    "${CMAKE_SOURCE_DIR}/gpio/adapter/")

 target_link_libraries(SilKitDemoModifyGPIODevice
    PRIVATE
        SilKit::SilKit
)
set_target_properties(SilKitDemoModifyGPIODevice 
	PROPERTIES
	#ensure SilKit shared libraries can be loaded
    INSTALL_RPATH "$ORIGIN"
    BUILD_RPATH "$ORIGIN"
    RUNTIME_OUTPUT_DIRECTORY $<$<BOOL:ALWAYS>:${SILKIT_ADAPTERS_GPIO_OUTPUT_DIRECTORY}>
)

# Compile options for valgrind
target_compile_options(SilKitDemoModifyGPIODevice
    PRIVATE
        -g -O0
        -pedantic -Wall -Wextra
        -Wcast-align -Wformat=2
        -Wshadow -Wsign-conversion -Wsign-promo -Wstrict-overflow=5
        -Wno-unused
        -Wpacked
        -Werror -Wfatal-errors
)