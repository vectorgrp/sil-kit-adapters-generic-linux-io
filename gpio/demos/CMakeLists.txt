
add_executable(SilKitDemoDisplayGPIODevice
    "SilKitDemoDisplayGPIODevice.cpp"
 )

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


 add_executable(SilKitDemoModifyGPIODevice
    "SilKitDemoModifyGPIODevice.cpp"
 )

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