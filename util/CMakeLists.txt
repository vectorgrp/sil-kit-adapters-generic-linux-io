# SPDX-FileCopyrightText: Copyright 2025 Vector Informatik GmbH
# SPDX-License-Identifier: MIT

add_library(Utility STATIC
    FileHelper.cpp
    YamlHelper.cpp
    Exceptions.hpp
)

if(DEFINED QNX_BUILD)
    target_compile_definitions(Utility PRIVATE QNX_BUILD=${QNX_BUILD})
endif()

target_link_libraries(Utility PUBLIC
    asio
    SilKit::SilKit
    yaml-cpp
    sil-kit-adapters-common
)

if(MINGW)
    target_compile_definitions(Utility INTERFACE _WIN32_WINNT=0x0601)
    target_link_libraries(Utility PUBLIC -lwsock32 -lws2_32) #windows socket/ wsa
endif()
