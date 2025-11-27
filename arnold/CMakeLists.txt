# CMakeLists for Arnold Shader
cmake_minimum_required(VERSION 3.14)
project(aiNormalReconstructZ)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Arnold SDK location
set(ARNOLD_SDK "C:/Program Files/Autodesk/Arnold/maya2025" CACHE PATH "Arnold SDK directory")

# Include Arnold headers
include_directories("${ARNOLD_SDK}/include")
include_directories("${ARNOLD_SDK}/include/arnold")

# Link Arnold library
link_directories("${ARNOLD_SDK}/lib")

# Create shared library (DLL)
add_library(aiNormalReconstructZ SHARED
    aiNormalReconstructZ.cpp
)

# Windows definitions
target_compile_definitions(aiNormalReconstructZ PRIVATE
    _WIN64
    WIN32
)

# Link Arnold library
target_link_libraries(aiNormalReconstructZ
    ai.lib
)

# Output settings
set_target_properties(aiNormalReconstructZ PROPERTIES
    PREFIX ""
    SUFFIX ".dll"
    RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_SOURCE_DIR}/output"
    LIBRARY_OUTPUT_DIRECTORY_RELEASE "${CMAKE_SOURCE_DIR}/output"
)

# Copy .mtd file to output
add_custom_command(TARGET aiNormalReconstructZ POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_SOURCE_DIR}/output"
    COMMAND ${CMAKE_COMMAND} -E copy
    "${CMAKE_SOURCE_DIR}/aiNormalReconstructZ.mtd"
    "${CMAKE_SOURCE_DIR}/output/aiNormalReconstructZ.mtd"
)
