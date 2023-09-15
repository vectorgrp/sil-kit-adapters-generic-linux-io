
add_executable(SilKitDemoDisplayModifyGPIODevice
    "SilKitDemoDisplayModifyGPIODevice.cpp"
)
target_link_libraries(SilKitDemoDisplayModifyGPIODevice
    PRIVATE
        SilKit::SilKit
)
set_target_properties(SilKitDemoDisplayModifyGPIODevice 
	PROPERTIES
	#ensure SilKit shared libraries can be loaded
    INSTALL_RPATH "$ORIGIN"
    BUILD_RPATH "$ORIGIN"
    RUNTIME_OUTPUT_DIRECTORY $<$<BOOL:ALWAYS>:${SILKIT_ADAPTERS_GPIO_OUTPUT_DIRECTORY}>
)
