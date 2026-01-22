#pragma once

#include "Types.h"
#include <functional>

namespace PixelEngine {

// Brush shape enum
enum class BrushShape : uint8_t {
    Circle = 0,
    Square = 1
};

// Tool mode enum
enum class ToolMode : uint8_t {
    Brush = 0,      // Normal brush for placing materials
    Line = 1,       // Line tool for drawing platforms
    Rectangle = 2,  // Rectangle tool (click and drag)
    Circle = 3,     // Circle/ellipse tool (click and drag)
    Fill = 4,       // Flood fill tool
    Pipette = 5     // Inspect/pick material under cursor
};

// Input state
struct InputState {
    bool mouse_left_down;
    bool mouse_right_down;
    int32_t mouse_x;
    int32_t mouse_y;

    // Current window/view size (updated on resize)
    int32_t view_width;
    int32_t view_height;

    // Current material to spawn (controlled by keyboard)
    MaterialID selected_material;

    // Brush settings
    int32_t brush_radius;
    BrushShape brush_shape;

    // Tool mode
    ToolMode tool_mode;

    // Shape tool state (for line, rectangle, circle tools)
    bool shape_drawing;     // Currently drawing a shape
    int32_t shape_start_x;  // Start point of shape
    int32_t shape_start_y;

    // Tool options
    bool filled_shapes;     // Draw filled shapes vs outlines only

    // Clear world flag (set by keyboard, cleared after processing)
    bool clear_world;

    // Debug GUI toggle
    bool show_debug_gui;

    // Post-processing controls (one-shot toggles, cleared after processing)
    bool toggle_post_processing;  // F1 - toggle all post-processing
    bool toggle_bloom;            // F2 - toggle bloom only
    bool toggle_color;            // F3 - toggle color grading
    bool toggle_vignette;         // F4 - toggle vignette
    bool increase_bloom;          // + - increase bloom intensity
    bool decrease_bloom;          // - - decrease bloom intensity

    // Page navigation (one-shot, cleared after processing)
    bool prev_page;   // ,/< - previous page
    bool next_page;   // ./> - next page

    // Menu navigation (one-shot, cleared after processing)
    bool menu_up;         // Up arrow - menu up
    bool menu_down;       // Down arrow - menu down
    bool menu_left;       // Left arrow - menu left / prev tab
    bool menu_right;      // Right arrow - menu right / next tab
    bool menu_select;     // Enter/Return - select option
    bool escape_pressed;  // Escape - back/menu
    bool open_journal;    // J - open journal (story mode)

    // Simulation speed controls (one-shot, cleared after processing)
    bool pause_toggle;    // Space - pause/resume simulation
    bool speed_up;        // > - increase simulation speed
    bool speed_down;      // < - decrease simulation speed
    bool show_help;       // H - toggle help overlay

    // Background settings
    uint32_t background_color;       // RGBA background for empty cells
    bool transparent_background;     // True = see-through window
    bool show_color_menu;            // Show color picker menu

    InputState()
        : mouse_left_down(false)
        , mouse_right_down(false)
        , mouse_x(0)
        , mouse_y(0)
        , view_width(WORLD_WIDTH)
        , view_height(WORLD_HEIGHT)
        , selected_material(MaterialID::Sand)
        , brush_radius(5)
        , brush_shape(BrushShape::Circle)
        , tool_mode(ToolMode::Brush)
        , shape_drawing(false)
        , shape_start_x(0)
        , shape_start_y(0)
        , filled_shapes(true)
        , clear_world(false)
        , show_debug_gui(true)
        , toggle_post_processing(false)
        , toggle_bloom(false)
        , toggle_color(false)
        , toggle_vignette(false)
        , increase_bloom(false)
        , decrease_bloom(false)
        , prev_page(false)
        , next_page(false)
        , menu_up(false)
        , menu_down(false)
        , menu_left(false)
        , menu_right(false)
        , menu_select(false)
        , escape_pressed(false)
        , open_journal(false)
        , pause_toggle(false)
        , speed_up(false)
        , speed_down(false)
        , show_help(false)
        , background_color(0xFF1A1A2E)  // Dark blue default
        , transparent_background(false)
        , show_color_menu(false) {}
};

// Platform callbacks
struct PlatformCallbacks {
    std::function<void(float delta_time)> on_update;
    std::function<void()> on_render;
};

// Platform layer - handles windowing, input, and timing
class Platform {
public:
    Platform();
    ~Platform();

    // Initialize platform and create window
    bool initialize(int32_t window_width, int32_t window_height, const char* title);

    // Run the main application loop
    void run(const PlatformCallbacks& callbacks);

    // Get input state
    const InputState& get_input_state() const { return input_state_; }
    InputState& get_input_state() { return input_state_; }

    // Get Metal view (for renderer initialization)
    void* get_metal_view();

    // Check if running
    bool is_running() const { return running_; }

private:
    bool initialized_;
    bool running_;
    InputState input_state_;

    // Cocoa objects (stored as void* to avoid Objective-C in header)
    void* window_;       // NSWindow*
    void* metal_view_;   // MTKView*
    void* app_delegate_; // AppDelegate*

    friend class PlatformImpl;
};

} // namespace PixelEngine
