cmake_minimum_required(VERSION 3.14)
project(NormalReconstructZ)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Maya version and locations
set(MAYA_VERSION 2025)
set(MAYA_LOCATION "C:/Program Files/Autodesk/Maya${MAYA_VERSION}" CACHE PATH "Maya installation directory")
set(MAYA_DEVKIT_LOCATION "C:/Program Files/Autodesk/Maya${MAYA_VERSION}/devkit/devkitBase" CACHE PATH "Maya devkit directory")

# Set the correct extension for Maya plugins
set(CMAKE_SHARED_LIBRARY_SUFFIX ".mll")

# Include Maya directories
include_directories(
    ${MAYA_DEVKIT_LOCATION}/include
)

# Link directories - using devkit lib path
link_directories(
    ${MAYA_DEVKIT_LOCATION}/lib
)

# Create plugin
add_library(NormalReconstructZ SHARED
    normalReconstructZ.cpp
)

# Set output name and specific properties for Windows
set_target_properties(NormalReconstructZ PROPERTIES
    PREFIX ""
    SUFFIX ${CMAKE_SHARED_LIBRARY_SUFFIX}
)

# Windows-specific definitions
target_compile_definitions(NormalReconstructZ PRIVATE
    NOMINMAX
    _USE_MATH_DEFINES
    NT_PLUGIN
    REQUIRE_IOSTREAM
)

# Link Maya libraries
target_link_libraries(NormalReconstructZ
    Foundation.lib
    OpenMaya.lib
    OpenMayaUI.lib
    OpenMayaRender.lib
)

# Set the output directory
set_target_properties(NormalReconstructZ PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_SOURCE_DIR}/output"
)
