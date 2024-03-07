
add_executable(SilKitDemoGLIOAdvaluesForwardDevice
    "SilKitDemoGLIOAdvaluesForwardDevice.cpp"

    "${CMAKE_SOURCE_DIR}/util/Parsing.cpp"
    "${CMAKE_SOURCE_DIR}/util/SignalHandler.cpp"
)
target_link_libraries(SilKitDemoGLIOAdvaluesForwardDevice
    PRIVATE
        SilKit::SilKit
)

target_compile_options(SilKitDemoGLIOAdvaluesForwardDevice
    PRIVATE  ${compile_options}
)

set_target_properties(SilKitDemoGLIOAdvaluesForwardDevice 
    PROPERTIES
    #ensure SilKit shared libraries can be loaded
    INSTALL_RPATH "$ORIGIN/../lib:$ORIGIN"
    BUILD_RPATH "$ORIGIN"
    RUNTIME_OUTPUT_DIRECTORY $<$<BOOL:ALWAYS>:${SILKIT_ADAPTERS_GENERIC_LINUX_IO_OUTPUT_DIRECTORY}>
)