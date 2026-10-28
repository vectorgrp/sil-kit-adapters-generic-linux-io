# Third party YAML parsing
set(THIRD_PARTY_BINARY_DIR "${CMAKE_CURRENT_BINARY_DIR}" CACHE STRING "" FORCE)

add_subdirectory(
    rapidyaml
    "${THIRD_PARTY_BINARY_DIR}/_tp_rapidyaml"
    EXCLUDE_FROM_ALL
)

add_library(YamlInterface INTERFACE)
target_include_directories(YamlInterface
    INTERFACE $<TARGET_PROPERTY:rapidyaml-static,INTERFACE_INCLUDE_DIRECTORIES>)
