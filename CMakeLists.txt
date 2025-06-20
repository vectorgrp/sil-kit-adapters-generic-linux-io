# SPDX-FileCopyrightText: Copyright 2025 Vector Informatik GmbH
# SPDX-License-Identifier: MIT

cmake_minimum_required(VERSION 3.5)
project(VectorSilKitAdapterGenericLinuxIO)
include(FetchContent)

set(CMAKE_BUILD_RPATH_USE_ORIGIN ON)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_CXX_STANDARD 14)

set(SILKIT_ADAPTER_GENERIC_LINUX_IO_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/bin")
set(SILKIT_ADAPTER_GENERIC_LINUX_IO_LIBRARY_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/lib")

set(SILKIT_VERSION "4.0.50" CACHE STRING "If no SIL Kit package is specified, this version will be downloaded")
set(SILKIT_FLAVOR "ubuntu-18.04-x86_64-gcc" CACHE STRING "If no SIL Kit package is specified, this package flavor will be downloaded")

find_package(Threads REQUIRED)
add_library(asio INTERFACE)
target_include_directories(asio INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/third_party/asio/asio/include)
target_compile_definitions(asio INTERFACE ASIO_STANDALONE=1)
target_link_libraries(asio INTERFACE Threads::Threads)

# Set SIL Kit lib name
if(CMAKE_BUILD_TYPE STREQUAL "Release")
    set(SILKIT_SO_NAME "libSilKit.so")
else() # Debug or RelWithDebInfo
    set(SILKIT_SO_NAME "libSilKitd.so")
endif()

if(DEFINED SILKIT_PACKAGE_DIR)
    # if the user forces using a specific SIL Kit package, use it
	message(STATUS "SILKIT_PACKAGE_DIR has been set to: ${SILKIT_PACKAGE_DIR}, CMAKE will look for a SIL Kit package in that directory")
	find_package(SilKit 4.0.40
		REQUIRED 
		CONFIG  
		NO_CMAKE_PACKAGE_REGISTRY 
		NO_DEFAULT_PATH 
		PATHS "${SILKIT_PACKAGE_DIR}")
else()
    # otherwise, look for an installed version of SIL Kit
    message(STATUS "SILKIT_PACKAGE_DIR has not been set by user. Attempting to find an installed version of SIL Kit")
    find_package(SilKit 4.0.40
        CONFIG)
        
    if(NOT SilKit_FOUND)
    # if installed SIL Kit is not found, fetch it from github.com
        message(STATUS "No version of SIL Kit present. Attempting to fetch [SilKit-${SILKIT_VERSION}-${SILKIT_FLAVOR}] from github.com")
        FetchContent_Declare(
                silkit
                URL https://github.com/vectorgrp/sil-kit/releases/download/sil-kit%2Fv${SILKIT_VERSION}/SilKit-${SILKIT_VERSION}-${SILKIT_FLAVOR}.zip
                DOWNLOAD_DIR ${CMAKE_CURRENT_LIST_DIR}/Downloads
            )

        message(STATUS "SIL Kit: fetching [SilKit-${SILKIT_VERSION}-${SILKIT_FLAVOR}]")
        FetchContent_MakeAvailable(silkit)

        set(SILKIT_SOURCE_DIR ${silkit_SOURCE_DIR}/SilKit-Source)
        message(STATUS "SIL Kit: using source code from: \"${SILKIT_SOURCE_DIR}\"")
        message(STATUS "SIL Kit: using pre-built binaries from: ${silkit_SOURCE_DIR}")

        message(STATUS "Searching SilKit package which has been fetched from github.com")
        find_package(SilKit 4.0.40
            REQUIRED
            CONFIG
            NO_CMAKE_PACKAGE_REGISTRY
            NO_DEFAULT_PATH
            PATHS "${silkit_SOURCE_DIR}"
        )
    endif()	
endif()
	
if(TARGET SilKit::SilKit)
    message(STATUS "SIL Kit package has been successfully imported as a CMake target. [version : ${SilKit_VERSION}]")
else()
	message(FATAL_ERROR "Something went wrong : Could not find SIL Kit package.")
endif()

set(compile_options
    -pedantic -Wall -Wextra
    -Wcast-align -Wformat=2
    -Wshadow -Wsign-promo -Wstrict-overflow=5
    -Wno-unused
    -Wpacked
    -Wfatal-errors
)

if(DEFINED ENV{QNX_BUILD})
    set(QNX_BUILD 1)
    add_definitions(-DQNX_BUILD=${QNX_BUILD})
endif()

add_subdirectory(adapter)
add_subdirectory(advalues/demos)
add_subdirectory(chardev/demos)
add_subdirectory(util)
add_subdirectory(third_party)

if(NOT DEFINED QNX_BUILD)
    add_subdirectory(gpio/demos)
endif()

################################################################################
# Distribution of the source code and binaries
################################################################################
# Install sources and binaries
# Copy all files from the source and bin directory to the proper destination
# Leave out git repo related data

install(
    DIRECTORY
        ${CMAKE_CURRENT_SOURCE_DIR}/adapter
        ${CMAKE_CURRENT_SOURCE_DIR}/advalues
        ${CMAKE_CURRENT_SOURCE_DIR}/chardev
        ${CMAKE_CURRENT_SOURCE_DIR}/cmake
        ${CMAKE_CURRENT_SOURCE_DIR}/third_party
        ${CMAKE_CURRENT_SOURCE_DIR}/.devcontainer
        ${CMAKE_CURRENT_SOURCE_DIR}/.vscode
    DESTINATION .
    COMPONENT source
    EXCLUDE_FROM_ALL
    REGEX "\.git$" EXCLUDE
    REGEX "\.github$" EXCLUDE
)

install(
    FILES
        CMakeLists.txt
        CMakePresets.json
        LICENSE
        README.md
        SECURITY.md
        CONTRIBUTING.md
    DESTINATION .
    COMPONENT source
    EXCLUDE_FROM_ALL
)

install(
    FILES
        ${CMAKE_CURRENT_SOURCE_DIR}/adapter/demos/CANoe4SW_SE/run.sh
        ${CMAKE_CURRENT_SOURCE_DIR}/adapter/demos/CANoe4SW_SE/run_all.sh
        ${CMAKE_CURRENT_SOURCE_DIR}/adapter/demos/CANoe4SW_SE/run_adapter.sh
    DESTINATION adapter/demos/CANoe4SW_SE
    PERMISSIONS
        OWNER_READ OWNER_WRITE OWNER_EXECUTE
        GROUP_READ GROUP_EXECUTE
        WORLD_READ WORLD_EXECUTE
    COMPONENT source
    EXCLUDE_FROM_ALL
)

install(
    FILES
        ${CMAKE_CURRENT_SOURCE_DIR}/adapter/demos/tasks_scripts/build_adapter.sh
        ${CMAKE_CURRENT_SOURCE_DIR}/adapter/demos/tasks_scripts/clean_up.sh
        ${CMAKE_CURRENT_SOURCE_DIR}/adapter/demos/tasks_scripts/prepare_run_silkit_registry.sh
        ${CMAKE_CURRENT_SOURCE_DIR}/adapter/demos/tasks_scripts/run_demos.sh
        ${CMAKE_CURRENT_SOURCE_DIR}/adapter/demos/tasks_scripts/stop_processes.sh
        ${CMAKE_CURRENT_SOURCE_DIR}/adapter/demos/tasks_scripts/stop_clean_all.sh
        ${CMAKE_CURRENT_SOURCE_DIR}/adapter/demos/tasks_scripts/update_chips.sh
    DESTINATION adapter/demos/tasks_scripts/tasks_scripts
    PERMISSIONS
        OWNER_READ OWNER_WRITE OWNER_EXECUTE
        GROUP_READ GROUP_EXECUTE
        WORLD_READ WORLD_EXECUTE
    COMPONENT source
    EXCLUDE_FROM_ALL
)

install(
    FILES
        ${CMAKE_CURRENT_SOURCE_DIR}/advalues/demos/CANoe4SW_SE/run.sh
        ${CMAKE_CURRENT_SOURCE_DIR}/advalues/demos/CANoe4SW_SE/run_all.sh
        ${CMAKE_CURRENT_SOURCE_DIR}/advalues/demos/CANoe4SW_SE/create_adchips_run_adapter.sh
    DESTINATION advalues/demos/CANoe4SW_SE
    PERMISSIONS
        OWNER_READ OWNER_WRITE OWNER_EXECUTE
        GROUP_READ GROUP_EXECUTE
        WORLD_READ WORLD_EXECUTE
    COMPONENT source
    EXCLUDE_FROM_ALL
)

install(
    FILES
        ${CMAKE_CURRENT_SOURCE_DIR}/advalues/demos/create_adchips.sh
    DESTINATION advalues/demos
    PERMISSIONS
        OWNER_READ OWNER_WRITE OWNER_EXECUTE
        GROUP_READ GROUP_EXECUTE
        WORLD_READ WORLD_EXECUTE
    COMPONENT source
    EXCLUDE_FROM_ALL
)

install(
    FILES
        ${CMAKE_CURRENT_SOURCE_DIR}/advalues/demos/tasks_scripts/clean_up.sh
        ${CMAKE_CURRENT_SOURCE_DIR}/advalues/demos/tasks_scripts/start_demo.sh
        ${CMAKE_CURRENT_SOURCE_DIR}/advalues/demos/tasks_scripts/update_adchip.sh
    DESTINATION advalues/demos/tasks_scripts
    PERMISSIONS
        OWNER_READ OWNER_WRITE OWNER_EXECUTE
        GROUP_READ GROUP_EXECUTE
        WORLD_READ WORLD_EXECUTE
    COMPONENT source
    EXCLUDE_FROM_ALL
)

install(
    FILES
        ${CMAKE_CURRENT_SOURCE_DIR}/chardev/demos/CANoe4SW_SE/run.sh
        ${CMAKE_CURRENT_SOURCE_DIR}/chardev/demos/CANoe4SW_SE/run_all.sh
        ${CMAKE_CURRENT_SOURCE_DIR}/chardev/demos/CANoe4SW_SE/create_chardevs_run_adapter.sh
    DESTINATION chardev/demos/CANoe4SW_SE
    PERMISSIONS
        OWNER_READ OWNER_WRITE OWNER_EXECUTE
        GROUP_READ GROUP_EXECUTE
        WORLD_READ WORLD_EXECUTE
    COMPONENT source
    EXCLUDE_FROM_ALL
)

install(
    FILES
        ${CMAKE_CURRENT_SOURCE_DIR}/chardev/demos/create_chardevs.sh
    DESTINATION chardev/demos
    PERMISSIONS
        OWNER_READ OWNER_WRITE OWNER_EXECUTE
        GROUP_READ GROUP_EXECUTE
        WORLD_READ WORLD_EXECUTE
    COMPONENT source
    EXCLUDE_FROM_ALL
)

install(
    FILES
        ${CMAKE_CURRENT_SOURCE_DIR}/chardev/demos/tasks_scripts/clean_up.sh
        ${CMAKE_CURRENT_SOURCE_DIR}/chardev/demos/tasks_scripts/start_demo.sh
        ${CMAKE_CURRENT_SOURCE_DIR}/chardev/demos/tasks_scripts/update_chardev.sh
    DESTINATION chardev/demos/tasks_scripts
    PERMISSIONS
        OWNER_READ OWNER_WRITE OWNER_EXECUTE
        GROUP_READ GROUP_EXECUTE
        WORLD_READ WORLD_EXECUTE
    COMPONENT source
    EXCLUDE_FROM_ALL
)
include(GNUInstallDirs)

install(
    FILES 
        ${SILKIT_ADAPTER_GENERIC_LINUX_IO_LIBRARY_DIRECTORY}/${SILKIT_SO_NAME}
    DESTINATION ${CMAKE_INSTALL_LIBDIR}
    PERMISSIONS
        OWNER_READ OWNER_WRITE
        GROUP_READ
        WORLD_READ
    COMPONENT lib
    EXCLUDE_FROM_ALL
)

install(
    TARGETS 
        sil-kit-demo-glio-advalues-forward-device
        sil-kit-demo-glio-chardev-forward-device
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR} COMPONENT Runtime
    PERMISSIONS
        OWNER_READ OWNER_WRITE OWNER_EXECUTE
        GROUP_READ GROUP_EXECUTE
        WORLD_READ WORLD_EXECUTE    
    EXCLUDE_FROM_ALL
)

# Disable GPIO mode on QNX
if(NOT DEFINED QNX_BUILD)
    install(
        DIRECTORY
            ${CMAKE_CURRENT_SOURCE_DIR}/gpio
        DESTINATION .
        COMPONENT source
        EXCLUDE_FROM_ALL
        REGEX "\.git$" EXCLUDE
        REGEX "\.github$" EXCLUDE
    )

    install(
        FILES
            ${CMAKE_CURRENT_SOURCE_DIR}/gpio/demos/CANoe4SW_SE/run.sh
            ${CMAKE_CURRENT_SOURCE_DIR}/gpio/demos/CANoe4SW_SE/run_all.sh
            ${CMAKE_CURRENT_SOURCE_DIR}/gpio/demos/CANoe4SW_SE/run_adapter.sh
        DESTINATION gpio/demos/CANoe4SW_SE
        PERMISSIONS
            OWNER_READ OWNER_WRITE OWNER_EXECUTE
            GROUP_READ GROUP_EXECUTE
            WORLD_READ WORLD_EXECUTE
        COMPONENT source
        EXCLUDE_FROM_ALL
    )

    install(
        FILES
            ${CMAKE_CURRENT_SOURCE_DIR}/gpio/demos/create_gpio_sim.sh
            ${CMAKE_CURRENT_SOURCE_DIR}/gpio/demos/clean_gpio_sim.sh
        DESTINATION gpio/demos
        PERMISSIONS
            OWNER_READ OWNER_WRITE OWNER_EXECUTE
            GROUP_READ GROUP_EXECUTE
            WORLD_READ WORLD_EXECUTE
        COMPONENT source
        EXCLUDE_FROM_ALL
    )

    install(
    FILES
        ${CMAKE_CURRENT_SOURCE_DIR}/gpio/demos/tasks_scripts/clean_up.sh
        ${CMAKE_CURRENT_SOURCE_DIR}/gpio/demos/tasks_scripts/start_demo.sh
        ${CMAKE_CURRENT_SOURCE_DIR}/gpio/demos/tasks_scripts/update_gpio.sh
    DESTINATION gpio/demos/tasks_scripts
    PERMISSIONS
        OWNER_READ OWNER_WRITE OWNER_EXECUTE
        GROUP_READ GROUP_EXECUTE
        WORLD_READ WORLD_EXECUTE
    COMPONENT source
    EXCLUDE_FROM_ALL
    )

    install(
        TARGETS
            sil-kit-demo-glio-gpio-forward-device
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR} COMPONENT Runtime
        PERMISSIONS
            OWNER_READ OWNER_WRITE OWNER_EXECUTE
            GROUP_READ GROUP_EXECUTE
            WORLD_READ WORLD_EXECUTE    
        EXCLUDE_FROM_ALL
    )
