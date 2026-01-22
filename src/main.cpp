#include "Platform.h"
#include "Material.h"
#include "World.h"
#include "Simulation.h"
#include "MetalRenderer.h"
#include "GameMode.h"
#include "DiscoverySystem.h"

#include <iostream>
#include <vector>
#include <cmath>

using namespace PixelEngine;

// All materials organized by category for the dropdown menus
struct MaterialEntry {
    MaterialID id;
    const char* name;
};

struct MaterialCategory {
    const char* name;
    const MaterialEntry* materials;
    int count;
    bool is_open;  // Runtime state - whether dropdown is expanded
};

// Category: Basic
static const MaterialEntry BASIC_MATERIALS[] = {
    {MaterialID::Sand, "Sand"},
    {MaterialID::Water, "Water"},
    {MaterialID::Stone, "Stone"},
    {MaterialID::Steam, "Steam"},
    {MaterialID::Oil, "Oil"},
    {MaterialID::Fire, "Fire"},
    {MaterialID::Wood, "Wood"},
    {MaterialID::Acid, "Acid"},
    {MaterialID::Lava, "Lava"},
    {MaterialID::Grass, "Grass"},
    {MaterialID::Bedrock, "Bedrock"},
    {MaterialID::Ceramic, "Ceramic"},
    {MaterialID::Granite, "Granite"},
    {MaterialID::Marble, "Marble"},
    {MaterialID::Sandstone, "Sandstone"},
    {MaterialID::Limestone, "Limestone"},
    {MaterialID::Slate, "Slate"},
    {MaterialID::Basalt, "Basalt"},
    {MaterialID::Quartz_Block, "Quartz"},
    {MaterialID::Soil, "Soil"},
};

// Category: Powders
static const MaterialEntry POWDER_MATERIALS[] = {
    {MaterialID::Ash, "Ash"},
    {MaterialID::Dirt, "Dirt"},
    {MaterialID::Gravel, "Gravel"},
    {MaterialID::Snow, "Snow"},
    {MaterialID::Gunpowder, "Gunpowder"},
    {MaterialID::Salt, "Salt"},
    {MaterialID::Coal, "Coal"},
    {MaterialID::Rust, "Rust"},
    {MaterialID::Sawdust, "Sawdust"},
    {MaterialID::Glass_Powder, "Glass Pwdr"},
    {MaterialID::Thermite_Powder, "Thermite P"},
    {MaterialID::Sugar, "Sugar"},
    {MaterialID::Iron_Filings, "Iron Files"},
    {MaterialID::Chalk, "Chalk"},
    {MaterialID::Calcium, "Calcium"},
    {MaterialID::Flour, "Flour"},
    {MaterialID::Sulfur, "Sulfur"},
    {MaterialID::Cement, "Cement"},
    {MaterialID::Fertilizer, "Fertilizer"},
    {MaterialID::Volcanic_Ash, "Volc Ash"},
};

// Category: Liquids
static const MaterialEntry LIQUID_MATERIALS[] = {
    {MaterialID::Honey, "Honey"},
    {MaterialID::Mud, "Mud"},
    {MaterialID::Blood, "Blood"},
    {MaterialID::Poison, "Poison"},
    {MaterialID::Slime, "Slime"},
    {MaterialID::Milk, "Milk"},
    {MaterialID::Alcohol, "Alcohol"},
    {MaterialID::Mercury, "Mercury"},
    {MaterialID::Petrol, "Petrol"},
    {MaterialID::Glue, "Glue"},
    {MaterialID::Tar, "Tar"},
    {MaterialID::Juice, "Juice"},
    {MaterialID::Sap, "Sap"},
    {MaterialID::Bleach, "Bleach"},
    {MaterialID::Ink, "Ink"},
    {MaterialID::Brine, "Brine"},
    {MaterialID::Coffee, "Coffee"},
    {MaterialID::Soap, "Soap"},
    {MaterialID::Paint, "Paint"},
    {MaterialID::Sewage, "Sewage"},
};

// Category: Gases
static const MaterialEntry GAS_MATERIALS[] = {
    {MaterialID::Smoke, "Smoke"},
    {MaterialID::Toxic_Gas, "Toxic Gas"},
    {MaterialID::Hydrogen, "Hydrogen"},
    {MaterialID::Helium, "Helium"},
    {MaterialID::Methane, "Methane"},
    {MaterialID::Spark, "Spark"},
    {MaterialID::Plasma, "Plasma"},
    {MaterialID::Dust, "Dust"},
    {MaterialID::Spore, "Spore"},
    {MaterialID::Confetti, "Confetti"},
    {MaterialID::Chlorine, "Chlorine"},
    {MaterialID::Liquid_Nitrogen, "Liq Nitro"},
    {MaterialID::Oxygen, "Oxygen"},
    {MaterialID::Ammonia, "Ammonia"},
    {MaterialID::Carbon_Dioxide, "CO2"},
    {MaterialID::Nitrous, "Nitrous"},
    {MaterialID::Steam_Hot, "Hot Steam"},
    {MaterialID::Miasma, "Miasma"},
    {MaterialID::Pheromone, "Pheromone"},
    {MaterialID::Nerve_Gas, "Nerve Gas"},
};

// Category: Solids
static const MaterialEntry SOLID_MATERIALS[] = {
    {MaterialID::Metal, "Metal"},
    {MaterialID::Gold, "Gold"},
    {MaterialID::Ice, "Ice"},
    {MaterialID::Glass, "Glass"},
    {MaterialID::Brick, "Brick"},
    {MaterialID::Obsidian, "Obsidian"},
    {MaterialID::Diamond, "Diamond"},
    {MaterialID::Copper, "Copper"},
    {MaterialID::Rubber, "Rubber"},
    {MaterialID::Concrete, "Concrete"},
    {MaterialID::Titanium, "Titanium"},
    {MaterialID::Clay, "Clay"},
    {MaterialID::Charcoal, "Charcoal"},
    {MaterialID::Silver, "Silver"},
    {MaterialID::Platinum, "Platinum"},
    {MaterialID::Lead, "Lead"},
    {MaterialID::Tin, "Tin"},
    {MaterialID::Zinc, "Zinc"},
    {MaterialID::Bronze, "Bronze"},
    {MaterialID::Steel, "Steel"},
};

// Category: Organic
static const MaterialEntry ORGANIC_MATERIALS[] = {
    {MaterialID::Leaf, "Leaf"},
    {MaterialID::Moss, "Moss"},
    {MaterialID::Vine, "Vine"},
    {MaterialID::Fungus, "Fungus"},
    {MaterialID::Seed, "Seed"},
    {MaterialID::Flower, "Flower"},
    {MaterialID::Algae, "Algae"},
    {MaterialID::Coral, "Coral"},
    {MaterialID::Wax, "Wax"},
    {MaterialID::Flesh, "Flesh"},
    {MaterialID::Bamboo, "Bamboo"},
    {MaterialID::Honeycomb, "Honeycomb"},
    {MaterialID::Bone, "Bone"},
    {MaterialID::Pollen, "Pollen"},
    {MaterialID::Root, "Root"},
    {MaterialID::Bark, "Bark"},
    {MaterialID::Fruit, "Fruit"},
    {MaterialID::Egg, "Egg"},
    {MaterialID::Web, "Web"},
    {MaterialID::Mucus, "Mucus"},
};

// Category: Special
static const MaterialEntry SPECIAL_MATERIALS[] = {
    {MaterialID::Life, "Life"},       // Spawns people when landing safely
    {MaterialID::Clone, "Clone"},
    {MaterialID::Void, "Void"},
    {MaterialID::Fuse, "Fuse"},
    {MaterialID::TNT, "TNT"},
    {MaterialID::C4, "C4"},
    {MaterialID::Firework, "Firework"},
    {MaterialID::Lightning, "Lightning"},
    {MaterialID::Portal_In, "Portal In"},
    {MaterialID::Portal_Out, "Portal Out"},
    {MaterialID::Napalm, "Napalm"},
    {MaterialID::Thermite, "Thermite"},
    {MaterialID::Bomb, "Bomb"},
    {MaterialID::Nuke, "Nuke"},
    {MaterialID::Laser, "Laser"},
    {MaterialID::Black_Hole, "Black Hole"},
    {MaterialID::White_Hole, "White Hole"},
    {MaterialID::Acid_Gas, "Acid Gas"},
    {MaterialID::Ice_Bomb, "Ice Bomb"},
    {MaterialID::Fire_Bomb, "Fire Bomb"},
};

// Category: Fantasy
static const MaterialEntry FANTASY_MATERIALS[] = {
    {MaterialID::Magic, "Magic"},
    {MaterialID::Crystal, "Crystal"},
    {MaterialID::Ectoplasm, "Ectoplasm"},
    {MaterialID::Antimatter, "Antimatter"},
    {MaterialID::Fairy_Dust, "Fairy Dust"},
    {MaterialID::Dragon_Fire, "Dragon Fire"},
    {MaterialID::Frost, "Frost"},
    {MaterialID::Ember, "Ember"},
    {MaterialID::Stardust, "Stardust"},
    {MaterialID::Void_Dust, "Void Dust"},
    {MaterialID::Mana, "Mana"},
    {MaterialID::Mirage, "Mirage"},
    {MaterialID::Holy_Water, "Holy Water"},
    {MaterialID::Cursed, "Cursed"},
    {MaterialID::Blessed, "Blessed"},
    {MaterialID::Soul, "Soul"},
    {MaterialID::Spirit, "Spirit"},
    {MaterialID::Aether, "Aether"},
    {MaterialID::Nether, "Nether"},
    {MaterialID::Phoenix_Ash, "PhoenixAsh"},
};

#define ARRAY_COUNT(arr) (sizeof(arr) / sizeof(arr[0]))

static const int NUM_CATEGORIES = 8;

// Global pointer for discovery system callbacks (C-style function pointers can't capture)
static DiscoverySystem* g_discovery_system_ptr = nullptr;
static uint32_t g_frame_counter = 0;

// Callback functions for Material.cpp discovery hooks
static void discovery_callback(MaterialID mat_a, MaterialID mat_b,
                               MaterialID result_a, MaterialID result_b,
                               uint32_t frame_number) {
    if (g_discovery_system_ptr) {
        g_discovery_system_ptr->on_combination_occurred(mat_a, mat_b, result_a, result_b, frame_number);
    }
}

static bool material_unlock_checker(MaterialID id) {
    if (g_discovery_system_ptr) {
        return g_discovery_system_ptr->is_material_unlocked(id);
    }
    return true;  // If no discovery system, allow all materials
}

// Safety net callback: unlock any material that spawns in the world
// This catches materials created by reactions we might have missed in the combination list
static void material_spawn_callback(MaterialID id) {
    if (g_discovery_system_ptr && !g_discovery_system_ptr->is_material_unlocked(id)) {
        // Unlock and show popup for auto-discovered materials
        g_discovery_system_ptr->unlock_with_popup(id, g_frame_counter);
    }
}

// Application class - ties everything together
class PixelEngineApp {
public:
    PixelEngineApp()
        : material_system_()
        , world_(WORLD_WIDTH, WORLD_HEIGHT, material_system_)
        , simulation_(world_)
        , renderer_()
        , platform_()
        , accumulator_(0.0f)
        , frame_count_(0)
        , fps_timer_(0.0f)
        , current_fps_(60.0f)
        , active_cells_display_(0)
        , open_category_(-1)
        , scroll_offset_(0)
        , favorites_count_(0) {

        // Initialize favorites with common starting materials
        favorites_[favorites_count_++] = MaterialID::Sand;
        favorites_[favorites_count_++] = MaterialID::Water;
        favorites_[favorites_count_++] = MaterialID::Stone;
        favorites_[favorites_count_++] = MaterialID::Fire;

        pixel_buffer_.resize(WORLD_WIDTH * WORLD_HEIGHT);

        // Initialize categories array
        categories_[0] = {"Basic", BASIC_MATERIALS, (int)ARRAY_COUNT(BASIC_MATERIALS)};
        categories_[1] = {"Powders", POWDER_MATERIALS, (int)ARRAY_COUNT(POWDER_MATERIALS)};
        categories_[2] = {"Liquids", LIQUID_MATERIALS, (int)ARRAY_COUNT(LIQUID_MATERIALS)};
        categories_[3] = {"Gases", GAS_MATERIALS, (int)ARRAY_COUNT(GAS_MATERIALS)};
        categories_[4] = {"Solids", SOLID_MATERIALS, (int)ARRAY_COUNT(SOLID_MATERIALS)};
        categories_[5] = {"Organic", ORGANIC_MATERIALS, (int)ARRAY_COUNT(ORGANIC_MATERIALS)};
        categories_[6] = {"Special", SPECIAL_MATERIALS, (int)ARRAY_COUNT(SPECIAL_MATERIALS)};
        categories_[7] = {"Fantasy", FANTASY_MATERIALS, (int)ARRAY_COUNT(FANTASY_MATERIALS)};
    }

    bool initialize() {
        std::cout << "Initializing Pixel Engine...\n";

        // Initialize platform (window)
        if (!platform_.initialize(WORLD_WIDTH, WORLD_HEIGHT, "Pixel Engine - Falling Sand")) {
            std::cerr << "Failed to initialize platform\n";
            return false;
        }

        // Initialize renderer
        if (!renderer_.initialize(platform_.get_metal_view(), WORLD_WIDTH, WORLD_HEIGHT)) {
            std::cerr << "Failed to initialize renderer\n";
            return false;
        }

        // Set up initial world (border walls)
        create_initial_world();

        // Initialize discovery system with combination data from Material.cpp
        set_combination_data(get_combinations_data(), get_combinations_count());

        std::cout << "Engine initialized successfully!\n";
        std::cout << "World size: " << WORLD_WIDTH << "x" << WORLD_HEIGHT << "\n";
        std::cout << "Chunk size: " << CHUNK_SIZE << "x" << CHUNK_SIZE << "\n";
        std::cout << "Total chunks: " << world_.get_chunks_wide() << "x" << world_.get_chunks_high() << "\n";
        std::cout << "\nControls:\n";
        std::cout << "  Left Mouse: Place material\n";
        std::cout << "  Right Mouse: Erase (place empty)\n";
        std::cout << "  1: Sand    2: Water   3: Stone\n";
        std::cout << "  4: Steam   5: Oil     6: Fire\n";
        std::cout << "  7: Wood    8: Acid    9: Lava\n";
        std::cout << "  0: Ash     G: Grass   S: Smoke\n";
        std::cout << "  P: Person\n";
        std::cout << "  [/]: Brush size   B: Brush shape\n";
        std::cout << "  C: Clear World    Tab: Toggle GUI\n";
        std::cout << "\nMaterial Pages (80 materials!):\n";
        std::cout << "  ,/<: Previous page  ./>: Next page\n";
        std::cout << "  Click palette to select material\n";
        std::cout << "\nPost-Processing:\n";
        std::cout << "  F1: Toggle all effects\n";
        std::cout << "  F2: Toggle bloom   F3: Toggle color\n";
        std::cout << "  F4: Toggle vignette\n";
        std::cout << "  +/-: Adjust bloom intensity\n";
        std::cout << "\n  Q: Quit\n";

        return true;
    }

    void run() {
        PlatformCallbacks callbacks;
        callbacks.on_update = [this](float delta_time) { this->update(delta_time); };
        callbacks.on_render = [this]() { this->render(); };

        platform_.run(callbacks);
    }

private:
    MaterialSystem material_system_;
    World world_;
    Simulation simulation_;
    MetalRenderer renderer_;
    Platform platform_;

    std::vector<uint32_t> pixel_buffer_;

    float accumulator_;
    uint64_t frame_count_;
    float fps_timer_;
    float current_fps_;
    uint32_t active_cells_display_;

