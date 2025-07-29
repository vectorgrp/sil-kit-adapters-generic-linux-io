# SPDX-FileCopyrightText: Copyright 2025 Vector Informatik GmbH
# SPDX-License-Identifier: MIT

set(SOURCES_FILES
    "SilKitAdapterGenericLinuxIO.cpp"
    "InotifyHandler.cpp"
    "AdapterFactory.cpp"

    "../chardev/adapter/ChardevAdapter.cpp"
    "../advalues/adapter/AdAdapter.cpp"
)

if(NOT DEFINED QNX_BUILD)
    set(SOURCES_GPIO
        "../gpio/adapter/GpioAdapter.cpp"
        "../gpio/adapter/GpioWrapper.cpp"
    )
endif()

add_executable(sil-kit-adapter-generic-linux-io
    ${SOURCES_FILES}
    ${SOURCES_GPIO}
)

target_include_directories(sil-kit-adapter-generic-linux-io 
    PUBLIC 
        ${CMAKE_CURRENT_SOURCE_DIR}
        "${CMAKE_SOURCE_DIR}"
)

target_link_libraries(sil-kit-adapter-generic-linux-io
    PRIVATE
        SilKit::SilKit
        asio
        Utility
        sil-kit-adapters-common
)

set_target_properties(sil-kit-adapter-generic-linux-io 
	PROPERTIES
    #ensure SilKit shared libraries can be loaded
    INSTALL_RPATH "$ORIGIN/../lib:$ORIGIN"
    BUILD_RPATH "$ORIGIN"
    RUNTIME_OUTPUT_DIRECTORY $<$<BOOL:ALWAYS>:${SILKIT_ADAPTER_GENERIC_LINUX_IO_OUTPUT_DIRECTORY}>
)

target_compile_options(sil-kit-adapter-generic-linux-io
    PRIVATE ${compile_options}
)

copy_sil_kit_to_bin(sil-kit-adapter-generic-linux-io
    ${SILKIT_ADAPTER_GENERIC_LINUX_IO_OUTPUT_DIRECTORY}
    ${SILKIT_ADAPTER_GENERIC_LINUX_IO_LIBRARY_DIRECTORY}
)