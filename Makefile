# Makefile for Pixel Engine
# Uses clang++ directly for Apple Silicon

CXX = clang++
CXXFLAGS = -std=c++20 -O3 -Wall -Wextra -march=armv8.5-a -mtune=native
OBJCXXFLAGS = $(CXXFLAGS)

# Directories
SRC_DIR = src
INCLUDE_DIR = include
BUILD_DIR = build
SHADER_DIR = shaders

# Frameworks
FRAMEWORKS = -framework Cocoa -framework Metal -framework MetalKit -framework QuartzCore

# Include paths
INCLUDES = -I$(INCLUDE_DIR)

# Source files
CPP_SOURCES = $(SRC_DIR)/main.cpp \
              $(SRC_DIR)/Material.cpp \
              $(SRC_DIR)/World.cpp \
              $(SRC_DIR)/Simulation.cpp

MM_SOURCES = $(SRC_DIR)/MetalRenderer.mm \
             $(SRC_DIR)/Platform.mm

# Object files
CPP_OBJECTS = $(CPP_SOURCES:$(SRC_DIR)/%.cpp=$(BUILD_DIR)/%.o)
MM_OBJECTS = $(MM_SOURCES:$(SRC_DIR)/%.mm=$(BUILD_DIR)/%.o)

ALL_OBJECTS = $(CPP_OBJECTS) $(MM_OBJECTS)

# Output
TARGET = $(BUILD_DIR)/PixelEngine

# Shader
SHADER_SRC = $(SHADER_DIR)/shader.metal
SHADER_LIB = $(BUILD_DIR)/shaders/shader.metallib

.PHONY: all clean run

all: $(TARGET) $(SHADER_LIB)

# Create build directory
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)
	mkdir -p $(BUILD_DIR)/shaders

# Compile C++ source files
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

# Compile Objective-C++ source files
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.mm | $(BUILD_DIR)
	$(CXX) $(OBJCXXFLAGS) $(INCLUDES) -c $< -o $@

# Link executable
$(TARGET): $(ALL_OBJECTS)
	$(CXX) $(OBJCXXFLAGS) $(ALL_OBJECTS) $(FRAMEWORKS) -o $(TARGET)
	@echo "Build complete: $(TARGET)"

# Compile Metal shader
$(SHADER_LIB): $(SHADER_SRC) | $(BUILD_DIR)
	xcrun -sdk macosx metal -c $(SHADER_SRC) -o $(BUILD_DIR)/shaders/shader.air
	xcrun -sdk macosx metallib $(BUILD_DIR)/shaders/shader.air -o $(SHADER_LIB)
	@echo "Shader compiled: $(SHADER_LIB)"

# Run the application
run: all
	cd $(BUILD_DIR) && ./PixelEngine

# Clean build artifacts
clean:
	rm -rf $(BUILD_DIR)

# Print variables for debugging
debug:
	@echo "CXX: $(CXX)"
	@echo "CPP_SOURCES: $(CPP_SOURCES)"
	@echo "MM_SOURCES: $(MM_SOURCES)"
	@echo "CPP_OBJECTS: $(CPP_OBJECTS)"
	@echo "MM_OBJECTS: $(MM_OBJECTS)"