    // Dropdown UI state
    struct CategoryState {
        const char* name;
        const MaterialEntry* materials;
        int count;
    };
    CategoryState categories_[NUM_CATEGORIES];
    int open_category_;  // -1 = all closed, 0-7 = which category is open
    int scroll_offset_;  // For scrolling within a dropdown

    // Game mode and discovery system
    GameState game_state_;
    DiscoverySystem discovery_system_;

    // Material favorites bar (max 6 slots)
    static constexpr int MAX_FAVORITES = 6;
    MaterialID favorites_[MAX_FAVORITES];
    int favorites_count_;

    // Check if material is in favorites
    bool is_favorite(MaterialID mat) const {
        for (int i = 0; i < favorites_count_; i++) {
            if (favorites_[i] == mat) return true;
        }
        return false;
    }

    // Add material to favorites (returns false if already full or already favorite)
    bool add_favorite(MaterialID mat) {
        if (favorites_count_ >= MAX_FAVORITES) return false;
        if (is_favorite(mat)) return false;
        favorites_[favorites_count_++] = mat;
        std::cout << "Added " << find_material_name(mat) << " to favorites\n";
        return true;
    }

    // Remove material from favorites
    bool remove_favorite(MaterialID mat) {
        for (int i = 0; i < favorites_count_; i++) {
            if (favorites_[i] == mat) {
                // Shift remaining favorites down
                for (int j = i; j < favorites_count_ - 1; j++) {
                    favorites_[j] = favorites_[j + 1];
                }
                favorites_count_--;
                std::cout << "Removed " << find_material_name(mat) << " from favorites\n";
                return true;
            }
        }
        return false;
    }

    // Toggle favorite status
    void toggle_favorite(MaterialID mat) {
        if (is_favorite(mat)) {
            remove_favorite(mat);
        } else {
            add_favorite(mat);
        }
    }

    void create_initial_world() {
        // Empty world - no borders
        // Materials will fall off edges and wrap or disappear based on simulation rules
    }

    void update(float delta_time) {
        // Handle input based on current game mode
        handle_input();

        // Only run simulation when playing
        if (game_state_.should_simulate()) {
            // Track play time in story mode
            if (game_state_.current_mode == GameMode::StoryMode) {
                game_state_.play_time_accumulator += delta_time;
                if (game_state_.play_time_accumulator >= 1.0f) {
                    game_state_.total_play_time_seconds++;
                    game_state_.play_time_accumulator -= 1.0f;
                }
            }

            // Fixed timestep simulation (with speed multiplier)
            accumulator_ += delta_time * game_state_.simulation_speed;

            // Cap accumulator to prevent spiral of death - max 2 updates per frame
            float effective_timestep = FIXED_TIMESTEP;
            if (accumulator_ > effective_timestep * 2.0f) {
                accumulator_ = effective_timestep * 2.0f;
            }

            // Update simulation with fixed timestep (max 2 per frame)
            int updates = 0;
            int max_updates = (game_state_.simulation_speed >= 2.0f) ? 4 : 2;
            while (accumulator_ >= effective_timestep && updates < max_updates) {
                // Update frame counter for discovery system
                g_frame_counter++;
                set_current_frame(g_frame_counter);

                simulation_.update();
                accumulator_ -= effective_timestep;
                ++updates;
            }
        }

        // Update achievement popup timer
        if (game_state_.showing_achievement_popup) {
            game_state_.achievement_popup_timer -= delta_time;
            if (game_state_.achievement_popup_timer <= 0) {
                game_state_.showing_achievement_popup = false;
            }
        }

        // Check for new discoveries in story mode
        if (game_state_.current_mode == GameMode::StoryMode) {
            check_discoveries();
        }

        // FPS counter (always update)
        ++frame_count_;
        fps_timer_ += delta_time;
        if (fps_timer_ >= 1.0f) {
            current_fps_ = static_cast<float>(frame_count_) / fps_timer_;
            active_cells_display_ = simulation_.get_updated_cells();

            // Only print FPS during gameplay
            if (game_state_.is_playing()) {
                std::cout << "FPS: " << frame_count_
                          << " | Active chunks: " << simulation_.get_active_chunks()
                          << " | Updated cells: " << simulation_.get_updated_cells()
                          << "\n";
            }
            frame_count_ = 0;
            fps_timer_ = 0.0f;
        }
    }

    void check_discoveries() {
        // Process any new discoveries from the discovery system
        while (discovery_system_.has_new_discovery()) {
            DiscoveryEntry entry = discovery_system_.pop_new_discovery();

            // Show popup for new material unlocks
            if (entry.material_unlocked != MaterialID::Empty) {
                // Queue achievement popup
                game_state_.showing_achievement_popup = true;
                game_state_.achievement_popup_timer = 3.0f;  // Show for 3 seconds
                game_state_.pending_achievement_id = static_cast<int>(entry.material_unlocked);

                std::cout << "NEW DISCOVERY: Unlocked material!\n";
            }
        }
    }

    // UI Layout constants
    static constexpr int UI_PANEL_X = WORLD_WIDTH - 145;
    static constexpr int UI_PANEL_WIDTH = 140;
    static constexpr int UI_HEADER_HEIGHT = 18;
    static constexpr int UI_ITEM_HEIGHT = 16;
    static constexpr int UI_SWATCH_SIZE = 12;
    static constexpr int UI_PADDING = 4;

    // Calculate total UI height for hit testing
    int get_ui_total_height() const {
        int height = 30;  // Title + spacing
        height += 45;     // Favorites bar (label + bar + spacing)
        for (int i = 0; i < NUM_CATEGORIES; i++) {
            height += UI_HEADER_HEIGHT + 2;  // Category header
            if (open_category_ == i) {
                height += categories_[i].count * UI_ITEM_HEIGHT;  // Expanded items
            }
        }
        height += 30;  // Selected material display at bottom
        return height;
    }

    // Helper to place a single cell of material
    void place_material_cell(int32_t x, int32_t y, MaterialID material) {
        if (world_.in_bounds(x, y)) {
            Cell& cell = world_.get_cell(x, y);
            cell.flags = 0;
            cell.velocity_y = 0;
            world_.set_material(x, y, material);
        }
    }

    // Draw a line of material using Bresenham's algorithm with thickness
    void draw_line(int32_t x0, int32_t y0, int32_t x1, int32_t y1, MaterialID material, int32_t thickness = 3) {
        int32_t dx = std::abs(x1 - x0);
        int32_t dy = std::abs(y1 - y0);
        int32_t sx = (x0 < x1) ? 1 : -1;
        int32_t sy = (y0 < y1) ? 1 : -1;
        int32_t err = dx - dy;

        while (true) {
            // Draw a thick point at current position
            for (int32_t ty = -thickness/2; ty <= thickness/2; ty++) {
                for (int32_t tx = -thickness/2; tx <= thickness/2; tx++) {
                    place_material_cell(x0 + tx, y0 + ty, material);
                }
            }

            if (x0 == x1 && y0 == y1) break;

            int32_t e2 = 2 * err;
            if (e2 > -dy) {
                err -= dy;
                x0 += sx;
            }
            if (e2 < dx) {
                err += dx;
                y0 += sy;
            }
        }
    }

    // Draw a rectangle (filled or outline)
    void draw_rectangle(int32_t x0, int32_t y0, int32_t x1, int32_t y1, MaterialID material, bool filled) {
        int32_t left = std::min(x0, x1);
        int32_t right = std::max(x0, x1);
        int32_t top = std::min(y0, y1);
        int32_t bottom = std::max(y0, y1);

        if (filled) {
            for (int32_t y = top; y <= bottom; y++) {
                for (int32_t x = left; x <= right; x++) {
                    place_material_cell(x, y, material);
                }
            }
        } else {
            // Draw outline with thickness
            int32_t thickness = 2;
            // Top and bottom edges
            for (int32_t x = left; x <= right; x++) {
                for (int32_t t = 0; t < thickness; t++) {
                    place_material_cell(x, top + t, material);
                    place_material_cell(x, bottom - t, material);
                }
            }
            // Left and right edges
            for (int32_t y = top; y <= bottom; y++) {
                for (int32_t t = 0; t < thickness; t++) {
                    place_material_cell(left + t, y, material);
                    place_material_cell(right - t, y, material);
                }
            }
        }
    }

    // Draw an ellipse (filled or outline) using midpoint algorithm
    void draw_ellipse(int32_t x0, int32_t y0, int32_t x1, int32_t y1, MaterialID material, bool filled) {
        int32_t cx = (x0 + x1) / 2;
        int32_t cy = (y0 + y1) / 2;
        int32_t rx = std::abs(x1 - x0) / 2;
        int32_t ry = std::abs(y1 - y0) / 2;

        if (rx == 0 || ry == 0) {
            // Degenerate to line
            draw_line(x0, y0, x1, y1, material, 2);
            return;
        }

        if (filled) {
            // Filled ellipse - scan each row
            for (int32_t y = -ry; y <= ry; y++) {
                // Calculate x extent for this row using ellipse equation
                // (x/rx)^2 + (y/ry)^2 = 1
                // x = rx * sqrt(1 - (y/ry)^2)
                float yf = static_cast<float>(y) / static_cast<float>(ry);
                float xf = std::sqrt(1.0f - yf * yf);
                int32_t xExtent = static_cast<int32_t>(xf * rx);

                for (int32_t x = -xExtent; x <= xExtent; x++) {
                    place_material_cell(cx + x, cy + y, material);
                }
            }
        } else {
            // Outline ellipse using parametric approach
            int32_t thickness = 2;
            int steps = std::max(rx, ry) * 4;
            for (int i = 0; i < steps; i++) {
                float angle = 2.0f * 3.14159f * i / steps;
                int32_t x = cx + static_cast<int32_t>(rx * std::cos(angle));
                int32_t y = cy + static_cast<int32_t>(ry * std::sin(angle));
                for (int32_t ty = -thickness/2; ty <= thickness/2; ty++) {
                    for (int32_t tx = -thickness/2; tx <= thickness/2; tx++) {
                        place_material_cell(x + tx, y + ty, material);
                    }
                }
            }
        }
    }

    // Flood fill algorithm
    void flood_fill(int32_t x, int32_t y, MaterialID fill_material) {
        if (!world_.in_bounds(x, y)) return;

        MaterialID target_material = world_.get_material(x, y);
        if (target_material == fill_material) return;  // Already the fill color

        // Use a stack-based flood fill to avoid recursion depth issues
        std::vector<std::pair<int32_t, int32_t>> stack;
        stack.push_back({x, y});

        int max_fill = 50000;  // Safety limit
        int filled = 0;

        while (!stack.empty() && filled < max_fill) {
            auto [cx, cy] = stack.back();
            stack.pop_back();

            if (!world_.in_bounds(cx, cy)) continue;
            if (world_.get_material(cx, cy) != target_material) continue;

            place_material_cell(cx, cy, fill_material);
            filled++;

            // Add neighbors
            stack.push_back({cx + 1, cy});
            stack.push_back({cx - 1, cy});
            stack.push_back({cx, cy + 1});
            stack.push_back({cx, cy - 1});
        }
    }

    // Returns: 0 = no click in UI, 1 = material selected, 2 = category toggled, 3 = favorite clicked
    int check_dropdown_click(int32_t mx, int32_t my, MaterialID& clicked_material, bool is_right_click = false) {
        // Check if click is within the UI panel area
        if (mx < UI_PANEL_X - 5 || mx > WORLD_WIDTH) {
            return 0;  // Not in UI
        }

        bool story_mode = (game_state_.current_mode == GameMode::StoryMode);
        int x = UI_PANEL_X;
        int y = 10;  // Start position

        // Skip title area (Story Mode has extra line for Journal hint)
        if (story_mode) {
            y += 30;  // Title + progress + Journal hint
        } else {
            y += 15;
        }

        // ===== FAVORITES BAR CLICK =====
        // Skip "Favorites" label
        y += 12;

        // Check favorites bar area
        const int FAV_SWATCH_SIZE = 18;
        const int FAV_SPACING = 22;
        const int FAV_BAR_HEIGHT = 28;

        if (my >= y && my < y + FAV_BAR_HEIGHT) {
            // Check which slot was clicked
            int fav_x = x + 3;
            int fav_y = y + 5;
            for (int i = 0; i < MAX_FAVORITES; i++) {
                if (mx >= fav_x && mx < fav_x + FAV_SWATCH_SIZE &&
                    my >= fav_y && my < fav_y + FAV_SWATCH_SIZE) {
                    if (i < favorites_count_) {
                        if (is_right_click) {
                            // Right-click removes from favorites
                            remove_favorite(favorites_[i]);
                            return 3;  // Favorite modified
                        } else {
                            // Left-click selects the material
                            MaterialID mat_id = favorites_[i];
                            // In Story Mode, only allow selecting unlocked materials
                            if (story_mode && !discovery_system_.is_material_unlocked(mat_id)) {
                                return 0;  // Can't select locked material
                            }
                            clicked_material = mat_id;
                            return 1;  // Material selected
                        }
                    }
                    return 0;  // Empty slot clicked
                }
                fav_x += FAV_SPACING;
            }
        }

        y += FAV_BAR_HEIGHT + 5;

        // Check each category
        for (int cat = 0; cat < NUM_CATEGORIES; cat++) {
            // Category header click area
            if (my >= y && my < y + UI_HEADER_HEIGHT) {
                if (!is_right_click) {
                    // Toggle this category
                    if (open_category_ == cat) {
                        open_category_ = -1;  // Close it
                    } else {
                        open_category_ = cat;  // Open it (closes others)
                    }
                    scroll_offset_ = 0;
                    return 2;  // Category toggled
                }
                return 0;
            }
            y += UI_HEADER_HEIGHT + 2;

            // If this category is open, check material items
            if (open_category_ == cat) {
                for (int i = 0; i < categories_[cat].count; i++) {
                    if (my >= y && my < y + UI_ITEM_HEIGHT) {
                        MaterialID mat_id = categories_[cat].materials[i].id;
                        // In Story Mode, only allow selecting unlocked materials
                        if (story_mode && !discovery_system_.is_material_unlocked(mat_id)) {
                            return 0;  // Can't select locked material
                        }
                        if (is_right_click) {
                            // Right-click toggles favorite
                            toggle_favorite(mat_id);
                            return 3;  // Favorite toggled
                        }
                        clicked_material = mat_id;
                        return 1;  // Material selected
                    }
                    y += UI_ITEM_HEIGHT;
                }
            }
        }

        return 0;  // Click was in UI but not on any interactive element
    }

    // =========================================================================
    // MODE-SPECIFIC INPUT HANDLERS
    // =========================================================================

