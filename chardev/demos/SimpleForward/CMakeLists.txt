
add_executable(sil-kit-demo-glio-chardev-forward-device
    "SilKitDemoGLIOChardevForwardDevice.cpp"

    "${CMAKE_SOURCE_DIR}/util/Parsing.cpp"
    "${CMAKE_SOURCE_DIR}/util/SignalHandler.cpp"
)

target_link_libraries(sil-kit-demo-glio-chardev-forward-device
    PRIVATE
        SilKit::SilKit
)

target_compile_options(sil-kit-demo-glio-chardev-forward-device
    PRIVATE  ${compile_options}
)

set_target_properties(sil-kit-demo-glio-chardev-forward-device 
    PROPERTIES
    #ensure SilKit shared libraries can be loaded
    INSTALL_RPATH "$ORIGIN/../lib:$ORIGIN"
    BUILD_RPATH "$ORIGIN"
    RUNTIME_OUTPUT_DIRECTORY $<$<BOOL:ALWAYS>:${SILKIT_ADAPTERS_GENERIC_LINUX_IO_OUTPUT_DIRECTORY}>
)