cmake_minimum_required(VERSION 3.20)
project(PixelEngine VERSION 1.0 LANGUAGES CXX OBJCXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Platform check
if(NOT APPLE)
    message(FATAL_ERROR "This engine requires macOS and Apple Silicon")
endif()

# Source files
set(SOURCES
    src/main.cpp
    src/Material.cpp
    src/World.cpp
    src/Simulation.cpp
    src/MetalRenderer.mm
    src/Platform.mm
)

set(HEADERS
    include/Types.h
    include/Material.h
    include/World.h
    include/Simulation.h
    include/MetalRenderer.h
    include/Platform.h
)

# Create executable
add_executable(PixelEngine ${SOURCES} ${HEADERS})

# Include directories
target_include_directories(PixelEngine PRIVATE include)

# Compiler flags for Apple Silicon optimization
target_compile_options(PixelEngine PRIVATE
    -Wall
    -Wextra
    -O3
    -march=armv8.5-a  # Apple Silicon optimization
    -mtune=native
)

# macOS frameworks
target_link_libraries(PixelEngine
    "-framework Cocoa"
    "-framework Metal"
    "-framework MetalKit"
    "-framework QuartzCore"
)

# Copy shaders to build directory
add_custom_command(TARGET PixelEngine POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/shaders
    $<TARGET_FILE_DIR:PixelEngine>/shaders
)