    void handle_menu_input(InputState& input) {
        // Navigate menu up
        if (input.menu_up) {
            int sel = static_cast<int>(game_state_.menu_selection);
            if (sel > 0) sel--;
            else sel = static_cast<int>(MenuSelection::COUNT) - 1;  // Wrap around
            game_state_.menu_selection = static_cast<MenuSelection>(sel);
            input.menu_up = false;
        }

        // Navigate menu down
        if (input.menu_down) {
            int sel = static_cast<int>(game_state_.menu_selection);
            if (sel < static_cast<int>(MenuSelection::COUNT) - 1) sel++;
            else sel = 0;  // Wrap around
            game_state_.menu_selection = static_cast<MenuSelection>(sel);
            input.menu_down = false;
        }

        // Select menu option
        if (input.menu_select) {
            switch (game_state_.menu_selection) {
                case MenuSelection::Sandbox:
                    game_state_.current_mode = GameMode::Sandbox;
                    // Disable story mode hooks
                    set_story_mode(false);
                    set_discovery_callback(nullptr);
                    set_material_unlock_checker(nullptr);
                    world_.set_material_spawn_callback(nullptr);
                    g_discovery_system_ptr = nullptr;
                    world_.clear_world();
                    create_initial_world();
                    std::cout << "Starting SANDBOX mode - all materials unlocked!\n";
                    break;

                case MenuSelection::StoryMode:
                    game_state_.current_mode = GameMode::StoryMode;
                    discovery_system_.reset_to_starter_set();
                    // Enable story mode hooks
                    g_discovery_system_ptr = &discovery_system_;
                    set_story_mode(true);
                    set_discovery_callback(discovery_callback);
                    set_material_unlock_checker(material_unlock_checker);
                    // Safety net: unlock any material that spawns in the world
                    world_.set_material_spawn_callback(material_spawn_callback);
                    world_.clear_world();
                    create_initial_world();
                    std::cout << "Starting STORY mode - discover materials by combining!\n";
                    std::cout << "Press [J] to open the Discovery Journal\n";
                    break;

                case MenuSelection::Achievements:
                    game_state_.current_mode = GameMode::Achievements;
                    break;

                case MenuSelection::Quit:
                    std::cout << "Goodbye!\n";
                    exit(0);
                    break;

                default:
                    break;
            }
            input.menu_select = false;
        }

        // Clear any other input flags that might have been set
        input.escape_pressed = false;
        input.open_journal = false;
    }

    void handle_journal_input(InputState& input) {
        bool sandbox_mode = (game_state_.previous_mode == GameMode::Sandbox);
        int max_tabs = sandbox_mode ? 2 : 4;  // Sandbox: Recipes, Materials. Story: all 4 tabs

        // Close journal
        if (input.escape_pressed) {
            game_state_.current_mode = game_state_.previous_mode;
            input.escape_pressed = false;
            return;
        }

        // Tab navigation with left/right arrows or ,/. keys
        bool go_prev = input.prev_page || input.menu_left;
        bool go_next = input.next_page || input.menu_right;

        // Check for tab clicks
        int panel_x = 30;
        int panel_y = 30;
        int tab_x = panel_x + 20;
        int tab_y = panel_y + 30;
        int tab_height = 20;
        int tab_width = 80;

        static bool was_clicking = false;
        bool is_clicking = input.mouse_left_down;

        if (is_clicking && !was_clicking) {
            // Check if click is in tab area
            if (input.mouse_y >= tab_y && input.mouse_y < tab_y + tab_height) {
                for (int i = 0; i < max_tabs; i++) {
                    int tx = tab_x + i * 85;
                    if (input.mouse_x >= tx && input.mouse_x < tx + tab_width) {
                        game_state_.journal_tab = static_cast<JournalTab>(i);
                        game_state_.journal_scroll = 0;
                        break;
                    }
                }
            }
        }
        was_clicking = is_clicking;

        if (go_prev) {
            int tab = static_cast<int>(game_state_.journal_tab);
            if (tab > 0) tab--;
            else tab = max_tabs - 1;
            game_state_.journal_tab = static_cast<JournalTab>(tab);
            game_state_.journal_scroll = 0;  // Reset scroll when changing tabs
            input.prev_page = false;
            input.menu_left = false;
        }
        if (go_next) {
            int tab = static_cast<int>(game_state_.journal_tab);
            if (tab < max_tabs - 1) tab++;
            else tab = 0;
            game_state_.journal_tab = static_cast<JournalTab>(tab);
            game_state_.journal_scroll = 0;
            input.next_page = false;
            input.menu_right = false;
        }

        // Up/Down for scrolling content or changing categories
        bool on_materials_tab = (game_state_.journal_tab == JournalTab::Materials) ||
                                (sandbox_mode && game_state_.journal_tab == JournalTab::Materials);

        if (input.menu_up) {
            if (on_materials_tab) {
                // Navigate categories
                if (game_state_.journal_selected_category > 0) {
                    game_state_.journal_selected_category--;
                    game_state_.journal_scroll = 0;
                }
            } else {
                // Scroll up (for recipes list)
                int items_per_page = (WORLD_HEIGHT - 60 - 90) / 18;
                if (game_state_.journal_scroll >= items_per_page) {
                    game_state_.journal_scroll -= items_per_page;
                } else {
                    game_state_.journal_scroll = 0;
                }
            }
            input.menu_up = false;
        }
        if (input.menu_down) {
            if (on_materials_tab) {
                // Navigate categories
                if (game_state_.journal_selected_category < NUM_CATEGORIES - 1) {
                    game_state_.journal_selected_category++;
                    game_state_.journal_scroll = 0;
                }
            } else {
                // Scroll down
                int items_per_page = (WORLD_HEIGHT - 60 - 90) / 18;
                int total_items = sandbox_mode ? get_combinations_count() :
                                  static_cast<int>(discovery_system_.get_all_discoveries().size());
                if (game_state_.journal_scroll + items_per_page < total_items) {
                    game_state_.journal_scroll += items_per_page;
                }
            }
            input.menu_down = false;
        }

        input.open_journal = false;
        input.menu_select = false;
    }

    void handle_achievements_input(InputState& input) {
        // Return to menu
        if (input.escape_pressed || input.menu_select) {
            game_state_.current_mode = GameMode::MainMenu;
            input.escape_pressed = false;
            input.menu_select = false;
        }
    }

    void handle_pause_input(InputState& input) {
        // Resume game
        if (input.escape_pressed || input.menu_select) {
            game_state_.current_mode = game_state_.previous_mode;
            input.escape_pressed = false;
            input.menu_select = false;
        }
    }

    // =========================================================================
    // MAIN GAMEPLAY INPUT HANDLER
    // =========================================================================

    void handle_input() {
        auto& input = const_cast<InputState&>(platform_.get_input_state());

        // Handle input based on current game mode
        switch (game_state_.current_mode) {
            case GameMode::MainMenu:
                handle_menu_input(input);
                return;  // Don't process other input in menu

            case GameMode::Journal:
                handle_journal_input(input);
                return;

            case GameMode::Achievements:
                handle_achievements_input(input);
                return;

            case GameMode::Paused:
                handle_pause_input(input);
                return;

            case GameMode::Sandbox:
            case GameMode::StoryMode:
                // Continue to gameplay input handling below
                break;
        }

        // Handle escape key during gameplay
        if (input.escape_pressed) {
            game_state_.previous_mode = game_state_.current_mode;
            game_state_.current_mode = GameMode::MainMenu;
            input.escape_pressed = false;
            return;
        }

        // Handle journal key (J) in both story mode and sandbox mode
        if (input.open_journal && (game_state_.current_mode == GameMode::StoryMode ||
                                   game_state_.current_mode == GameMode::Sandbox)) {
            game_state_.previous_mode = game_state_.current_mode;
            game_state_.current_mode = GameMode::Journal;
            game_state_.journal_tab = JournalTab::Recent;  // Reset to first tab
            game_state_.journal_scroll = 0;
            input.open_journal = false;
            return;
        }

        // Handle clear world request
        if (input.clear_world) {
            world_.clear_world();
            create_initial_world();
            input.clear_world = false;  // Clear the flag
            std::cout << "World cleared!\n";
        }

        // Handle post-processing toggles
        if (input.toggle_post_processing) {
            renderer_.set_post_processing_enabled(!renderer_.is_post_processing_enabled());
            input.toggle_post_processing = false;
            std::cout << "Post-processing: " << (renderer_.is_post_processing_enabled() ? "ON" : "OFF") << "\n";
        }
        if (input.toggle_bloom) {
            renderer_.toggle_effect(EFFECT_BLOOM);
            input.toggle_bloom = false;
            std::cout << "Bloom: " << (renderer_.is_effect_enabled(EFFECT_BLOOM) ? "ON" : "OFF") << "\n";
        }
        if (input.toggle_color) {
            renderer_.toggle_effect(EFFECT_COLOR);
            input.toggle_color = false;
            std::cout << "Color grading: " << (renderer_.is_effect_enabled(EFFECT_COLOR) ? "ON" : "OFF") << "\n";
        }
        if (input.toggle_vignette) {
            renderer_.toggle_effect(EFFECT_VIGNETTE);
            input.toggle_vignette = false;
            std::cout << "Vignette: " << (renderer_.is_effect_enabled(EFFECT_VIGNETTE) ? "ON" : "OFF") << "\n";
        }
        if (input.increase_bloom) {
            float intensity = renderer_.params().bloom_intensity;
            renderer_.set_bloom_intensity(std::min(1.0f, intensity + 0.05f));
            input.increase_bloom = false;
            std::cout << "Bloom intensity: " << renderer_.params().bloom_intensity << "\n";
        }
        if (input.decrease_bloom) {
            float intensity = renderer_.params().bloom_intensity;
            renderer_.set_bloom_intensity(std::max(0.0f, intensity - 0.05f));
            input.decrease_bloom = false;
            std::cout << "Bloom intensity: " << renderer_.params().bloom_intensity << "\n";
        }

        // Simulation speed controls
        if (input.pause_toggle) {
            game_state_.simulation_paused = !game_state_.simulation_paused;
            input.pause_toggle = false;
            std::cout << "Simulation: " << (game_state_.simulation_paused ? "PAUSED" : "RUNNING") << "\n";
        }
        if (input.speed_up) {
            // Speed levels: 0.25, 0.5, 1.0, 2.0, 4.0
            if (game_state_.simulation_speed < 0.5f) game_state_.simulation_speed = 0.5f;
            else if (game_state_.simulation_speed < 1.0f) game_state_.simulation_speed = 1.0f;
            else if (game_state_.simulation_speed < 2.0f) game_state_.simulation_speed = 2.0f;
            else if (game_state_.simulation_speed < 4.0f) game_state_.simulation_speed = 4.0f;
            input.speed_up = false;
            std::cout << "Simulation speed: " << game_state_.simulation_speed << "x\n";
        }
        if (input.speed_down) {
            if (game_state_.simulation_speed > 2.0f) game_state_.simulation_speed = 2.0f;
            else if (game_state_.simulation_speed > 1.0f) game_state_.simulation_speed = 1.0f;
            else if (game_state_.simulation_speed > 0.5f) game_state_.simulation_speed = 0.5f;
            else if (game_state_.simulation_speed > 0.25f) game_state_.simulation_speed = 0.25f;
            input.speed_down = false;
            std::cout << "Simulation speed: " << game_state_.simulation_speed << "x\n";
        }

        // Help overlay toggle
        if (input.show_help) {
            game_state_.show_help_overlay = !game_state_.show_help_overlay;
            input.show_help = false;
        }

        // Close color menu on escape
        if (input.escape_pressed && input.show_color_menu) {
            input.show_color_menu = false;
            input.escape_pressed = false;  // Consume escape
        }

        // Category navigation with keyboard (prev/next cycle through categories)
        if (input.prev_page) {
            if (open_category_ > 0) {
                open_category_--;
            } else if (open_category_ == -1) {
                open_category_ = NUM_CATEGORIES - 1;
            } else {
                open_category_ = -1;  // Close all
            }
            input.prev_page = false;
        }
        if (input.next_page) {
            if (open_category_ < NUM_CATEGORIES - 1) {
                open_category_++;
            } else {
                open_category_ = -1;  // Close all
            }
            input.next_page = false;
        }

        // Check for UI clicks (only on initial click)
        static bool was_mouse_down = false;
        static bool was_right_mouse_down = false;
        bool is_mouse_down = input.mouse_left_down;
        bool is_right_down = input.mouse_right_down;

        // Handle right-click on UI for favorites
        if (is_right_down && !was_right_mouse_down) {
            int32_t mx = input.mouse_x;
            int32_t my = input.mouse_y;

            MaterialID clicked_mat;
            int click_result = check_dropdown_click(mx, my, clicked_mat, true);  // true = right click
            if (click_result == 3) {
                // Favorite was toggled
                was_right_mouse_down = is_right_down;
                return;
            }
        }
        was_right_mouse_down = is_right_down;

        if (is_mouse_down && !was_mouse_down) {
            int32_t mx = input.mouse_x;
            int32_t my = input.mouse_y;

            // Check brush palette clicks (left panel)
            if (mx >= BRUSH_PANEL_X && mx <= BRUSH_PANEL_X + BRUSH_PANEL_WIDTH &&
                my >= BRUSH_PANEL_Y && my <= BRUSH_PANEL_Y + 230) {

                // Check tool button clicks
                int btn_x = BRUSH_PANEL_X + 5;
                int btn_y = BRUSH_PANEL_Y + 20;  // After title
                ToolMode tools[] = {ToolMode::Brush, ToolMode::Line, ToolMode::Rectangle, ToolMode::Circle, ToolMode::Fill};

                for (int i = 0; i < 5; i++) {
                    if (mx >= btn_x && mx < btn_x + BRUSH_BTN_SIZE &&
                        my >= btn_y && my < btn_y + BRUSH_BTN_SIZE) {
                        input.tool_mode = tools[i];
                        input.shape_drawing = false;
                        was_mouse_down = is_mouse_down;
                        return;
                    }
                    // Move to next position (2 columns)
                    if (i % 2 == 0) {
                        btn_x += BRUSH_BTN_SPACING + 20;
                    } else {
                        btn_x = BRUSH_PANEL_X + 5;
                        btn_y += BRUSH_BTN_SPACING;
                    }
                }

                was_mouse_down = is_mouse_down;
                return;  // Click was in brush palette, consume it
            }

            // Check material dropdown clicks (right panel)
            MaterialID clicked_mat;
            int click_result = check_dropdown_click(mx, my, clicked_mat);
            if (click_result == 1) {
                input.selected_material = clicked_mat;
                was_mouse_down = is_mouse_down;
                return;
            } else if (click_result == 2) {
                was_mouse_down = is_mouse_down;
                return;
            }
        }

        was_mouse_down = is_mouse_down;

        // Get current mouse position
        int32_t mx = input.mouse_x;
        int32_t my = input.mouse_y;

        // Don't interact if clicking in the right UI area
        int ui_height = get_ui_total_height();
        bool in_ui = (mx >= UI_PANEL_X - 5 && my <= ui_height + 10);
        // Also check left brush palette area
        if (mx <= BRUSH_PANEL_X + BRUSH_PANEL_WIDTH + 5 && my <= 240) {
            in_ui = true;
        }

        // ===== SHAPE TOOLS (Line, Rectangle, Circle) =====
        if (input.tool_mode == ToolMode::Line ||
            input.tool_mode == ToolMode::Rectangle ||
            input.tool_mode == ToolMode::Circle) {

            if (input.mouse_left_down && !in_ui) {
                if (!input.shape_drawing) {
                    // Start drawing a shape
                    input.shape_start_x = mx;
                    input.shape_start_y = my;
                    input.shape_drawing = true;
                }
                // While drawing, preview is handled in render
            } else if (input.shape_drawing) {
                // Mouse released - commit the shape
                if (input.tool_mode == ToolMode::Line) {
                    draw_line(input.shape_start_x, input.shape_start_y, mx, my, input.selected_material);
                } else if (input.tool_mode == ToolMode::Rectangle) {
                    draw_rectangle(input.shape_start_x, input.shape_start_y, mx, my,
                                   input.selected_material, input.filled_shapes);
                } else if (input.tool_mode == ToolMode::Circle) {
                    draw_ellipse(input.shape_start_x, input.shape_start_y, mx, my,
                                 input.selected_material, input.filled_shapes);
                }
                input.shape_drawing = false;
            }

            // Right click cancels shape drawing
            if (input.mouse_right_down && input.shape_drawing) {
                input.shape_drawing = false;
            }

            return;  // Don't process brush tool when in shape mode
        }

        // ===== PIPETTE (INSPECT) TOOL =====
        if (input.tool_mode == ToolMode::Pipette) {
            static bool pipette_was_down = false;
            if (input.mouse_left_down && !pipette_was_down && !in_ui) {
                // Pick the material under cursor and select it
                if (world_.in_bounds(mx, my)) {
                    MaterialID picked = world_.get_material(mx, my);
                    if (picked != MaterialID::Empty) {
                        // Check if material is available (in story mode, must be unlocked)
                        bool can_select = true;
                        if (game_state_.current_mode == GameMode::StoryMode) {
                            can_select = discovery_system_.is_material_unlocked(picked);
                        }
                        if (can_select) {
                            platform_.get_input_state().selected_material = picked;
                            std::cout << "Picked material: " << get_material_name(picked) << "\n";
                        }
                    }
                }
            }
            pipette_was_down = input.mouse_left_down;
            return;  // Don't process brush tool when in pipette mode
        }

        // ===== FILL TOOL =====
        if (input.tool_mode == ToolMode::Fill) {
            static bool fill_was_down = false;
            if (input.mouse_left_down && !fill_was_down && !in_ui) {
                flood_fill(mx, my, input.selected_material);
            }
            fill_was_down = input.mouse_left_down;
            return;  // Don't process brush tool when in fill mode
        }

        // ===== BRUSH TOOL MODE =====
        // Place materials with mouse
        if (input.mouse_left_down || input.mouse_right_down) {
            int32_t x = mx;
            int32_t y = my;

            if (in_ui) {
                return;
            }

            // Use configurable brush for all materials (including Life)
            int32_t brush_radius = input.brush_radius;
            BrushShape brush_shape = input.brush_shape;

            for (int32_t dy = -brush_radius; dy <= brush_radius; ++dy) {
                for (int32_t dx = -brush_radius; dx <= brush_radius; ++dx) {
                    bool in_brush = false;

                    // Check if pixel is within brush shape
                    if (brush_shape == BrushShape::Circle) {
                        // Circular brush - use distance check
                        in_brush = (dx * dx + dy * dy <= brush_radius * brush_radius);
                    } else {
                        // Square brush - all pixels in square
                        in_brush = true;
                    }

                    if (in_brush) {
                        int32_t px = x + dx;
                        int32_t py = y + dy;

                        if (world_.in_bounds(px, py)) {
                            if (input.mouse_left_down) {
                                // CRITICAL: Clear cell state before placing new material
                                // This prevents grass from inheriting burn state from previous fire/smoke
                                Cell& cell = world_.get_cell(px, py);
                                cell.flags = 0;  // Clear all flags (lifetime, flow_direction, updated)
                                cell.velocity_y = 0;  // Clear velocity

                                // Place selected material
                                world_.set_material(px, py, input.selected_material);

                                // Initialize properties for materials that need it
                                if (input.selected_material == MaterialID::Fire) {
                                    cell.set_lifetime(30);
                                    cell.velocity_y = -5;  // Start rising
                                } else if (input.selected_material == MaterialID::Steam) {
                                    cell.velocity_y = -5;  // Start rising (no lifetime!)
                                } else if (input.selected_material == MaterialID::Smoke) {
                                    cell.set_lifetime(40);
                                    cell.velocity_y = -3;  // Start rising
                                } else if (input.selected_material == MaterialID::Ash) {
                                    cell.velocity_y = -2;  // Start rising
                                }
                                // NOTE: Person material handled separately above
                            } else if (input.mouse_right_down) {
                                // Erase (place empty) - also clear state
                                Cell& cell = world_.get_cell(px, py);
                                cell.flags = 0;
                                cell.velocity_y = 0;
                                world_.set_material(px, py, MaterialID::Empty);
                            }
                        }
                    }
                }
            }
        }
    }

