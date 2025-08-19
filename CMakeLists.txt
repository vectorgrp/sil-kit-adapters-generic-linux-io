# SPDX-FileCopyrightText: Copyright 2025 Vector Informatik GmbH
# SPDX-License-Identifier: MIT

cmake_minimum_required(VERSION 3.12)
project(VectorSilKitAdapterGenericLinuxIO)

option(ENABLE_ASAN "Enable -f sanitize=address for builds (requires gcc, clang,  VS2019)" OFF)
option(ENABLE_UBSAN "Enable -f sanitize=undefined for builds (requires gcc, clang)" OFF)
option(ENABLE_THREADSAN "Enable -f sanitize=thread for builds (requires gcc, clang)" OFF)
option(BUILD_LINUX_PACKAGE "Creates a SIL Kit Adapter Generic Linux IO build suitable for package managers in Linux Distributions (.deb)" OFF)

if(BUILD_LINUX_PACKAGE)
    add_subdirectory(docs/man)
endif()

set(CMAKE_BUILD_RPATH_USE_ORIGIN ON)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_CXX_STANDARD 14)

set(SILKIT_ADAPTER_GENERIC_LINUX_IO_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/bin")
set(SILKIT_ADAPTER_GENERIC_LINUX_IO_LIBRARY_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/lib")

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_LIST_DIR}/cmake)
include(SilKitAdapterVersion)
configure_silkit_adapter_version(${PROJECT_NAME})
add_definitions(-DSILKIT_ADAPTER_VERSION="${PROJECT_VERSION}")

include(common/cmake/RepoTargets.cmake)
include(common/cmake/Building.cmake)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_LIST_DIR}/cmake)
include(BuildSettings)
enable_address_sanitizer(${ENABLE_ASAN})
enable_undefined_sanitizer(${ENABLE_UBSAN})
enable_thread_sanitizer(${ENABLE_THREADSAN})

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
        ${CMAKE_CURRENT_SOURCE_DIR}/common
        ${CMAKE_CURRENT_SOURCE_DIR}/docs
        ${CMAKE_CURRENT_SOURCE_DIR}/util
        ${CMAKE_CURRENT_SOURCE_DIR}/.devcontainer
        ${CMAKE_CURRENT_SOURCE_DIR}/.vscode
    DESTINATION .
    COMPONENT source
    EXCLUDE_FROM_ALL

    FILES_MATCHING
        PATTERN *.cpp
        PATTERN *.hpp
        PATTERN *.cmake
        PATTERN *.ps1
        PATTERN *.sh
        PATTERN *.json
        PATTERN *.yaml
        PATTERN *.can
        PATTERN *.cfg
        PATTERN *.vcdl
        PATTERN *.vCODM
        PATTERN *.xvp
        PATTERN *.xml
        PATTERN *.fbbin
        PATTERN *.md
        PATTERN *.png
        PATTERN *.1
        PATTERN LICENSE
        PATTERN Dockerfile
        PATTERN CMakeLists.txt

    REGEX "\.git$" EXCLUDE
    REGEX "\.github$" EXCLUDE
)

install(
    DIRECTORY            
        ${CMAKE_CURRENT_SOURCE_DIR}/third_party
    DESTINATION .
    COMPONENT source
    EXCLUDE_FROM_ALL
    REGEX "\.git$" EXCLUDE
    REGEX "\.github$" EXCLUDE
)

include(common/cmake/InstallFunctions.cmake)

install_root_source_files()

install_scripts_to(adapter/demos/CANoe4SW_SE
    ${CMAKE_CURRENT_SOURCE_DIR}/adapter/demos/CANoe4SW_SE/run.sh
    ${CMAKE_CURRENT_SOURCE_DIR}/adapter/demos/CANoe4SW_SE/run_all.sh
)

install_scripts_to(adapter/demos/tasks_scripts
    ${CMAKE_CURRENT_SOURCE_DIR}/adapter/demos/tasks_scripts/build_adapter.sh
    ${CMAKE_CURRENT_SOURCE_DIR}/adapter/demos/tasks_scripts/clean_up.sh
    ${CMAKE_CURRENT_SOURCE_DIR}/adapter/demos/tasks_scripts/prepare_run_silkit_registry.sh
    ${CMAKE_CURRENT_SOURCE_DIR}/adapter/demos/tasks_scripts/run_demos.sh
    ${CMAKE_CURRENT_SOURCE_DIR}/adapter/demos/tasks_scripts/run_adapter.sh
    ${CMAKE_CURRENT_SOURCE_DIR}/adapter/demos/tasks_scripts/stop_processes.sh
    ${CMAKE_CURRENT_SOURCE_DIR}/adapter/demos/tasks_scripts/stop_clean_all.sh
    ${CMAKE_CURRENT_SOURCE_DIR}/adapter/demos/tasks_scripts/update_chips.sh
)

