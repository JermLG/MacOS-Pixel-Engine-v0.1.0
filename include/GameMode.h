#pragma once

#include <cstdint>

namespace PixelEngine {

// Game mode determines which screen/state is active
enum class GameMode : uint8_t {
    MainMenu = 0,     // Title screen with mode selection
    Sandbox = 1,      // Original mode - all materials available
    StoryMode = 2,    // Discovery mode - unlock materials progressively
    Journal = 3,      // Viewing discovery journal (overlay on StoryMode)
    Achievements = 4, // Viewing achievements screen
    Paused = 5        // Game paused (can return to menu)
};

// Main menu options
enum class MenuSelection : uint8_t {
    Sandbox = 0,
    StoryMode = 1,
    Achievements = 2,
    Quit = 3,
    COUNT = 4
};

// Journal tab selection
enum class JournalTab : uint8_t {
    Recent = 0,       // Recent discoveries (chronological)
    Materials = 1,    // All materials by category
    Recipes = 2,      // Discovered combinations
    Hints = 3,        // Hints for undiscovered combos
    COUNT = 4
};

// Tracks current game state and UI state
struct GameState {
    GameMode current_mode;
    GameMode previous_mode;       // For returning from overlays
    MenuSelection menu_selection;

    // Journal UI state
    JournalTab journal_tab;
    int journal_scroll;
    int journal_selected_category;

    // Achievement popup state
    bool showing_achievement_popup;
    float achievement_popup_timer;
    int pending_achievement_id;    // -1 if none

    // Transition effects
    bool transition_pending;
    float transition_timer;
    float transition_duration;

    // Play time tracking (for save)
    uint32_t total_play_time_seconds;
    float play_time_accumulator;

    // Simulation speed control
    float simulation_speed;       // 0.25, 0.5, 1.0, 2.0, 4.0
    bool simulation_paused;       // Manual pause (separate from menu)
    bool show_help_overlay;       // Help screen visible

    GameState()
        : current_mode(GameMode::MainMenu)
        , previous_mode(GameMode::MainMenu)
        , menu_selection(MenuSelection::Sandbox)
        , journal_tab(JournalTab::Recent)
        , journal_scroll(0)
        , journal_selected_category(0)
        , showing_achievement_popup(false)
        , achievement_popup_timer(0.0f)
        , pending_achievement_id(-1)
        , transition_pending(false)
        , transition_timer(0.0f)
        , transition_duration(0.3f)
        , total_play_time_seconds(0)
        , play_time_accumulator(0.0f)
        , simulation_speed(1.0f)
        , simulation_paused(false)
        , show_help_overlay(false) {}

    // Helper to check if we're in gameplay (not menu/overlay)
    bool is_playing() const {
        return current_mode == GameMode::Sandbox ||
               current_mode == GameMode::StoryMode;
    }

    // Helper to check if simulation should run
    bool should_simulate() const {
        return is_playing() && current_mode != GameMode::Paused && !simulation_paused;
    }
};

} // namespace PixelEngine