    void draw_filled_rect(int x, int y, int width, int height, uint32_t color) {
        for (int py = y; py < y + height && py < WORLD_HEIGHT; ++py) {
            for (int px = x; px < x + width && px < WORLD_WIDTH; ++px) {
                if (px >= 0 && py >= 0) {
                    pixel_buffer_[py * WORLD_WIDTH + px] = color;
                }
            }
        }
    }

    void draw_rect(int x, int y, int width, int height, uint32_t color) {
        // Top and bottom edges
        for (int px = x; px < x + width && px < WORLD_WIDTH; ++px) {
            if (px >= 0) {
                if (y >= 0 && y < WORLD_HEIGHT) {
                    pixel_buffer_[y * WORLD_WIDTH + px] = color;
                }
                int bottom = y + height - 1;
                if (bottom >= 0 && bottom < WORLD_HEIGHT) {
                    pixel_buffer_[bottom * WORLD_WIDTH + px] = color;
                }
            }
        }
        // Left and right edges
        for (int py = y; py < y + height && py < WORLD_HEIGHT; ++py) {
            if (py >= 0) {
                if (x >= 0 && x < WORLD_WIDTH) {
                    pixel_buffer_[py * WORLD_WIDTH + x] = color;
                }
                int right = x + width - 1;
                if (right >= 0 && right < WORLD_WIDTH) {
                    pixel_buffer_[py * WORLD_WIDTH + right] = color;
                }
            }
        }
    }

    // Simple 5x7 pixel font rendering (numbers and letters)
    void draw_char(int x, int y, char c, uint32_t color) {
        // Bitmap font for digits 0-9 and uppercase A-Z
        static const uint8_t font[37][7] = {
            // Digits 0-9
            {0x7E, 0xC3, 0xC3, 0xC3, 0xC3, 0xC3, 0x7E}, // 0
            {0x18, 0x38, 0x18, 0x18, 0x18, 0x18, 0x7E}, // 1
            {0x7E, 0xC3, 0x03, 0x3E, 0xC0, 0xC0, 0xFF}, // 2
            {0x7E, 0xC3, 0x03, 0x3E, 0x03, 0xC3, 0x7E}, // 3
            {0xC3, 0xC3, 0xC3, 0xFF, 0x03, 0x03, 0x03}, // 4
            {0xFF, 0xC0, 0xC0, 0xFE, 0x03, 0xC3, 0x7E}, // 5
            {0x7E, 0xC0, 0xC0, 0xFE, 0xC3, 0xC3, 0x7E}, // 6
            {0xFF, 0x03, 0x06, 0x0C, 0x18, 0x18, 0x18}, // 7
            {0x7E, 0xC3, 0xC3, 0x7E, 0xC3, 0xC3, 0x7E}, // 8
            {0x7E, 0xC3, 0xC3, 0x7F, 0x03, 0x03, 0x7E}, // 9
            // Letters A-Z
            {0x7E, 0xC3, 0xC3, 0xFF, 0xC3, 0xC3, 0xC3}, // A
            {0xFE, 0xC3, 0xC3, 0xFE, 0xC3, 0xC3, 0xFE}, // B
            {0x7E, 0xC3, 0xC0, 0xC0, 0xC0, 0xC3, 0x7E}, // C
            {0xFC, 0xC6, 0xC3, 0xC3, 0xC3, 0xC6, 0xFC}, // D
            {0xFF, 0xC0, 0xC0, 0xFE, 0xC0, 0xC0, 0xFF}, // E
            {0xFF, 0xC0, 0xC0, 0xFE, 0xC0, 0xC0, 0xC0}, // F
            {0x7E, 0xC3, 0xC0, 0xCF, 0xC3, 0xC3, 0x7E}, // G
            {0xC3, 0xC3, 0xC3, 0xFF, 0xC3, 0xC3, 0xC3}, // H
            {0x7E, 0x18, 0x18, 0x18, 0x18, 0x18, 0x7E}, // I
            {0x3F, 0x06, 0x06, 0x06, 0xC6, 0xC6, 0x7C}, // J
            {0xC3, 0xC6, 0xCC, 0xF8, 0xCC, 0xC6, 0xC3}, // K
            {0xC0, 0xC0, 0xC0, 0xC0, 0xC0, 0xC0, 0xFF}, // L
            {0xC3, 0xE7, 0xFF, 0xDB, 0xC3, 0xC3, 0xC3}, // M
            {0xC3, 0xE3, 0xF3, 0xDB, 0xCF, 0xC7, 0xC3}, // N
            {0x7E, 0xC3, 0xC3, 0xC3, 0xC3, 0xC3, 0x7E}, // O
            {0xFE, 0xC3, 0xC3, 0xFE, 0xC0, 0xC0, 0xC0}, // P
            {0x7E, 0xC3, 0xC3, 0xC3, 0xDB, 0xC6, 0x7D}, // Q
            {0xFE, 0xC3, 0xC3, 0xFE, 0xCC, 0xC6, 0xC3}, // R
            {0x7E, 0xC3, 0xC0, 0x7E, 0x03, 0xC3, 0x7E}, // S
            {0xFF, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18}, // T
            {0xC3, 0xC3, 0xC3, 0xC3, 0xC3, 0xC3, 0x7E}, // U
            {0xC3, 0xC3, 0xC3, 0xC3, 0x66, 0x3C, 0x18}, // V
            {0xC3, 0xC3, 0xC3, 0xDB, 0xFF, 0xE7, 0xC3}, // W
            {0xC3, 0x66, 0x3C, 0x18, 0x3C, 0x66, 0xC3}, // X
            {0xC3, 0xC3, 0x66, 0x3C, 0x18, 0x18, 0x18}, // Y
            {0xFF, 0x03, 0x06, 0x0C, 0x18, 0x30, 0xFF}, // Z
            {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}  // Space
        };

        int idx;
        if (c >= '0' && c <= '9') {
            idx = c - '0';
        } else if (c >= 'A' && c <= 'Z') {
            idx = 10 + (c - 'A');
        } else if (c >= 'a' && c <= 'z') {
            idx = 10 + (c - 'a');  // Convert lowercase to uppercase
        } else {
            idx = 36;  // Space
        }

        for (int row = 0; row < 7; ++row) {
            for (int col = 0; col < 8; ++col) {
                if (font[idx][row] & (1 << (7 - col))) {
                    int px = x + col;
                    int py = y + row;
                    if (px >= 0 && px < WORLD_WIDTH && py >= 0 && py < WORLD_HEIGHT) {
                        pixel_buffer_[py * WORLD_WIDTH + px] = color;
                    }
                }
            }
        }
    }

    void draw_text(int x, int y, const std::string& text, uint32_t color) {
        int cx = x;
        for (char c : text) {
            draw_char(cx, y, c, color);
            cx += 9;  // Character width + spacing
        }
    }

    // Find material name by ID (searches all categories)
    const char* find_material_name(MaterialID id) const {
        for (int cat = 0; cat < NUM_CATEGORIES; cat++) {
            for (int i = 0; i < categories_[cat].count; i++) {
                if (categories_[cat].materials[i].id == id) {
                    return categories_[cat].materials[i].name;
                }
            }
        }
        return "Unknown";
    }