install_scripts_to(advalues/demos/CANoe4SW_SE
    ${CMAKE_CURRENT_SOURCE_DIR}/advalues/demos/CANoe4SW_SE/run.sh
    ${CMAKE_CURRENT_SOURCE_DIR}/advalues/demos/CANoe4SW_SE/run_all.sh
)

install_scripts_to(advalues/demos
    ${CMAKE_CURRENT_SOURCE_DIR}/advalues/demos/create_adchips.sh
)

install_scripts_to(advalues/demos/tasks_scripts
    ${CMAKE_CURRENT_SOURCE_DIR}/advalues/demos/tasks_scripts/clean_up.sh
    ${CMAKE_CURRENT_SOURCE_DIR}/advalues/demos/tasks_scripts/start_demo.sh
    ${CMAKE_CURRENT_SOURCE_DIR}/advalues/demos/tasks_scripts/update_adchip.sh
    ${CMAKE_CURRENT_SOURCE_DIR}/advalues/demos/tasks_scripts/create_adchips_run_adapter.sh
)

install_scripts_to(chardev/demos/CANoe4SW_SE
    ${CMAKE_CURRENT_SOURCE_DIR}/chardev/demos/CANoe4SW_SE/run.sh
    ${CMAKE_CURRENT_SOURCE_DIR}/chardev/demos/CANoe4SW_SE/run_all.sh
)

install_scripts_to(chardev/demos
    ${CMAKE_CURRENT_SOURCE_DIR}/chardev/demos/create_chardevs.sh
)

install_scripts_to(chardev/demos/tasks_scripts
    ${CMAKE_CURRENT_SOURCE_DIR}/chardev/demos/tasks_scripts/clean_up.sh
    ${CMAKE_CURRENT_SOURCE_DIR}/chardev/demos/tasks_scripts/start_demo.sh
    ${CMAKE_CURRENT_SOURCE_DIR}/chardev/demos/tasks_scripts/update_chardev.sh
    ${CMAKE_CURRENT_SOURCE_DIR}/chardev/demos/tasks_scripts/create_chardevs_run_adapter.sh
)

include(GNUInstallDirs)

install_silkit_lib(
    ${SILKIT_ADAPTER_GENERIC_LINUX_IO_OUTPUT_DIRECTORY}
    ${SILKIT_ADAPTER_GENERIC_LINUX_IO_LIBRARY_DIRECTORY}
)

install_demo(sil-kit-demo-glio-advalues-forward-device)
install_demo(sil-kit-demo-glio-chardev-forward-device)

# Disable GPIO mode on QNX
if(NOT DEFINED QNX_BUILD)
    install_git_source(${CMAKE_CURRENT_SOURCE_DIR}/gpio)

    install_scripts_to(gpio/demos/CANoe4SW_SE
        ${CMAKE_CURRENT_SOURCE_DIR}/gpio/demos/CANoe4SW_SE/run.sh
        ${CMAKE_CURRENT_SOURCE_DIR}/gpio/demos/CANoe4SW_SE/run_all.sh
    )

    install_scripts_to(gpio/demos
        ${CMAKE_CURRENT_SOURCE_DIR}/gpio/demos/create_gpio_sim.sh
        ${CMAKE_CURRENT_SOURCE_DIR}/gpio/demos/clean_gpio_sim.sh
    )

    install_scripts_to(gpio/demos/tasks_scripts
        ${CMAKE_CURRENT_SOURCE_DIR}/gpio/demos/tasks_scripts/clean_up.sh
        ${CMAKE_CURRENT_SOURCE_DIR}/gpio/demos/tasks_scripts/start_demo.sh
        ${CMAKE_CURRENT_SOURCE_DIR}/gpio/demos/tasks_scripts/update_gpio.sh
        ${CMAKE_CURRENT_SOURCE_DIR}/gpio/demos/tasks_scripts/run_adapter.sh
    )

    install_demo(sil-kit-demo-glio-gpio-forward-device)
endif()

install_adapter(sil-kit-adapter-generic-linux-io)

###############################################################################
# Packaging
###############################################################################
include(common/cmake/Packaging.cmake)

if(NOT DEFINED PACKAGE_FILENAME_SUFFIX)
    set(PACKAGE_FILENAME_SUFFIX "-${PROJECT_VERSION}")
endif()

declare_package(
    "SilKit-Adapter-Generic-Linux-IO"
    "binary release of SIL Kit Adapter Generic Linux IO"
)

set(CPACK_PACKAGE_VERSION_MAJOR ${PROJECT_VERSION_MAJOR})
set(CPACK_PACKAGE_VERSION_MINOR ${PROJECT_VERSION_MINOR})
set(CPACK_PACKAGE_VERSION_PATCH ${PROJECT_VERSION_PATCH})
