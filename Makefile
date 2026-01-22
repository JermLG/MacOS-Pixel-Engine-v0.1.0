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
              $(SRC_DIR)/Simulation.cpp \
              $(SRC_DIR)/DiscoverySystem.cpp

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

# ============================================================================
# App Bundle
# ============================================================================
APP_NAME = PixelEngine
APP_BUNDLE = $(BUILD_DIR)/$(APP_NAME).app
APP_CONTENTS = $(APP_BUNDLE)/Contents
APP_MACOS = $(APP_CONTENTS)/MacOS
APP_RESOURCES = $(APP_CONTENTS)/Resources

.PHONY: app

# Build the .app bundle
app: $(TARGET) $(SHADER_LIB)
	@echo "Creating app bundle..."
	@mkdir -p $(APP_MACOS)
	@mkdir -p $(APP_RESOURCES)/shaders
	@# Copy executable
	@cp $(TARGET) $(APP_MACOS)/$(APP_NAME)
	@# Copy shaders
	@cp $(SHADER_LIB) $(APP_RESOURCES)/shaders/
	@# Create Info.plist
	@echo '<?xml version="1.0" encoding="UTF-8"?>' > $(APP_CONTENTS)/Info.plist
	@echo '<!DOCTYPE plist PUBLIC "-//Apple//DTD PLIST 1.0//EN" "http://www.apple.com/DTDs/PropertyList-1.0.dtd">' >> $(APP_CONTENTS)/Info.plist
	@echo '<plist version="1.0">' >> $(APP_CONTENTS)/Info.plist
	@echo '<dict>' >> $(APP_CONTENTS)/Info.plist
	@echo '    <key>CFBundleExecutable</key>' >> $(APP_CONTENTS)/Info.plist
	@echo '    <string>$(APP_NAME)</string>' >> $(APP_CONTENTS)/Info.plist
	@echo '    <key>CFBundleIdentifier</key>' >> $(APP_CONTENTS)/Info.plist
	@echo '    <string>com.vibin.pixelengine</string>' >> $(APP_CONTENTS)/Info.plist
	@echo '    <key>CFBundleName</key>' >> $(APP_CONTENTS)/Info.plist
	@echo '    <string>$(APP_NAME)</string>' >> $(APP_CONTENTS)/Info.plist
	@echo '    <key>CFBundleDisplayName</key>' >> $(APP_CONTENTS)/Info.plist
	@echo '    <string>Pixel Engine</string>' >> $(APP_CONTENTS)/Info.plist
	@echo '    <key>CFBundleVersion</key>' >> $(APP_CONTENTS)/Info.plist
	@echo '    <string>1.0.0</string>' >> $(APP_CONTENTS)/Info.plist
	@echo '    <key>CFBundleShortVersionString</key>' >> $(APP_CONTENTS)/Info.plist
	@echo '    <string>1.0.0</string>' >> $(APP_CONTENTS)/Info.plist
	@echo '    <key>CFBundlePackageType</key>' >> $(APP_CONTENTS)/Info.plist
	@echo '    <string>APPL</string>' >> $(APP_CONTENTS)/Info.plist
	@echo '    <key>CFBundleIconFile</key>' >> $(APP_CONTENTS)/Info.plist
	@echo '    <string>AppIcon</string>' >> $(APP_CONTENTS)/Info.plist
	@echo '    <key>LSMinimumSystemVersion</key>' >> $(APP_CONTENTS)/Info.plist
	@echo '    <string>11.0</string>' >> $(APP_CONTENTS)/Info.plist
	@echo '    <key>NSHighResolutionCapable</key>' >> $(APP_CONTENTS)/Info.plist
	@echo '    <true/>' >> $(APP_CONTENTS)/Info.plist
	@echo '    <key>NSSupportsAutomaticGraphicsSwitching</key>' >> $(APP_CONTENTS)/Info.plist
	@echo '    <true/>' >> $(APP_CONTENTS)/Info.plist
	@echo '</dict>' >> $(APP_CONTENTS)/Info.plist
	@echo '</plist>' >> $(APP_CONTENTS)/Info.plist
	@# Copy icon if it exists
	@if [ -f "resources/AppIcon.icns" ]; then \
		cp resources/AppIcon.icns $(APP_RESOURCES)/; \
	fi
	@echo "App bundle created: $(APP_BUNDLE)"

# Create a distributable DMG
.PHONY: dmg
dmg: app
	@echo "Creating DMG..."
	@rm -f $(BUILD_DIR)/$(APP_NAME).dmg
	@hdiutil create -volname "$(APP_NAME)" -srcfolder $(APP_BUNDLE) -ov -format UDZO $(BUILD_DIR)/$(APP_NAME).dmg
	@echo "DMG created: $(BUILD_DIR)/$(APP_NAME).dmg"

# ============================================================================
# itch.io Publishing
# ============================================================================
ITCH_USER = JermLG
ITCH_GAME = pixel-engine
BUTLER = arch -x86_64 ~/bin/butler

.PHONY: publish
publish: app
	@echo "Publishing to itch.io..."
	@BUTLER_API_KEY=$$(cat ~/.config/itch/butler_creds) $(BUTLER) push $(APP_BUNDLE) $(ITCH_USER)/$(ITCH_GAME):mac
	@echo "Published to https://$(ITCH_USER).itch.io/$(ITCH_GAME)"