    void render_material_palette() {
        // Colors (fully opaque for transparency support)
        const uint32_t bg_color = 0xFF181818;        // Dark gray background
        const uint32_t header_bg = 0xFF303030;      // Slightly lighter for headers
        const uint32_t header_open_bg = 0xFF404050; // Purple tint when open
        const uint32_t text_color = 0xFFFFFFFF;     // White
        const uint32_t text_dim = 0xFFAAAAAA;       // Dim gray text
        const uint32_t locked_color = 0xFF606060;  // Dark gray for locked materials
        const uint32_t selected_color = 0xFFFFFF00; // Yellow for selected
        const uint32_t arrow_color = 0xFF00FFFF;    // Cyan arrows

        // Check if we're in Story Mode
        bool story_mode = (game_state_.current_mode == GameMode::StoryMode);

        // Calculate glow animation (pulsing 0.5 to 1.0 over ~1 second)
        float glow_phase = sinf(g_frame_counter * 0.1f) * 0.5f + 0.5f;  // 0 to 1
        uint8_t glow_alpha = static_cast<uint8_t>(80 + glow_phase * 80);  // 80-160 alpha
        uint32_t glow_color = (glow_alpha << 24) | 0x00FF88;  // Cyan-green glow

        const auto& input = platform_.get_input_state();
        int x = UI_PANEL_X;
        int y = 10;

        // Calculate panel height
        int panel_height = get_ui_total_height();

        // Draw main background panel
        draw_filled_rect(x - 5, 5, UI_PANEL_WIDTH + 10, panel_height, bg_color);

        // Title and progress (in Story Mode)
        if (story_mode) {
            draw_text(x, y, "MATERIALS", text_color);
            int unlocked = discovery_system_.get_unlocked_count();
            int total = discovery_system_.get_total_materials();
            std::string progress = std::to_string(unlocked) + "/" + std::to_string(total);
            draw_text(x + 75, y, progress, 0xFF00FF00);  // Green progress
            y += 15;
            draw_text(x, y, "[J] Journal", text_dim);
            y += 15;
        } else {
            draw_text(x, y, "MATERIALS", text_color);
            y += 15;
        }

        // ===== FAVORITES BAR =====
        const uint32_t fav_bg = 0xFF252530;  // Slightly different from main bg
        const uint32_t fav_border = 0xFF505060;  // Border color
        const int FAV_SWATCH_SIZE = 18;
        const int FAV_SPACING = 22;
        const int FAV_BAR_HEIGHT = 28;

        // Draw favorites section header
        draw_text(x, y, "Favorites", text_dim);
        draw_text(x + 65, y, "(R-click)", 0xFF707070);
        y += 12;

        // Draw favorites bar background
        draw_filled_rect(x - 3, y, UI_PANEL_WIDTH + 6, FAV_BAR_HEIGHT, fav_bg);
        draw_rect(x - 3, y, UI_PANEL_WIDTH + 6, FAV_BAR_HEIGHT, fav_border);

        // Draw favorite slots
        int fav_x = x + 3;
        int fav_y = y + 5;
        for (int i = 0; i < MAX_FAVORITES; i++) {
            bool has_material = (i < favorites_count_);
            bool is_selected = has_material && (input.selected_material == favorites_[i]);

            // Draw slot background (empty or with material color)
            if (has_material) {
                // In Story Mode, check if material is unlocked
                bool is_unlocked = !story_mode || discovery_system_.is_material_unlocked(favorites_[i]);

                if (is_unlocked) {
                    Color mat_color = material_system_.get_material(favorites_[i]).base_color;
                    draw_filled_rect(fav_x, fav_y, FAV_SWATCH_SIZE, FAV_SWATCH_SIZE, mat_color.to_rgba32());
                } else {
                    // Locked material in favorites - show gray
                    draw_filled_rect(fav_x, fav_y, FAV_SWATCH_SIZE, FAV_SWATCH_SIZE, locked_color);
                }

                // Selection border
                if (is_selected) {
                    draw_rect(fav_x - 1, fav_y - 1, FAV_SWATCH_SIZE + 2, FAV_SWATCH_SIZE + 2, selected_color);
                }
            } else {
                // Empty slot - draw dim outline
                draw_rect(fav_x, fav_y, FAV_SWATCH_SIZE, FAV_SWATCH_SIZE, 0xFF404040);
            }

            fav_x += FAV_SPACING;
        }

        y += FAV_BAR_HEIGHT + 5;

        // Draw each category
        for (int cat = 0; cat < NUM_CATEGORIES; cat++) {
            bool is_open = (open_category_ == cat);

            // Check if any material in this category has potential discoveries (Story Mode)
            bool category_has_discoveries = false;
            if (story_mode) {
                for (int i = 0; i < categories_[cat].count; i++) {
                    MaterialID mid = categories_[cat].materials[i].id;
                    if (discovery_system_.is_material_unlocked(mid) &&
                        discovery_system_.has_potential_discovery(mid)) {
                        category_has_discoveries = true;
                        break;
                    }
                }
            }

            // Category header background (with glow tint if has discoveries)
            uint32_t hdr_bg = is_open ? header_open_bg : header_bg;
            if (category_has_discoveries && !is_open) {
                // Add subtle glow tint to header
                hdr_bg = 0xFF304040;  // Slight cyan tint
            }
            draw_filled_rect(x - 3, y, UI_PANEL_WIDTH + 6, UI_HEADER_HEIGHT, hdr_bg);

            // Arrow indicator (> or v) - glow if has discoveries
            uint32_t arrow_col = category_has_discoveries ? 0xFF00FFAA : arrow_color;
            if (is_open) {
                draw_text(x, y + 4, "V", arrow_col);
            } else {
                draw_text(x, y + 4, ">", arrow_col);
            }

            // Category name (glow if has discoveries)
            uint32_t cat_name_color = category_has_discoveries ? 0xFF00FFAA : text_color;
            draw_text(x + 12, y + 4, std::string(categories_[cat].name), cat_name_color);

            // Item count (in Story Mode, show unlocked/total)
            std::string count_str;
            if (story_mode) {
                int unlocked = 0;
                for (int i = 0; i < categories_[cat].count; i++) {
                    if (discovery_system_.is_material_unlocked(categories_[cat].materials[i].id)) {
                        unlocked++;
                    }
                }
                count_str = "(" + std::to_string(unlocked) + "/" + std::to_string(categories_[cat].count) + ")";
            } else {
                count_str = "(" + std::to_string(categories_[cat].count) + ")";
            }
            draw_text(x + 80, y + 4, count_str, text_dim);

            y += UI_HEADER_HEIGHT + 2;

            // If category is open, draw its materials
            if (is_open) {
                for (int i = 0; i < categories_[cat].count; i++) {
                    const MaterialEntry& mat = categories_[cat].materials[i];
                    bool is_unlocked = !story_mode || discovery_system_.is_material_unlocked(mat.id);
                    bool is_selected = (input.selected_material == mat.id);

                    // Check if this material has potential discoveries (Story Mode only)
                    bool has_discovery = story_mode && is_unlocked &&
                                         discovery_system_.has_potential_discovery(mat.id);

                    // Highlight background for selected material (only if unlocked)
                    if (is_selected && is_unlocked) {
                        draw_filled_rect(x - 3, y, UI_PANEL_WIDTH + 6, UI_ITEM_HEIGHT, 0xFF404080);
                    }

                    if (is_unlocked) {
                        // Draw glow effect for materials with potential discoveries
                        if (has_discovery && !is_selected) {
                            // Outer glow (larger, more transparent)
                            draw_filled_rect(x + 5, y - 1, UI_SWATCH_SIZE + 8, UI_SWATCH_SIZE + 6, glow_color);
                        }

                        // Material color swatch
                        Color mat_color = material_system_.get_material(mat.id).base_color;
                        uint32_t swatch_color = mat_color.to_rgba32();
                        draw_filled_rect(x + 8, y + 2, UI_SWATCH_SIZE, UI_SWATCH_SIZE, swatch_color);

                        // Swatch border
                        if (is_selected) {
                            // Yellow border for selected
                            draw_filled_rect(x + 7, y + 1, UI_SWATCH_SIZE + 2, 1, selected_color);
                            draw_filled_rect(x + 7, y + 2 + UI_SWATCH_SIZE, UI_SWATCH_SIZE + 2, 1, selected_color);
                            draw_filled_rect(x + 7, y + 1, 1, UI_SWATCH_SIZE + 2, selected_color);
                            draw_filled_rect(x + 8 + UI_SWATCH_SIZE, y + 1, 1, UI_SWATCH_SIZE + 2, selected_color);
                        } else if (has_discovery) {
                            // Glowing border for materials with discoveries
                            uint32_t border_glow = 0xFF00FFAA;  // Bright cyan-green
                            draw_filled_rect(x + 7, y + 1, UI_SWATCH_SIZE + 2, 1, border_glow);
                            draw_filled_rect(x + 7, y + 2 + UI_SWATCH_SIZE, UI_SWATCH_SIZE + 2, 1, border_glow);
                            draw_filled_rect(x + 7, y + 1, 1, UI_SWATCH_SIZE + 2, border_glow);
                            draw_filled_rect(x + 8 + UI_SWATCH_SIZE, y + 1, 1, UI_SWATCH_SIZE + 2, border_glow);
                        }

                        // Material name
                        uint32_t name_color = is_selected ? selected_color : text_color;
                        if (has_discovery && !is_selected) {
                            name_color = 0xFF00FFAA;  // Cyan-green for materials with discoveries
                        }
                        draw_text(x + 8 + UI_SWATCH_SIZE + 6, y + 3, std::string(mat.name), name_color);
                    } else {
                        // Locked material - show mystery swatch and "???"
                        draw_filled_rect(x + 8, y + 2, UI_SWATCH_SIZE, UI_SWATCH_SIZE, locked_color);
                        draw_text(x + 8 + UI_SWATCH_SIZE + 6, y + 3, "???", locked_color);
                    }

                    y += UI_ITEM_HEIGHT;
                }
            }
        }

        // Selected material display at bottom
        y += 5;
        draw_filled_rect(x - 5, y, UI_PANEL_WIDTH + 10, 25, 0xFF000000);

        // Get selected material info
        const char* selected_name = find_material_name(input.selected_material);
        Color sel_color = material_system_.get_material(input.selected_material).base_color;

        // Draw selected material swatch
        draw_filled_rect(x, y + 5, 16, 16, sel_color.to_rgba32());

        // Draw selected material name
        draw_text(x + 22, y + 8, std::string(selected_name), selected_color);
    }

    // Brush Palette UI constants
    static constexpr int BRUSH_PANEL_X = 5;
    static constexpr int BRUSH_PANEL_Y = 5;
    static constexpr int BRUSH_PANEL_WIDTH = 110;
    static constexpr int BRUSH_BTN_SIZE = 22;
    static constexpr int BRUSH_BTN_SPACING = 26;

    // Get tool name string
    const char* get_tool_name(ToolMode mode) const {
        switch (mode) {
            case ToolMode::Brush: return "Brush";
            case ToolMode::Line: return "Line";
            case ToolMode::Rectangle: return "Rect";
            case ToolMode::Circle: return "Circle";
            case ToolMode::Fill: return "Fill";
            case ToolMode::Pipette: return "Inspect";
            default: return "???";
        }
    }

    void render_brush_palette() {
        const uint32_t bg_color = 0xFF181818;
        const uint32_t btn_color = 0xFF303030;
        const uint32_t btn_selected = 0xFF505080;
        const uint32_t text_color = 0xFFFFFFFF;
        const uint32_t text_dim = 0xFFAAAAAA;
        const uint32_t highlight = 0xFFFFFF00;

        const auto& input = platform_.get_input_state();

        int x = BRUSH_PANEL_X;
        int y = BRUSH_PANEL_Y;

        // Calculate panel height
        int panel_height = 230;

        // Draw background
        draw_filled_rect(x, y, BRUSH_PANEL_WIDTH, panel_height, bg_color);

        // Title
        draw_text(x + 5, y + 5, "TOOLS", text_color);
        y += 20;

        // Tool buttons - 2 columns
        struct ToolButton {
            ToolMode mode;
            const char* label;
            const char* key;
        };
        ToolButton tools[] = {
            {ToolMode::Brush, "D", "D"},
            {ToolMode::Line, "L", "L"},
            {ToolMode::Rectangle, "R", "R"},
            {ToolMode::Circle, "O", "O"},
            {ToolMode::Fill, "E", "E"},
            {ToolMode::Pipette, "I", "I"},
        };

        int btn_x = x + 5;
        int btn_y = y;
        for (int i = 0; i < 6; i++) {
            bool selected = (input.tool_mode == tools[i].mode);
            uint32_t btn_bg = selected ? btn_selected : btn_color;

            // Draw button
            draw_filled_rect(btn_x, btn_y, BRUSH_BTN_SIZE, BRUSH_BTN_SIZE, btn_bg);

            // Draw selection border
            if (selected) {
                draw_filled_rect(btn_x - 1, btn_y - 1, BRUSH_BTN_SIZE + 2, 1, highlight);
                draw_filled_rect(btn_x - 1, btn_y + BRUSH_BTN_SIZE, BRUSH_BTN_SIZE + 2, 1, highlight);
                draw_filled_rect(btn_x - 1, btn_y - 1, 1, BRUSH_BTN_SIZE + 2, highlight);
                draw_filled_rect(btn_x + BRUSH_BTN_SIZE, btn_y - 1, 1, BRUSH_BTN_SIZE + 2, highlight);
            }

            // Draw label
            draw_text(btn_x + 7, btn_y + 7, tools[i].label, text_color);

            // Move to next position (2 columns)
            if (i % 2 == 0) {
                btn_x += BRUSH_BTN_SPACING + 20;
            } else {
                btn_x = x + 5;
                btn_y += BRUSH_BTN_SPACING;
            }
        }

        y = btn_y + BRUSH_BTN_SPACING + 5;

        // Current tool name
        draw_text(x + 5, y, get_tool_name(input.tool_mode), highlight);
        y += 15;

        // Divider
        draw_filled_rect(x + 5, y, BRUSH_PANEL_WIDTH - 10, 1, 0xFF404040);
        y += 8;

        // Brush size (for brush tool)
        if (input.tool_mode == ToolMode::Brush) {
            draw_text(x + 5, y, "SIZE", text_dim);
            y += 12;

            // Size indicator
            std::string size_str = std::to_string(input.brush_radius);
            draw_text(x + 5, y, size_str, text_color);

            // Size bar
            int bar_width = 60;
            int bar_x = x + 30;
            draw_filled_rect(bar_x, y + 2, bar_width, 6, btn_color);
            int fill_width = (input.brush_radius * bar_width) / 20;
            draw_filled_rect(bar_x, y + 2, fill_width, 6, highlight);

            y += 15;

            // Brush shape
            draw_text(x + 5, y, "SHAPE", text_dim);
            y += 12;
            draw_text(x + 5, y,
                      (input.brush_shape == BrushShape::Circle) ? "Circle" : "Square",
                      text_color);
            y += 15;
        }

        // Fill mode for shape tools
        if (input.tool_mode == ToolMode::Rectangle || input.tool_mode == ToolMode::Circle) {
            draw_text(x + 5, y, "MODE", text_dim);
            y += 12;
            draw_text(x + 5, y,
                      input.filled_shapes ? "Filled" : "Outline",
                      text_color);
            draw_text(x + 60, y, "(X)", text_dim);
            y += 15;
        }

        // Keyboard hints at bottom
        y = BRUSH_PANEL_Y + panel_height - 25;
        draw_filled_rect(x, y - 5, BRUSH_PANEL_WIDTH, 30, 0xFF000000);
        draw_text(x + 5, y, "[/] Size", text_dim);
        draw_text(x + 5, y + 10, "B Shape", text_dim);
    }

    void render_debug_gui() {
        const uint32_t bg_color = 0xFF000000;  // Fully opaque
        const uint32_t text_color = 0xFFFFFFFF;
        const uint32_t warning_color = 0xFF0000FF;

        // Draw background panel (shifted right to not overlap brush palette)
        int panel_x = BRUSH_PANEL_X + BRUSH_PANEL_WIDTH + 10;
        draw_filled_rect(panel_x, 5, 150, 55, bg_color);

        // Draw FPS
        int y = 10;
        draw_text(panel_x + 5, y, "FPS: " + std::to_string(static_cast<int>(current_fps_)),
                  current_fps_ < 50.0f ? warning_color : text_color);

        // Draw active chunks
        y += 15;
        draw_text(panel_x + 5, y, "Chunks: " + std::to_string(simulation_.get_active_chunks()), text_color);

        // Draw active cells
        y += 15;
        draw_text(panel_x + 5, y, "Cells: " + std::to_string(active_cells_display_), text_color);
    }

    void render_speed_indicator() {
        // Show pause and speed status at top-center of screen
        if (!game_state_.is_playing()) return;

        std::string status;
        uint32_t color = 0xFFFFFFFF;

        if (game_state_.simulation_paused) {
            status = "|| PAUSED";
            color = 0xFF00FFFF;  // Cyan
        } else if (game_state_.simulation_speed != 1.0f) {
            char buf[32];
            snprintf(buf, sizeof(buf), "%.2gx SPEED", game_state_.simulation_speed);
            status = buf;
            color = (game_state_.simulation_speed > 1.0f) ? 0xFF00FF00 : 0xFFFFFF00;
        } else {
            return;  // Normal speed, don't show anything
        }

        int text_width = static_cast<int>(status.length()) * 7;
        int x = (WORLD_WIDTH - text_width) / 2;
        int y = 10;

        // Draw background (fully opaque)
        draw_filled_rect(x - 5, y - 3, text_width + 10, 18, 0xFF000000);
        draw_text(x, y, status, color);
    }

    // Color picker menu rendering and handling
    void render_color_menu() {
        auto& input = platform_.get_input_state();
        if (!input.show_color_menu) return;

        // Panel dimensions
        int panel_width = 220;
        int panel_height = 280;
        int panel_x = (WORLD_WIDTH - panel_width) / 2;
        int panel_y = (WORLD_HEIGHT - panel_height) / 2;

        // Background (fully opaque so transparency doesn't show through)
        draw_filled_rect(panel_x, panel_y, panel_width, panel_height, 0xFF101020);
        draw_rect(panel_x, panel_y, panel_width, panel_height, 0xFF4080FF);

        // Title
        draw_text(panel_x + 50, panel_y + 12, "BACKGROUND COLOR", 0xFF4080FF);

        int y = panel_y + 40;
        int col_x = panel_x + 15;

        // Color presets grid (3 columns x 4 rows)
        static const struct { uint32_t color; const char* name; } presets[] = {
            {0xFF000000, "Black"},
            {0xFF0D0D0D, "Near Black"},
            {0xFF1A1A2E, "Dark Blue"},
            {0xFF2E1A2E, "Dark Purple"},
            {0xFF1A2E1A, "Dark Green"},
            {0xFF2E2E1A, "Dark Brown"},
            {0xFF1A2E2E, "Dark Teal"},
            {0xFF2E1A1A, "Dark Red"},
            {0xFF202020, "Dark Gray"},
            {0xFF303030, "Gray"},
            {0xFF404040, "Medium Gray"},
            {0xFF505050, "Light Gray"},
        };
        static const int num_presets = sizeof(presets) / sizeof(presets[0]);

        const int SWATCH_SIZE = 32;
        const int SWATCH_SPACING = 40;
        const int SWATCHES_PER_ROW = 4;

        // Draw color swatches
        int mx = input.mouse_x;
        int my = input.mouse_y;
        static bool was_clicking = false;
        bool is_clicking = input.mouse_left_down;

        for (int i = 0; i < num_presets; i++) {
            int row = i / SWATCHES_PER_ROW;
            int col = i % SWATCHES_PER_ROW;
            int sx = col_x + col * SWATCH_SPACING + 20;
            int sy = y + row * SWATCH_SPACING;

            // Draw swatch
            draw_filled_rect(sx, sy, SWATCH_SIZE, SWATCH_SIZE, presets[i].color);

            // Check if this is the current color
            bool is_current = (input.background_color == presets[i].color && !input.transparent_background);
            if (is_current) {
                draw_rect(sx - 2, sy - 2, SWATCH_SIZE + 4, SWATCH_SIZE + 4, 0xFFFFFF00);
            }

            // Check hover
            if (mx >= sx && mx < sx + SWATCH_SIZE && my >= sy && my < sy + SWATCH_SIZE) {
                draw_rect(sx - 1, sy - 1, SWATCH_SIZE + 2, SWATCH_SIZE + 2, 0xFFFFFFFF);

                // Show color name tooltip
                draw_filled_rect(sx - 10, sy + SWATCH_SIZE + 4, 55, 14, 0xFF000000);
                draw_text(sx - 8, sy + SWATCH_SIZE + 6, presets[i].name, 0xFFCCCCCC);

                // Handle click
                if (is_clicking && !was_clicking) {
                    input.background_color = presets[i].color;
                    input.transparent_background = false;
                    std::cout << "Background: " << presets[i].name << "\n";
                }
            }
        }

        y += (num_presets / SWATCHES_PER_ROW + 1) * SWATCH_SPACING + 10;

        // Transparency toggle button
        int btn_x = col_x + 20;
        int btn_y = y;
        int btn_w = 160;
        int btn_h = 28;

        // Draw transparency button
        uint32_t btn_bg = input.transparent_background ? 0xFF305030 : 0xFF303030;
        draw_filled_rect(btn_x, btn_y, btn_w, btn_h, btn_bg);
        draw_rect(btn_x, btn_y, btn_w, btn_h, input.transparent_background ? 0xFF00FF00 : 0xFF606060);

        const char* trans_text = input.transparent_background ? "[X] Transparent" : "[ ] Transparent";
        draw_text(btn_x + 30, btn_y + 8, trans_text, input.transparent_background ? 0xFF00FF00 : 0xFFCCCCCC);

        // Check hover and click on transparency button
        if (mx >= btn_x && mx < btn_x + btn_w && my >= btn_y && my < btn_y + btn_h) {
            draw_rect(btn_x - 1, btn_y - 1, btn_w + 2, btn_h + 2, 0xFFFFFFFF);
            if (is_clicking && !was_clicking) {
                input.transparent_background = !input.transparent_background;
                std::cout << "Transparent: " << (input.transparent_background ? "ON" : "OFF") << "\n";
            }
        }

        y += btn_h + 15;

        // Footer
        draw_text(panel_x + 55, y, "Press T or Esc to close", 0xFF666666);

        was_clicking = is_clicking;
    }

