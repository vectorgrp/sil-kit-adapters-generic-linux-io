# Third party YAML parsing
set(THIRD_PARTY_BINARY_DIR "${CMAKE_CURRENT_BINARY_DIR}" CACHE STRING "" FORCE)

set(YAML_BUILD_SHARED_LIBS OFF CACHE BOOL "Disable building yaml-cpp as shared library" FORCE)

add_subdirectory(
    yaml-cpp
    "${THIRD_PARTY_BINARY_DIR}/_tp_yaml-cpp"
    EXCLUDE_FROM_ALL
)

if(CMAKE_CXX_COMPILER_ID MATCHES "Clang" AND TARGET yaml-cpp)
    target_compile_options(yaml-cpp INTERFACE -Wno-shadow)
endif()

add_library(YamlInterface INTERFACE)
target_include_directories(YamlInterface
    INTERFACE $<TARGET_PROPERTY:yaml-cpp,INTERFACE_INCLUDE_DIRECTORIES>)