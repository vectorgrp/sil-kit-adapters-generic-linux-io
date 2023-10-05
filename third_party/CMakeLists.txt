include(ExternalProject)

message(STATUS "Add subdirectory ThirdParty/libgpiod")

ExternalProject_Add(gpiod
    SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/libgpiod/"
    BINARY_DIR "${CMAKE_SOURCE_DIR}/build/"
    CONFIGURE_COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/libgpiod/autogen.sh --enable-tools=yes --enable-bindings-cxx --prefix=<BINARY_DIR> > /dev/null 2>&1
    BUILD_COMMAND make > /dev/null 2>&1
    INSTALL_COMMAND make install > /dev/null 2>&1
)