    void render_help_overlay() {
        if (!game_state_.show_help_overlay) return;

        // Semi-transparent overlay
        int panel_width = 320;
        int panel_height = 400;
        int panel_x = (WORLD_WIDTH - panel_width) / 2;
        int panel_y = (WORLD_HEIGHT - panel_height) / 2;

        // Background (fully opaque)
        draw_filled_rect(panel_x, panel_y, panel_width, panel_height, 0xFF101020);
        draw_rect(panel_x, panel_y, panel_width, panel_height, 0xFF4080FF);

        // Title
        draw_text(panel_x + 100, panel_y + 10, "KEYBOARD SHORTCUTS", 0xFF4080FF);

        int y = panel_y + 35;
        int col1 = panel_x + 15;
        int col2 = panel_x + 170;
        uint32_t key_color = 0xFF00FFFF;
        uint32_t desc_color = 0xFFCCCCCC;
        uint32_t header_color = 0xFFFFFF00;

        // Tools
        draw_text(col1, y, "--- TOOLS ---", header_color); y += 18;
        draw_text(col1, y, "D", key_color); draw_text(col1 + 30, y, "Brush (Draw)", desc_color);
        draw_text(col2, y, "L", key_color); draw_text(col2 + 30, y, "Line", desc_color); y += 14;
        draw_text(col1, y, "R", key_color); draw_text(col1 + 30, y, "Rectangle", desc_color);
        draw_text(col2, y, "O", key_color); draw_text(col2 + 30, y, "Circle", desc_color); y += 14;
        draw_text(col1, y, "E", key_color); draw_text(col1 + 30, y, "Fill", desc_color);
        draw_text(col2, y, "I", key_color); draw_text(col2 + 30, y, "Pipette", desc_color); y += 18;

        // Brush
        draw_text(col1, y, "--- BRUSH ---", header_color); y += 18;
        draw_text(col1, y, "[ ]", key_color); draw_text(col1 + 40, y, "Size -/+", desc_color);
        draw_text(col2, y, "B", key_color); draw_text(col2 + 30, y, "Shape", desc_color); y += 14;
        draw_text(col1, y, "X", key_color); draw_text(col1 + 30, y, "Filled/Outline", desc_color); y += 18;

        // Simulation
        draw_text(col1, y, "--- SIMULATION ---", header_color); y += 18;
        draw_text(col1, y, "Space", key_color); draw_text(col1 + 55, y, "Pause/Resume", desc_color); y += 14;
        draw_text(col1, y, "< >", key_color); draw_text(col1 + 40, y, "Speed -/+", desc_color);
        draw_text(col2, y, "C", key_color); draw_text(col2 + 30, y, "Clear", desc_color); y += 18;

        // Visual
        draw_text(col1, y, "--- VISUAL ---", header_color); y += 18;
        draw_text(col1, y, "T", key_color); draw_text(col1 + 30, y, "Color menu", desc_color);
        draw_text(col2, y, "F1", key_color); draw_text(col2 + 30, y, "Post-process", desc_color); y += 14;
        draw_text(col1, y, "F2", key_color); draw_text(col1 + 35, y, "Bloom", desc_color);
        draw_text(col2, y, "+ -", key_color); draw_text(col2 + 35, y, "Bloom +/-", desc_color); y += 18;

        // Materials
        draw_text(col1, y, "--- MATERIALS ---", header_color); y += 18;
        draw_text(col1, y, "R-Click", key_color); draw_text(col1 + 65, y, "Add/Remove favorite", desc_color); y += 14;
        draw_text(col1, y, ", .", key_color); draw_text(col1 + 40, y, "Prev/Next category", desc_color); y += 18;

        // Other
        draw_text(col1, y, "--- OTHER ---", header_color); y += 18;
        draw_text(col1, y, "Tab", key_color); draw_text(col1 + 45, y, "Debug info", desc_color);
        draw_text(col2, y, "J", key_color); draw_text(col2 + 30, y, "Journal", desc_color); y += 14;
        draw_text(col1, y, "Esc", key_color); draw_text(col1 + 45, y, "Menu", desc_color); y += 25;

        // Footer
        draw_text(panel_x + 100, y, "Press H to close", 0xFF888888);
    }

    // Draw a line preview in the pixel buffer (for visualization only)
    void draw_line_preview(int32_t x0, int32_t y0, int32_t x1, int32_t y1, uint32_t color) {
        int32_t thickness = 3;

        int32_t dx = std::abs(x1 - x0);
        int32_t dy = std::abs(y1 - y0);
        int32_t sx = (x0 < x1) ? 1 : -1;
        int32_t sy = (y0 < y1) ? 1 : -1;
        int32_t err = dx - dy;

        while (true) {
            // Draw a thick point at current position
            for (int32_t ty = -thickness/2; ty <= thickness/2; ty++) {
                for (int32_t tx = -thickness/2; tx <= thickness/2; tx++) {
                    int32_t px = x0 + tx;
                    int32_t py = y0 + ty;
                    if (px >= 0 && px < WORLD_WIDTH && py >= 0 && py < WORLD_HEIGHT) {
                        pixel_buffer_[py * WORLD_WIDTH + px] = color;
                    }
                }
            }

            if (x0 == x1 && y0 == y1) break;

            int32_t e2 = 2 * err;
            if (e2 > -dy) {
                err -= dy;
                x0 += sx;
            }
            if (e2 < dx) {
                err += dx;
                y0 += sy;
            }
        }
    }

    // Draw rectangle preview
    void draw_rect_preview(int32_t x0, int32_t y0, int32_t x1, int32_t y1, uint32_t color, bool filled) {
        int32_t left = std::min(x0, x1);
        int32_t right = std::max(x0, x1);
        int32_t top = std::min(y0, y1);
        int32_t bottom = std::max(y0, y1);

        if (filled) {
            for (int32_t py = top; py <= bottom; py++) {
                for (int32_t px = left; px <= right; px++) {
                    if (px >= 0 && px < WORLD_WIDTH && py >= 0 && py < WORLD_HEIGHT) {
                        pixel_buffer_[py * WORLD_WIDTH + px] = color;
                    }
                }
            }
        } else {
            // Outline only
            for (int32_t px = left; px <= right; px++) {
                if (px >= 0 && px < WORLD_WIDTH) {
                    if (top >= 0 && top < WORLD_HEIGHT)
                        pixel_buffer_[top * WORLD_WIDTH + px] = color;
                    if (bottom >= 0 && bottom < WORLD_HEIGHT)
                        pixel_buffer_[bottom * WORLD_WIDTH + px] = color;
                }
            }
            for (int32_t py = top; py <= bottom; py++) {
                if (py >= 0 && py < WORLD_HEIGHT) {
                    if (left >= 0 && left < WORLD_WIDTH)
                        pixel_buffer_[py * WORLD_WIDTH + left] = color;
                    if (right >= 0 && right < WORLD_WIDTH)
                        pixel_buffer_[py * WORLD_WIDTH + right] = color;
                }
            }
        }
    }

    // Draw ellipse preview
    void draw_ellipse_preview(int32_t x0, int32_t y0, int32_t x1, int32_t y1, uint32_t color, bool filled) {
        int32_t cx = (x0 + x1) / 2;
        int32_t cy = (y0 + y1) / 2;
        int32_t rx = std::abs(x1 - x0) / 2;
        int32_t ry = std::abs(y1 - y0) / 2;

        if (rx == 0 || ry == 0) {
            draw_line_preview(x0, y0, x1, y1, color);
            return;
        }

        if (filled) {
            for (int32_t dy = -ry; dy <= ry; dy++) {
                float yf = static_cast<float>(dy) / static_cast<float>(ry);
                float xf = std::sqrt(1.0f - yf * yf);
                int32_t xExtent = static_cast<int32_t>(xf * rx);

                for (int32_t dx = -xExtent; dx <= xExtent; dx++) {
                    int32_t px = cx + dx;
                    int32_t py = cy + dy;
                    if (px >= 0 && px < WORLD_WIDTH && py >= 0 && py < WORLD_HEIGHT) {
                        pixel_buffer_[py * WORLD_WIDTH + px] = color;
                    }
                }
            }
        } else {
            int steps = std::max(rx, ry) * 4;
            for (int i = 0; i < steps; i++) {
                float angle = 2.0f * 3.14159f * i / steps;
                int32_t px = cx + static_cast<int32_t>(rx * std::cos(angle));
                int32_t py = cy + static_cast<int32_t>(ry * std::sin(angle));
                if (px >= 0 && px < WORLD_WIDTH && py >= 0 && py < WORLD_HEIGHT) {
                    pixel_buffer_[py * WORLD_WIDTH + px] = color;
                }
            }
        }
    }

    void render_tool_preview() {
        const auto& input = platform_.get_input_state();
        int32_t x = input.mouse_x;
        int32_t y = input.mouse_y;

        // Don't show preview if in right UI area
        int ui_height = get_ui_total_height();
        if (x >= UI_PANEL_X - 5 && y <= ui_height + 10) {
            return;
        }
        // Don't show preview if in left brush palette area
        if (x <= BRUSH_PANEL_X + BRUSH_PANEL_WIDTH + 5 && y <= 240) {
            return;
        }

        uint32_t preview_color = 0x80FFFFFF;  // Semi-transparent white
        uint32_t cursor_color = input.shape_drawing ? 0xFFFF0000 : 0xFF00FFFF;

        // ===== SHAPE TOOLS PREVIEW =====
        if (input.tool_mode == ToolMode::Line ||
            input.tool_mode == ToolMode::Rectangle ||
            input.tool_mode == ToolMode::Circle) {

            if (input.shape_drawing) {
                // Draw preview shape from start to current mouse position
                if (input.tool_mode == ToolMode::Line) {
                    draw_line_preview(input.shape_start_x, input.shape_start_y, x, y, preview_color);
                } else if (input.tool_mode == ToolMode::Rectangle) {
                    draw_rect_preview(input.shape_start_x, input.shape_start_y, x, y,
                                      preview_color, input.filled_shapes);
                } else if (input.tool_mode == ToolMode::Circle) {
                    draw_ellipse_preview(input.shape_start_x, input.shape_start_y, x, y,
                                         preview_color, input.filled_shapes);
                }

                // Draw start point marker (green)
                for (int32_t dy = -2; dy <= 2; dy++) {
                    for (int32_t dx = -2; dx <= 2; dx++) {
                        int32_t px = input.shape_start_x + dx;
                        int32_t py = input.shape_start_y + dy;
                        if (px >= 0 && px < WORLD_WIDTH && py >= 0 && py < WORLD_HEIGHT) {
                            pixel_buffer_[py * WORLD_WIDTH + px] = 0xFF00FF00;
                        }
                    }
                }
            }

            // Draw crosshair at cursor
            for (int32_t i = -5; i <= 5; i++) {
                if (x + i >= 0 && x + i < WORLD_WIDTH && y >= 0 && y < WORLD_HEIGHT) {
                    pixel_buffer_[y * WORLD_WIDTH + (x + i)] = cursor_color;
                }
                if (x >= 0 && x < WORLD_WIDTH && y + i >= 0 && y + i < WORLD_HEIGHT) {
                    pixel_buffer_[(y + i) * WORLD_WIDTH + x] = cursor_color;
                }
            }
            return;
        }

        // ===== FILL TOOL PREVIEW =====
        if (input.tool_mode == ToolMode::Fill) {
            // Draw a bucket icon / target at cursor
            uint32_t fill_color = 0xFF00FFFF;
            for (int32_t i = -8; i <= 8; i++) {
                if (x + i >= 0 && x + i < WORLD_WIDTH && y >= 0 && y < WORLD_HEIGHT) {
                    pixel_buffer_[y * WORLD_WIDTH + (x + i)] = fill_color;
                }
                if (x >= 0 && x < WORLD_WIDTH && y + i >= 0 && y + i < WORLD_HEIGHT) {
                    pixel_buffer_[(y + i) * WORLD_WIDTH + x] = fill_color;
                }
            }
            // Draw a circle around it
            for (int angle = 0; angle < 360; angle += 15) {
                float rad = angle * 3.14159f / 180.0f;
                int32_t px = x + static_cast<int32_t>(6 * std::cos(rad));
                int32_t py = y + static_cast<int32_t>(6 * std::sin(rad));
                if (px >= 0 && px < WORLD_WIDTH && py >= 0 && py < WORLD_HEIGHT) {
                    pixel_buffer_[py * WORLD_WIDTH + px] = fill_color;
                }
            }
            return;
        }

        // ===== PIPETTE (INSPECT) TOOL PREVIEW =====
        if (input.tool_mode == ToolMode::Pipette) {
            // Get material at cursor position
            MaterialID mat = MaterialID::Empty;
            if (x >= 0 && x < WORLD_WIDTH && y >= 0 && y < WORLD_HEIGHT) {
                mat = world_.get_material(x, y);
            }

            // Draw a magnifying glass cursor
            uint32_t pipette_color = 0xFF00FFFF;  // Cyan
            // Circle
            for (int angle = 0; angle < 360; angle += 20) {
                float rad = angle * 3.14159f / 180.0f;
                int32_t px = x + static_cast<int32_t>(5 * std::cos(rad));
                int32_t py = y + static_cast<int32_t>(5 * std::sin(rad));
                if (px >= 0 && px < WORLD_WIDTH && py >= 0 && py < WORLD_HEIGHT) {
                    pixel_buffer_[py * WORLD_WIDTH + px] = pipette_color;
                }
            }
            // Handle
            for (int i = 5; i <= 10; i++) {
                int32_t px = x + i;
                int32_t py = y + i;
                if (px >= 0 && px < WORLD_WIDTH && py >= 0 && py < WORLD_HEIGHT) {
                    pixel_buffer_[py * WORLD_WIDTH + px] = pipette_color;
                }
            }

            // Draw material info tooltip
            const char* mat_name = get_material_name(mat);
            Color mat_color = material_system_.get_material(mat).base_color;

            // Tooltip position (offset from cursor)
            int tooltip_x = x + 15;
            int tooltip_y = y - 25;

            // Keep tooltip on screen
            if (tooltip_x + 80 > WORLD_WIDTH) tooltip_x = x - 90;
            if (tooltip_y < 5) tooltip_y = y + 15;

            // Draw tooltip background (fully opaque)
            draw_filled_rect(tooltip_x - 3, tooltip_y - 3, 85, 30, 0xFF202020);
            draw_rect(tooltip_x - 3, tooltip_y - 3, 85, 30, 0xFF606060);

            // Draw material color swatch
            draw_filled_rect(tooltip_x, tooltip_y, 12, 12, mat_color.to_rgba32());
            draw_rect(tooltip_x, tooltip_y, 12, 12, 0xFFFFFFFF);

            // Draw material name
            draw_text(tooltip_x + 16, tooltip_y + 2, mat_name, 0xFFFFFFFF);

            // Draw coordinates
            char coords[32];
            snprintf(coords, sizeof(coords), "(%d, %d)", x, y);
            draw_text(tooltip_x, tooltip_y + 14, coords, 0xFF888888);

            return;
        }

        // ===== BRUSH TOOL PREVIEW =====
        int32_t brush_radius = input.brush_radius;
        BrushShape brush_shape = input.brush_shape;

        // Draw brush outline
        for (int32_t dy = -brush_radius; dy <= brush_radius; ++dy) {
            for (int32_t dx = -brush_radius; dx <= brush_radius; ++dx) {
                bool is_edge = false;

                if (brush_shape == BrushShape::Circle) {
                    int dist_sq = dx * dx + dy * dy;
                    int radius_sq = brush_radius * brush_radius;
                    if (dist_sq <= radius_sq && dist_sq > (brush_radius - 1) * (brush_radius - 1)) {
                        is_edge = true;
                    }
                } else {
                    if (std::abs(dx) == brush_radius || std::abs(dy) == brush_radius) {
                        is_edge = true;
                    }
                }

                if (is_edge) {
                    int32_t px = x + dx;
                    int32_t py = y + dy;
                    if (px >= 0 && px < WORLD_WIDTH && py >= 0 && py < WORLD_HEIGHT) {
                        pixel_buffer_[py * WORLD_WIDTH + px] = preview_color;
                    }
                }
            }
        }

        // Draw center crosshair
        if (x >= 0 && x < WORLD_WIDTH && y >= 0 && y < WORLD_HEIGHT) {
            pixel_buffer_[y * WORLD_WIDTH + x] = 0xFFFF0000;
        }
    }

