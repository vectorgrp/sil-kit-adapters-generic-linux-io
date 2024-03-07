# Copyright (c) Vector Informatik GmbH. All rights reserved.

add_executable(SilKitAdapterGenericLinuxIO
    "SilKitAdapterGenericLinuxIO.cpp"

    "../chardev/adapter/ChardevManager.cpp"
    "../chardev/adapter/ChardevAdapter.cpp"

    "../advalues/adapter/AdManager.cpp"
    "../advalues/adapter/AdAdapter.cpp"

    "../gpio/adapter/GpioManager.cpp"
    "../gpio/adapter/GpioAdapter.cpp"
    "../gpio/adapter/GpioWrapper.cpp"

    "../util/FileHelper.cpp"
    "../util/YamlHelper.cpp"
    "../util/Parsing.cpp"
    "../util/SignalHandler.cpp"
)    

target_include_directories(SilKitAdapterGenericLinuxIO 
    PUBLIC 
        ${CMAKE_CURRENT_SOURCE_DIR}
        "${CMAKE_SOURCE_DIR}"
)

target_link_libraries(SilKitAdapterGenericLinuxIO
    PRIVATE
        SilKit::SilKit
        yaml-cpp
        asio
)

set_target_properties(SilKitAdapterGenericLinuxIO 
	PROPERTIES
    #ensure SilKit shared libraries can be loaded
    INSTALL_RPATH "$ORIGIN/../lib:$ORIGIN"
    BUILD_RPATH "$ORIGIN"
    RUNTIME_OUTPUT_DIRECTORY $<$<BOOL:ALWAYS>:${SILKIT_ADAPTERS_GENERIC_LINUX_IO_OUTPUT_DIRECTORY}>
)

target_compile_options(SilKitAdapterGenericLinuxIO
    PRIVATE ${compile_options}
)

add_custom_command(
    TARGET SilKitAdapterGenericLinuxIO
    POST_BUILD
    COMMAND ${CMAKE_COMMAND}
    ARGS -E copy $<TARGET_FILE:SilKit::SilKit> ${SILKIT_ADAPTERS_GENERIC_LINUX_IO_LIBRARY_DIRECTORY}/libSilKit.so
)