# Copyright (c) Vector Informatik GmbH. All rights reserved.

function (BuildDemo demoName)

    add_executable(${demoName}
    "${demoName}.cpp"
    "${CMAKE_SOURCE_DIR}/gpio/adapter/ChipDatas.cpp"
    )

    target_include_directories(${demoName}
    PUBLIC
        "${CMAKE_SOURCE_DIR}/gpio/"
    )

    target_link_libraries(${demoName}
    PRIVATE
        SilKit::SilKit
    )
    set_target_properties(${demoName} 
    PROPERTIES
    #ensure SilKit shared libraries can be loaded
    INSTALL_RPATH "$ORIGIN"
    BUILD_RPATH "$ORIGIN"
    RUNTIME_OUTPUT_DIRECTORY $<$<BOOL:ALWAYS>:${SILKIT_ADAPTERS_GPIO_OUTPUT_DIRECTORY}>
    )

    target_compile_options(${demoName}
    PRIVATE
        #-g -O0 # Compile options for valgrind
        -pedantic -Wall -Wextra
        -Wcast-align -Wformat=2
        -Wshadow -Wsign-conversion -Wsign-promo -Wstrict-overflow=5
        -Wno-unused
        -Wpacked
        -Werror -Wfatal-errors
    )

endfunction()

BuildDemo(SilKitDemoDisplayGPIODevice)
BuildDemo(SilKitDemoModifyGPIODevice)