    void render_enhanced_people() {
        // Make people more visible with AI STATE COLORS and animations
        for (int32_t y = 0; y < WORLD_HEIGHT; y++) {
            for (int32_t x = 0; x < WORLD_WIDTH; x++) {
                if (world_.get_material(x, y) == MaterialID::Person) {
                    Cell& cell = world_.get_cell(x, y);
                    uint8_t health = cell.get_health();

                    if (health == 0) continue;  // Dead, don't render

                    // ========================================
                    // DETECT AI STATE for visual feedback
                    // ========================================

                    bool touching_fire = false;
                    bool touching_lava = false;
                    bool in_water = false;

                    for (int dy = -1; dy <= 1; dy++) {
                        for (int dx = -1; dx <= 1; dx++) {
                            int nx = x + dx;
                            int ny = y + dy;
                            if (world_.in_bounds(nx, ny)) {
                                MaterialID neighbor = world_.get_material(nx, ny);
                                if (neighbor == MaterialID::Fire) touching_fire = true;
                                if (neighbor == MaterialID::Lava) touching_lava = true;
                                if (neighbor == MaterialID::Water) in_water = true;
                            }
                        }
                    }

                    // ========================================
                    // COLOR based on AI STATE (visible behavior)
                    // ========================================

                    uint32_t person_color;
                    uint32_t outline_color = 0xFFFFFFFF;  // Default white outline

                    if (touching_fire || touching_lava) {
                        // PANIC STATE: Bright orange-red (on fire / panicking)
                        person_color = 0xFFFF6000;
                        outline_color = 0xFFFF0000;  // Red outline when burning
                    } else if (in_water) {
                        // SWIMMING STATE: Cyan-blue (in water)
                        person_color = 0xFF00FFFF;
                        outline_color = 0xFF0080FF;  // Blue outline when wet
                    } else if (health < 30) {
                        // LOW HEALTH STATE: Dark red (dying)
                        person_color = 0xFF800000;
                        outline_color = 0xFFFF0000;  // Red outline when dying
                    } else if (health < 60) {
                        // INJURED STATE: Yellow (hurt but ok)
                        person_color = 0xFFFFFF00;
                        outline_color = 0xFFFFAA00;  // Orange outline when injured
                    } else {
                        // HEALTHY STATE: Bright magenta (happy and healthy)
                        person_color = 0xFFFF00FF;
                        outline_color = 0xFFFFFFFF;  // White outline when healthy
                    }

                    // ========================================
                    // DRAW 2x2 person block
                    // ========================================

                    for (int dy = 0; dy < 2; dy++) {
                        for (int dx = 0; dx < 2; dx++) {
                            int px = x + dx;
                            int py = y + dy;
                            if (px < WORLD_WIDTH && py < WORLD_HEIGHT) {
                                pixel_buffer_[py * WORLD_WIDTH + px] = person_color;
                            }
                        }
                    }

                    // ========================================
                    // DRAW STATE-COLORED OUTLINE
                    // ========================================

                    int outline_positions[][2] = {
                        {-1, -1}, {0, -1}, {1, -1}, {2, -1},
                        {-1, 0}, {2, 0},
                        {-1, 1}, {2, 1},
                        {-1, 2}, {0, 2}, {1, 2}, {2, 2}
                    };

                    for (auto& pos : outline_positions) {
                        int px = x + pos[0];
                        int py = y + pos[1];
                        if (px >= 0 && px < WORLD_WIDTH && py >= 0 && py < WORLD_HEIGHT) {
                            // Only draw outline if not overlapping another person
                            if (world_.get_material(px, py) != MaterialID::Person) {
                                pixel_buffer_[py * WORLD_WIDTH + px] = outline_color;
                            }
                        }
                    }

                    // ========================================
                    // FACING DIRECTION INDICATOR (small marker)
                    // ========================================

                    bool facing_right = cell.get_person_facing_right();
                    int eye_x = x + (facing_right ? 1 : 0);
                    int eye_y = y;

                    if (eye_x >= 0 && eye_x < WORLD_WIDTH && eye_y >= 0 && eye_y < WORLD_HEIGHT) {
                        pixel_buffer_[eye_y * WORLD_WIDTH + eye_x] = 0xFF000000;  // Black "eye" shows facing
                    }
                }

                // Also render Life particles with a sparkle effect
                if (world_.get_material(x, y) == MaterialID::Life) {
                    Cell& cell = world_.get_cell(x, y);
                    uint8_t sparkle = cell.get_lifetime();

                    // Animated sparkle color - cycles between pink and white
                    uint8_t intensity = 200 + (sparkle & 0x1F) * 2;
                    uint32_t life_color;
                    if ((sparkle & 0x08) != 0) {
                        // Pink phase
                        life_color = (0xFF << 24) | (intensity << 16) | ((intensity * 3 / 4) << 8) | intensity;
                    } else {
                        // White-ish phase
                        life_color = (0xFF << 24) | (intensity << 16) | (intensity << 8) | (intensity * 3 / 4);
                    }

                    // Draw the Life particle with a small glow
                    pixel_buffer_[y * WORLD_WIDTH + x] = life_color;

                    // Add glow effect around it
                    uint32_t glow_color = 0x40FF80FF;  // Semi-transparent magenta
                    for (int dy = -1; dy <= 1; dy++) {
                        for (int dx = -1; dx <= 1; dx++) {
                            if (dx == 0 && dy == 0) continue;
                            int gx = x + dx, gy = y + dy;
                            if (gx >= 0 && gx < WORLD_WIDTH && gy >= 0 && gy < WORLD_HEIGHT) {
                                MaterialID neighbor = world_.get_material(gx, gy);
                                if (neighbor == MaterialID::Empty) {
                                    // Blend glow with existing pixel (simple additive)
                                    pixel_buffer_[gy * WORLD_WIDTH + gx] = glow_color;
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    void render() {
        // Render based on current game mode
        switch (game_state_.current_mode) {
            case GameMode::MainMenu:
                render_main_menu();
                break;

            case GameMode::Sandbox:
            case GameMode::StoryMode:
                render_gameplay();
                break;

            case GameMode::Journal:
                render_gameplay();  // Render game in background
                render_journal_overlay();
                break;

            case GameMode::Achievements:
                render_achievements_screen();
                break;

            case GameMode::Paused:
                render_gameplay();  // Render game in background
                render_pause_overlay();
                break;
        }

        // Render achievement popup on top of everything (if active)
        if (game_state_.showing_achievement_popup) {
            render_discovery_popup();
        }

        // Update Metal texture
        renderer_.update_texture(pixel_buffer_.data());

        // Render frame
        renderer_.render();
    }

    void render_gameplay() {
        // Generate color buffer from world state (with background color)
        const auto& input = platform_.get_input_state();
        uint32_t bg_color = input.transparent_background ? 0x00000000 : input.background_color;
        world_.generate_color_buffer(pixel_buffer_.data(), bg_color);

        // Enhance people rendering (make them visible with 2x2 size and outline)
        render_enhanced_people();

        // Always show UI panels
        render_material_palette();  // Right side - materials
        render_brush_palette();     // Left side - tools

        // Draw tool preview at cursor
        render_tool_preview();

        // Render debug GUI overlay if enabled
        if (platform_.get_input_state().show_debug_gui) {
            render_debug_gui();
        }

        // Render speed/pause indicator
        render_speed_indicator();

        // Render color menu (on top of most things)
        render_color_menu();

        // Render help overlay (on top of everything)
        render_help_overlay();
    }

    void render_main_menu() {
        // Clear to dark background
        std::fill(pixel_buffer_.begin(), pixel_buffer_.end(), 0xFF1a1a2e);

        // Draw title
        const char* title = "PIXEL ENGINE";
        int title_x = WORLD_WIDTH / 2 - 60;
        int title_y = 80;
        draw_text(title_x, title_y, title, 0xFFFFFFFF);

        // Draw subtitle
        const char* subtitle = "Falling Sand Simulation";
        draw_text(title_x - 30, title_y + 20, subtitle, 0xFF888888);

        // Menu panel
        int panel_x = WORLD_WIDTH / 2 - 100;
        int panel_y = 180;
        int panel_width = 200;
        int button_height = 35;
        int button_spacing = 10;

        // Draw menu buttons
        const char* menu_items[] = {"SANDBOX", "STORY MODE", "ACHIEVEMENTS", "QUIT"};
        int num_items = 4;

        for (int i = 0; i < num_items; i++) {
            int btn_y = panel_y + i * (button_height + button_spacing);
            bool selected = (static_cast<int>(game_state_.menu_selection) == i);

            // Button background
            uint32_t bg_color = selected ? 0xFF4a4a6a : 0xFF2a2a4a;
            draw_filled_rect(panel_x, btn_y, panel_width, button_height, bg_color);

            // Button border
            uint32_t border_color = selected ? 0xFFFFFF00 : 0xFF555555;
            draw_rect(panel_x, btn_y, panel_width, button_height, border_color);

            // Selection indicator
            if (selected) {
                draw_text(panel_x + 10, btn_y + 10, ">", 0xFFFFFF00);
            }

            // Button text
            uint32_t text_color = selected ? 0xFFFFFFFF : 0xFFAAAAAA;
            int text_x = panel_x + 30;
            draw_text(text_x, btn_y + 10, menu_items[i], text_color);
        }

        // Draw controls hint at bottom
        draw_text(WORLD_WIDTH / 2 - 80, WORLD_HEIGHT - 60, "[UP/DOWN] Select", 0xFF666666);
        draw_text(WORLD_WIDTH / 2 - 60, WORLD_HEIGHT - 40, "[ENTER] Confirm", 0xFF666666);

        // Draw version/credits
        draw_text(10, WORLD_HEIGHT - 20, "v1.0 - Made with love", 0xFF444444);
    }

    void render_pause_overlay() {
        // Semi-transparent dark overlay
        for (int y = 0; y < WORLD_HEIGHT; y++) {
            for (int x = 0; x < WORLD_WIDTH; x++) {
                uint32_t& pixel = pixel_buffer_[y * WORLD_WIDTH + x];
                // Darken existing pixel
                uint8_t r = (pixel >> 0) & 0xFF;
                uint8_t g = (pixel >> 8) & 0xFF;
                uint8_t b = (pixel >> 16) & 0xFF;
                r = r / 2;
                g = g / 2;
                b = b / 2;
                pixel = (0xFF << 24) | (b << 16) | (g << 8) | r;
            }
        }

        // Draw pause text
        draw_filled_rect(WORLD_WIDTH / 2 - 60, WORLD_HEIGHT / 2 - 30, 120, 60, 0xFF2a2a4a);
        draw_rect(WORLD_WIDTH / 2 - 60, WORLD_HEIGHT / 2 - 30, 120, 60, 0xFFFFFFFF);
        draw_text(WORLD_WIDTH / 2 - 30, WORLD_HEIGHT / 2 - 10, "PAUSED", 0xFFFFFFFF);
        draw_text(WORLD_WIDTH / 2 - 50, WORLD_HEIGHT / 2 + 10, "[ESC] Resume", 0xFF888888);
    }

    void render_achievements_screen() {
        // Clear to dark background
        std::fill(pixel_buffer_.begin(), pixel_buffer_.end(), 0xFF1a1a2e);

        draw_text(WORLD_WIDTH / 2 - 60, 50, "ACHIEVEMENTS", 0xFFFFFFFF);

        // Show discovery progress
        int unlocked = discovery_system_.get_unlocked_count();
        int total = discovery_system_.get_total_materials();
        int combos = discovery_system_.get_discovered_combinations();
        int total_combos = discovery_system_.get_total_combinations();

        char progress_text[64];
        snprintf(progress_text, sizeof(progress_text), "Materials: %d / %d", unlocked, total);
        draw_text(100, 100, progress_text, 0xFFFFFFFF);

        snprintf(progress_text, sizeof(progress_text), "Combinations: %d / %d", combos, total_combos);
        draw_text(100, 130, progress_text, 0xFFFFFFFF);

        // Progress bar for materials
        int bar_x = 100;
        int bar_y = 160;
        int bar_width = 300;
        int bar_height = 20;
        draw_filled_rect(bar_x, bar_y, bar_width, bar_height, 0xFF333333);
        int filled_width = (bar_width * unlocked) / total;
        draw_filled_rect(bar_x, bar_y, filled_width, bar_height, 0xFF44AA44);
        draw_rect(bar_x, bar_y, bar_width, bar_height, 0xFF666666);

        draw_text(WORLD_WIDTH / 2 - 50, WORLD_HEIGHT - 60, "[ESC] Back", 0xFF666666);
    }

    void render_journal_overlay() {
        // Check previous_mode since current_mode is Journal when overlay is open
        bool sandbox_mode = (game_state_.previous_mode == GameMode::Sandbox);

        // Semi-transparent overlay
        int panel_x = 30;
        int panel_y = 30;
        int panel_width = WORLD_WIDTH - 60;
        int panel_height = WORLD_HEIGHT - 60;

        // Solid background (fully opaque so transparency doesn't show through)
        for (int y = panel_y; y < panel_y + panel_height; y++) {
            for (int x = panel_x; x < panel_x + panel_width; x++) {
                if (x >= 0 && x < WORLD_WIDTH && y >= 0 && y < WORLD_HEIGHT) {
                    pixel_buffer_[y * WORLD_WIDTH + x] = 0xFF1a1a2e;
                }
            }
        }

        draw_rect(panel_x, panel_y, panel_width, panel_height, 0xFFFFFFFF);

        // Title
        const char* title = sandbox_mode ? "RECIPE BOOK" : "DISCOVERY JOURNAL";
        draw_text(panel_x + panel_width / 2 - 50, panel_y + 10, title, 0xFFFFFFFF);

        // Tab buttons - different tabs for sandbox vs story
        const char* story_tabs[] = {"Recent", "Materials", "Recipes", "Hints"};
        const char* sandbox_tabs[] = {"Recipes", "Materials", "", ""};
        const char** tabs = sandbox_mode ? sandbox_tabs : story_tabs;
        int num_tabs = sandbox_mode ? 2 : 4;

        const auto& input = platform_.get_input_state();
        int tab_start_x = panel_x + 20;
        int tab_y = panel_y + 30;
        int tab_height = 20;
        int tab_width = 80;

        for (int i = 0; i < num_tabs; i++) {
            int tx = tab_start_x + i * 85;
            bool selected = (static_cast<int>(game_state_.journal_tab) == i);

            // Check for hover
            bool hovered = (input.mouse_x >= tx && input.mouse_x < tx + tab_width &&
                           input.mouse_y >= tab_y && input.mouse_y < tab_y + tab_height);

            // Draw tab background for selected or hovered
            if (selected) {
                draw_filled_rect(tx - 3, tab_y, tab_width, tab_height, 0xFF404060);
            } else if (hovered) {
                draw_filled_rect(tx - 3, tab_y, tab_width, tab_height, 0xFF303050);
            }

            uint32_t color = selected ? 0xFFFFFF00 : (hovered ? 0xFFCCCCCC : 0xFF888888);
            draw_text(tx, panel_y + 35, tabs[i], color);
        }

        // Navigation hint
        draw_text(panel_x + panel_width - 150, panel_y + 35, "[Left/Right] Tabs", 0xFF666666);

        // Content area
        int content_y = panel_y + 60;
        int content_height = panel_height - 90;
        int items_per_page = content_height / 18;

        // ===== SANDBOX MODE =====
        if (sandbox_mode) {
            if (game_state_.journal_tab == JournalTab::Recent) {  // "Recipes" in sandbox
                // Show ALL recipes
                int total_combos = get_combinations_count();

                // Scroll info
                int start_idx = game_state_.journal_scroll;
                int shown = 0;
                int y = content_y;

                // Header
                draw_text(panel_x + 30, y, "All Recipes:", 0xFFFFFF00);
                char count_text[32];
                snprintf(count_text, sizeof(count_text), "(%d total)", total_combos);
                draw_text(panel_x + 130, y, count_text, 0xFF888888);
                y += 22;

                // Debug: print once when first rendering
                static bool debug_printed = false;
                if (!debug_printed) {
                    debug_printed = true;
                    for (int d = 0; d < 5; d++) {
                        CombinationInfo c = get_combination_by_index(d);
                        std::cout << "DEBUG combo[" << d << "]: mat_a=" << static_cast<int>(c.mat_a)
                                  << " mat_b=" << static_cast<int>(c.mat_b)
                                  << " -> '" << get_material_name(c.mat_a) << "' + '" << get_material_name(c.mat_b) << "'\n";
                    }
                }

                for (int i = start_idx; i < total_combos && shown < items_per_page - 1; i++, shown++) {
                    CombinationInfo combo = get_combination_by_index(i);
                    if (combo.mat_a == MaterialID::Empty) continue;

                    const char* name_a = get_material_name(combo.mat_a);
                    const char* name_b = get_material_name(combo.mat_b);

                    // Determine what the combination produces
                    char recipe_text[96];
                    if (combo.result_a != MaterialID::Empty && combo.result_b != MaterialID::Empty) {
                        const char* result_a = get_material_name(combo.result_a);
                        const char* result_b = get_material_name(combo.result_b);
                        if (combo.result_a == combo.result_b) {
                            snprintf(recipe_text, sizeof(recipe_text), "%s + %s = %s", name_a, name_b, result_a);
                        } else {
                            snprintf(recipe_text, sizeof(recipe_text), "%s + %s = %s + %s", name_a, name_b, result_a, result_b);
                        }
                    } else if (combo.result_a != MaterialID::Empty) {
                        const char* result = get_material_name(combo.result_a);
                        snprintf(recipe_text, sizeof(recipe_text), "%s + %s = %s", name_a, name_b, result);
                    } else if (combo.result_b != MaterialID::Empty) {
                        const char* result = get_material_name(combo.result_b);
                        snprintf(recipe_text, sizeof(recipe_text), "%s + %s = %s", name_a, name_b, result);
                    } else {
                        snprintf(recipe_text, sizeof(recipe_text), "%s + %s = (reaction)", name_a, name_b);
                    }

                    draw_text(panel_x + 30, y, recipe_text, 0xFFCCCCCC);
                    y += 18;
                }

                // Scroll indicator
                if (total_combos > items_per_page) {
                    char scroll_text[32];
                    snprintf(scroll_text, sizeof(scroll_text), "Page %d/%d",
                             (start_idx / items_per_page) + 1,
                             ((total_combos - 1) / items_per_page) + 1);
                    draw_text(panel_x + panel_width / 2 - 30, panel_y + panel_height - 45, scroll_text, 0xFF888888);
                }
            } else if (game_state_.journal_tab == JournalTab::Materials) {
                // Show all materials by category
                int y = content_y;
                draw_text(panel_x + 30, y, "All Materials by Category:", 0xFFFFFF00);
                y += 22;

                int cat_idx = game_state_.journal_selected_category;
                if (cat_idx >= 0 && cat_idx < NUM_CATEGORIES) {
                    draw_text(panel_x + 30, y, categories_[cat_idx].name, 0xFF00FFFF);
                    y += 18;

                    int start_mat = game_state_.journal_scroll;
                    for (int i = start_mat; i < categories_[cat_idx].count && (y < panel_y + panel_height - 50); i++) {
                        Color mat_color = material_system_.get_material(categories_[cat_idx].materials[i].id).base_color;
                        draw_filled_rect(panel_x + 40, y + 1, 10, 10, mat_color.to_rgba32());
                        draw_text(panel_x + 55, y, categories_[cat_idx].materials[i].name, 0xFFCCCCCC);
                        y += 16;
                    }
                }

                // Category navigation hint
                draw_text(panel_x + 30, panel_y + panel_height - 45, "[Up/Down] Change category", 0xFF666666);
            }
        }
        // ===== STORY MODE =====
        else {
            if (game_state_.journal_tab == JournalTab::Recent) {
                // Show recent discoveries with recipe info
                const auto& discoveries = discovery_system_.get_all_discoveries();
                int y = content_y;

                draw_text(panel_x + 30, y, "Recent Discoveries:", 0xFFFFFF00);
                y += 22;

                int shown = 0;
                for (auto it = discoveries.rbegin(); it != discoveries.rend() && shown < items_per_page - 1; ++it, ++shown) {
                    if (it->material_unlocked != MaterialID::Empty) {
                        const char* mat_name = get_material_name(it->material_unlocked);

                        // Show the recipe that created it
                        char recipe_text[96];
                        if (it->ingredient_a != MaterialID::Empty && it->ingredient_b != MaterialID::Empty) {
                            const char* ing_a = get_material_name(it->ingredient_a);
                            const char* ing_b = get_material_name(it->ingredient_b);
                            snprintf(recipe_text, sizeof(recipe_text), "%s + %s = %s", ing_a, ing_b, mat_name);
                        } else {
                            snprintf(recipe_text, sizeof(recipe_text), "%s (unlocked)", mat_name);
                        }

                        Color mat_color = material_system_.get_material(it->material_unlocked).base_color;
                        draw_filled_rect(panel_x + 30, y + 1, 10, 10, mat_color.to_rgba32());
                        draw_text(panel_x + 45, y, recipe_text, 0xFFFFFFFF);
                        y += 18;
                    }
                }
                if (discoveries.empty()) {
                    draw_text(panel_x + 30, content_y + 22, "No discoveries yet!", 0xFF888888);
                    draw_text(panel_x + 30, content_y + 42, "Mix materials to discover new ones.", 0xFF666666);
                }
            } else if (game_state_.journal_tab == JournalTab::Materials) {
                // Show unlocked materials by category
                int y = content_y;
                draw_text(panel_x + 30, y, "Unlocked Materials:", 0xFFFFFF00);
                y += 22;

                int cat_idx = game_state_.journal_selected_category;
                if (cat_idx >= 0 && cat_idx < NUM_CATEGORIES) {
                    // Count unlocked in this category
                    int unlocked_count = 0;
                    for (int i = 0; i < categories_[cat_idx].count; i++) {
                        if (discovery_system_.is_material_unlocked(categories_[cat_idx].materials[i].id)) {
                            unlocked_count++;
                        }
                    }

                    char cat_header[64];
                    snprintf(cat_header, sizeof(cat_header), "%s (%d/%d)",
                             categories_[cat_idx].name, unlocked_count, categories_[cat_idx].count);
                    draw_text(panel_x + 30, y, cat_header, 0xFF00FFFF);
                    y += 18;

                    for (int i = 0; i < categories_[cat_idx].count && (y < panel_y + panel_height - 50); i++) {
                        MaterialID mat_id = categories_[cat_idx].materials[i].id;
                        bool unlocked = discovery_system_.is_material_unlocked(mat_id);

                        if (unlocked) {
                            Color mat_color = material_system_.get_material(mat_id).base_color;
                            draw_filled_rect(panel_x + 40, y + 1, 10, 10, mat_color.to_rgba32());
                            draw_text(panel_x + 55, y, categories_[cat_idx].materials[i].name, 0xFFCCCCCC);
                        } else {
                            draw_filled_rect(panel_x + 40, y + 1, 10, 10, 0xFF444444);
                            draw_text(panel_x + 55, y, "???", 0xFF666666);
                        }
                        y += 16;
                    }
                }

                draw_text(panel_x + 30, panel_y + panel_height - 45, "[Up/Down] Change category", 0xFF666666);

            } else if (game_state_.journal_tab == JournalTab::Recipes) {
                // Show discovered combinations
                int y = content_y;

                // Count discovered
                int discovered = discovery_system_.get_discovered_combinations();
                int total = discovery_system_.get_total_combinations();

                char header[64];
                snprintf(header, sizeof(header), "Discovered Recipes: %d/%d", discovered, total);
                draw_text(panel_x + 30, y, header, 0xFFFFFF00);
                y += 22;

                // Show discovered recipes from the discovery log
                const auto& discoveries = discovery_system_.get_all_discoveries();
                int start_idx = game_state_.journal_scroll;
                int shown = 0;

                // Build list of unique recipes
                for (auto it = discoveries.begin(); it != discoveries.end() && shown < items_per_page - 1; ++it) {
                    if (shown < start_idx) {
                        shown++;
                        continue;
                    }

                    if (it->ingredient_a != MaterialID::Empty && it->ingredient_b != MaterialID::Empty) {
                        const char* ing_a = get_material_name(it->ingredient_a);
                        const char* ing_b = get_material_name(it->ingredient_b);
                        const char* result = get_material_name(it->material_unlocked);

                        char recipe_text[96];
                        snprintf(recipe_text, sizeof(recipe_text), "%s + %s = %s", ing_a, ing_b, result);

                        Color result_color = material_system_.get_material(it->material_unlocked).base_color;
                        draw_filled_rect(panel_x + 30, y + 1, 10, 10, result_color.to_rgba32());
                        draw_text(panel_x + 45, y, recipe_text, 0xFFCCCCCC);
                        y += 18;
                    }
                    shown++;
                }

                if (discovered == 0) {
                    draw_text(panel_x + 30, content_y + 22, "No recipes discovered yet!", 0xFF888888);
                }

            } else if (game_state_.journal_tab == JournalTab::Hints) {
                // Show hints for undiscovered combinations
                auto hints = discovery_system_.get_available_hints();
                int y = content_y;

                draw_text(panel_x + 30, y, "Hints:", 0xFFFFFF00);
                y += 22;

                int shown = 0;
                for (const auto& hint : hints) {
                    if (shown >= items_per_page - 1) break;

                    if (hint.hint_level == HintLevel::Full) {
                        const char* name_a = get_material_name(hint.mat_a);
                        const char* name_b = get_material_name(hint.mat_b);
                        char hint_text[64];
                        snprintf(hint_text, sizeof(hint_text), "[!] %s + %s = ???", name_a, name_b);
                        draw_text(panel_x + 30, y, hint_text, 0xFF44FF44);
                    } else if (hint.hint_level == HintLevel::Partial) {
                        const char* name_a = get_material_name(hint.mat_a);
                        char hint_text[64];
                        snprintf(hint_text, sizeof(hint_text), "[?] %s + ??? = ???", name_a);
                        draw_text(panel_x + 30, y, hint_text, 0xFFFFFF44);
                    } else {
                        const char* cat_a = DiscoverySystem::get_material_category_name(hint.mat_a);
                        const char* cat_b = DiscoverySystem::get_material_category_name(hint.mat_b);
                        char hint_text[64];
                        snprintf(hint_text, sizeof(hint_text), "[?] Try %s + %s", cat_a, cat_b);
                        draw_text(panel_x + 30, y, hint_text, 0xFF888888);
                    }
                    y += 20;
                    shown++;
                }
                if (hints.empty()) {
                    draw_text(panel_x + 30, content_y + 22, "All combinations discovered!", 0xFF44FF44);
                }
            }
        }

        draw_text(panel_x + panel_width / 2 - 40, panel_y + panel_height - 25, "[ESC] Close", 0xFF888888);
    }

    void render_discovery_popup() {
        // Popup in top center of screen
        int popup_width = 250;
        int popup_height = 60;
        int popup_x = WORLD_WIDTH / 2 - popup_width / 2;
        int popup_y = 20;

        // Background with gold border
        draw_filled_rect(popup_x, popup_y, popup_width, popup_height, 0xFF2a2a4a);
        draw_rect(popup_x, popup_y, popup_width, popup_height, 0xFFFFD700);

        // Star icon
        draw_text(popup_x + 10, popup_y + 10, "*", 0xFFFFD700);

        // Title
        draw_text(popup_x + 30, popup_y + 10, "NEW DISCOVERY!", 0xFFFFD700);

        // Material name
        if (game_state_.pending_achievement_id >= 0) {
            MaterialID mat = static_cast<MaterialID>(game_state_.pending_achievement_id);
            const char* name = get_material_name(mat);
            char text[64];
            snprintf(text, sizeof(text), "Unlocked: %s", name);
            draw_text(popup_x + 30, popup_y + 32, text, 0xFFFFFFFF);
        }
    }

    const char* get_material_name(MaterialID id) const {
        for (int c = 0; c < NUM_CATEGORIES; c++) {
            for (int m = 0; m < categories_[c].count; m++) {
                if (categories_[c].materials[m].id == id) {
                    return categories_[c].materials[m].name;
                }
            }
        }
        return "Unknown";
    }
};

// Main entry point
int main(int argc, char* argv[]) {
    (void)argc;
    (void)argv;

    PixelEngineApp app;

    if (!app.initialize()) {
        std::cerr << "Failed to initialize application\n";
        return 1;
    }

    app.run();

    return 0;
}
