#pragma once

#include "Types.h"
#include <functional>

namespace PixelEngine {

// Brush shape enum
enum class BrushShape : uint8_t {
    Circle = 0,
    Square = 1
};

// Input state
struct InputState {
    bool mouse_left_down;
    bool mouse_right_down;
    int32_t mouse_x;
    int32_t mouse_y;

    // Current material to spawn (controlled by keyboard)
    MaterialID selected_material;

    // Brush settings
    int32_t brush_radius;
    BrushShape brush_shape;

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

    InputState()
        : mouse_left_down(false)
        , mouse_right_down(false)
        , mouse_x(0)
        , mouse_y(0)
        , selected_material(MaterialID::Sand)
        , brush_radius(5)
        , brush_shape(BrushShape::Circle)
        , clear_world(false)
        , show_debug_gui(true)
        , toggle_post_processing(false)
        , toggle_bloom(false)
        , toggle_color(false)
        , toggle_vignette(false)
        , increase_bloom(false)
        , decrease_bloom(false)
        , prev_page(false)
        , next_page(false) {}
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
