#include "Platform.h"
#include "Material.h"
#include "World.h"
#include "Simulation.h"
#include "MetalRenderer.h"

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
};

#define ARRAY_COUNT(arr) (sizeof(arr) / sizeof(arr[0]))

static const int NUM_CATEGORIES = 8;

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
        , scroll_offset_(0) {

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

    void create_initial_world() {
        // Create border walls
        for (int32_t x = 0; x < WORLD_WIDTH; ++x) {
            world_.set_material(x, 0, MaterialID::Stone);  // Bottom
            world_.set_material(x, WORLD_HEIGHT - 1, MaterialID::Stone);  // Top
        }
        for (int32_t y = 0; y < WORLD_HEIGHT; ++y) {
            world_.set_material(0, y, MaterialID::Stone);  // Left
            world_.set_material(WORLD_WIDTH - 1, y, MaterialID::Stone);  // Right
        }

        // Add some platforms for interesting initial state
        for (int32_t x = 200; x < 400; ++x) {
            world_.set_material(x, 300, MaterialID::Stone);
        }
        for (int32_t x = 450; x < 650; ++x) {
            world_.set_material(x, 400, MaterialID::Stone);
        }
    }

    void update(float delta_time) {
        // Handle input
        handle_input();

        // Fixed timestep simulation
        accumulator_ += delta_time;

        // Cap accumulator to prevent spiral of death
        if (accumulator_ > FIXED_TIMESTEP * 5.0f) {
            accumulator_ = FIXED_TIMESTEP * 5.0f;
        }

        // Update simulation with fixed timestep
        while (accumulator_ >= FIXED_TIMESTEP) {
            simulation_.update();
            accumulator_ -= FIXED_TIMESTEP;
        }

        // FPS counter
        ++frame_count_;
        fps_timer_ += delta_time;
        if (fps_timer_ >= 1.0f) {
            current_fps_ = static_cast<float>(frame_count_) / fps_timer_;
            active_cells_display_ = simulation_.get_updated_cells();

            std::cout << "FPS: " << frame_count_
                      << " | Active chunks: " << simulation_.get_active_chunks()
                      << " | Updated cells: " << simulation_.get_updated_cells()
                      << "\n";
            frame_count_ = 0;
            fps_timer_ = 0.0f;
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

    // Returns: 0 = no click in UI, 1 = material selected, 2 = category toggled
    int check_dropdown_click(int32_t mx, int32_t my, MaterialID& clicked_material) {
        // Check if click is within the UI panel area
        if (mx < UI_PANEL_X - 5 || mx > WORLD_WIDTH) {
            return 0;  // Not in UI
        }

        int y = 10;  // Start position

        // Skip title area
        y += 20;

        // Check each category
        for (int cat = 0; cat < NUM_CATEGORIES; cat++) {
            // Category header click area
            if (my >= y && my < y + UI_HEADER_HEIGHT) {
                // Toggle this category
                if (open_category_ == cat) {
                    open_category_ = -1;  // Close it
                } else {
                    open_category_ = cat;  // Open it (closes others)
                }
                scroll_offset_ = 0;
                return 2;  // Category toggled
            }
            y += UI_HEADER_HEIGHT + 2;

            // If this category is open, check material items
            if (open_category_ == cat) {
                for (int i = 0; i < categories_[cat].count; i++) {
                    if (my >= y && my < y + UI_ITEM_HEIGHT) {
                        clicked_material = categories_[cat].materials[i].id;
                        return 1;  // Material selected
                    }
                    y += UI_ITEM_HEIGHT;
                }
            }
        }

        return 0;  // Click was in UI but not on any interactive element
    }

    void handle_input() {
        auto& input = const_cast<InputState&>(platform_.get_input_state());

        // Handle clear world request
        if (input.clear_world) {
            world_.clear_world();
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
        bool is_mouse_down = input.mouse_left_down;

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
        // Colors
        const uint32_t bg_color = 0xE0181818;        // Dark gray background
        const uint32_t header_bg = 0xE0303030;      // Slightly lighter for headers
        const uint32_t header_open_bg = 0xE0404050; // Purple tint when open
        const uint32_t text_color = 0xFFFFFFFF;     // White
        const uint32_t text_dim = 0xFFAAAAAA;       // Dim gray text
        const uint32_t selected_color = 0xFFFFFF00; // Yellow for selected
        const uint32_t arrow_color = 0xFF00FFFF;    // Cyan arrows

        const auto& input = platform_.get_input_state();
        int x = UI_PANEL_X;
        int y = 10;

        // Calculate panel height
        int panel_height = get_ui_total_height();

        // Draw main background panel
        draw_filled_rect(x - 5, 5, UI_PANEL_WIDTH + 10, panel_height, bg_color);

        // Title
        draw_text(x, y, "MATERIALS", text_color);
        y += 20;

        // Draw each category
        for (int cat = 0; cat < NUM_CATEGORIES; cat++) {
            bool is_open = (open_category_ == cat);

            // Category header background
            uint32_t hdr_bg = is_open ? header_open_bg : header_bg;
            draw_filled_rect(x - 3, y, UI_PANEL_WIDTH + 6, UI_HEADER_HEIGHT, hdr_bg);

            // Arrow indicator (> or v)
            if (is_open) {
                draw_text(x, y + 4, "V", arrow_color);
            } else {
                draw_text(x, y + 4, ">", arrow_color);
            }

            // Category name
            draw_text(x + 12, y + 4, std::string(categories_[cat].name), text_color);

            // Item count
            std::string count_str = "(" + std::to_string(categories_[cat].count) + ")";
            draw_text(x + 90, y + 4, count_str, text_dim);

            y += UI_HEADER_HEIGHT + 2;

            // If category is open, draw its materials
            if (is_open) {
                for (int i = 0; i < categories_[cat].count; i++) {
                    const MaterialEntry& mat = categories_[cat].materials[i];
                    bool is_selected = (input.selected_material == mat.id);

                    // Highlight background for selected material
                    if (is_selected) {
                        draw_filled_rect(x - 3, y, UI_PANEL_WIDTH + 6, UI_ITEM_HEIGHT, 0xE0404080);
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
                    }

                    // Material name
                    uint32_t name_color = is_selected ? selected_color : text_color;
                    draw_text(x + 8 + UI_SWATCH_SIZE + 6, y + 3, std::string(mat.name), name_color);

                    y += UI_ITEM_HEIGHT;
                }
            }
        }

        // Selected material display at bottom
        y += 5;
        draw_filled_rect(x - 5, y, UI_PANEL_WIDTH + 10, 25, 0xE0000000);

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
            default: return "???";
        }
    }

    void render_brush_palette() {
        const uint32_t bg_color = 0xE0181818;
        const uint32_t btn_color = 0xE0303030;
        const uint32_t btn_selected = 0xE0505080;
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
        };

        int btn_x = x + 5;
        int btn_y = y;
        for (int i = 0; i < 5; i++) {
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
        draw_filled_rect(x, y - 5, BRUSH_PANEL_WIDTH, 30, 0xE0000000);
        draw_text(x + 5, y, "[/] Size", text_dim);
        draw_text(x + 5, y + 10, "B Shape", text_dim);
    }

    void render_debug_gui() {
        const uint32_t bg_color = 0x80000000;
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
        // Generate color buffer from world state
        world_.generate_color_buffer(pixel_buffer_.data());

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

        // Update Metal texture
        renderer_.update_texture(pixel_buffer_.data());

        // Render frame
        renderer_.render();
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
