# Third party YAML parsing 

add_subdirectory(yaml-cpp
    yaml-cpp
    EXCLUDE_FROM_ALL
)

add_library(YamlInterface INTERFACE)
target_include_directories(YamlInterface
    INTERFACE $<TARGET_PROPERTY:yaml-cpp,INTERFACE_INCLUDE_DIRECTORIES>)