endif()

# only sil-kit-adapter-generic-linux-io should be installed to /usr/local/bin by calling --target install (therefore it is not excluded)
install(
    TARGETS 
        sil-kit-adapter-generic-linux-io
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR} COMPONENT Runtime
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR} COMPONENT Runtime
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}/sil-kit-adapter-generic-linux-io COMPONENT Development   
    PERMISSIONS
        OWNER_READ OWNER_WRITE OWNER_EXECUTE
        GROUP_READ GROUP_EXECUTE
        WORLD_READ WORLD_EXECUTE
)

###############################################################################
# Packaging
###############################################################################

set(CPACK_GENERATOR "ZIP")

set(CPACK_PACKAGE_DESCRIPTION "binary release of SIL Kit Adapter Generic Linux IO")
set(CPACK_PACKAGE_NAME "SilKit-Adapter-Generic-Linux-IO")
set(CPACK_PACKAGE_VENDOR "Vector Informatik")
set(CPACK_PACKAGE_CONTACT "support@vector.com")
set(CPACK_PACKAGE_FILE_NAME "${CPACK_PACKAGE_NAME}${PACKAGE_FILENAME_SUFFIX}")

set(CPACK_ARCHIVE_COMPONENT_INSTALL ON)
set(CPACK_COMPONENTS_ALL_IN_ONE_PACKAGE ON)
set(CPACK_COMPONENT_INCLUDE_TOPLEVEL_DIRECTORY ON)

include(CPack)
