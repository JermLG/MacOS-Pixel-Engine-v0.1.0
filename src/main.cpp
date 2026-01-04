#include "Platform.h"
#include "Material.h"
#include "World.h"
#include "Simulation.h"
#include "MetalRenderer.h"

#include <iostream>
#include <vector>
#include <cmath>

using namespace PixelEngine;

// All materials organized by category for the palette
struct MaterialEntry {
    MaterialID id;
    const char* name;
};

// Materials organized into pages (10 per page)
static const MaterialEntry ALL_MATERIALS[] = {
    // Page 0: Basic (original materials)
    {MaterialID::Sand, "Sand"},
    {MaterialID::Water, "Water"},
    {MaterialID::Stone, "Stone"},
    {MaterialID::Steam, "Steam"},
    {MaterialID::Oil, "Oil"},
    {MaterialID::Fire, "Fire"},
    {MaterialID::Wood, "Wood"},
    {MaterialID::Acid, "Acid"},
    {MaterialID::Lava, "Lava"},
    {MaterialID::Ash, "Ash"},

    // Page 1: More Basic + Powders
    {MaterialID::Grass, "Grass"},
    {MaterialID::Smoke, "Smoke"},
    {MaterialID::Person, "Person"},
    {MaterialID::Dirt, "Dirt"},
    {MaterialID::Gravel, "Gravel"},
    {MaterialID::Snow, "Snow"},
    {MaterialID::Gunpowder, "Gunpowder"},
    {MaterialID::Salt, "Salt"},
    {MaterialID::Coal, "Coal"},
    {MaterialID::Rust, "Rust"},

    // Page 2: More Powders + Liquids
    {MaterialID::Sawdust, "Sawdust"},
    {MaterialID::Glass_Powder, "GlassPwdr"},
    {MaterialID::Honey, "Honey"},
    {MaterialID::Mud, "Mud"},
    {MaterialID::Blood, "Blood"},
    {MaterialID::Poison, "Poison"},
    {MaterialID::Slime, "Slime"},
    {MaterialID::Milk, "Milk"},
    {MaterialID::Alcohol, "Alcohol"},
    {MaterialID::Mercury, "Mercury"},

    // Page 3: More Liquids + Gases
    {MaterialID::Petrol, "Petrol"},
    {MaterialID::Glue, "Glue"},
    {MaterialID::Toxic_Gas, "ToxicGas"},
    {MaterialID::Hydrogen, "Hydrogen"},
    {MaterialID::Helium, "Helium"},
    {MaterialID::Methane, "Methane"},
    {MaterialID::Spark, "Spark"},
    {MaterialID::Plasma, "Plasma"},
    {MaterialID::Dust, "Dust"},
    {MaterialID::Spore, "Spore"},

    // Page 4: More Gases + Solids
    {MaterialID::Confetti, "Confetti"},
    {MaterialID::Metal, "Metal"},
    {MaterialID::Gold, "Gold"},
    {MaterialID::Ice, "Ice"},
    {MaterialID::Glass, "Glass"},
    {MaterialID::Brick, "Brick"},
    {MaterialID::Obsidian, "Obsidian"},
    {MaterialID::Diamond, "Diamond"},
    {MaterialID::Copper, "Copper"},
    {MaterialID::Rubber, "Rubber"},

    // Page 5: Organic
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

    // Page 6: Special
    {MaterialID::Clone, "Clone"},
    {MaterialID::Void, "Void"},
    {MaterialID::Fuse, "Fuse"},
    {MaterialID::TNT, "TNT"},
    {MaterialID::C4, "C4"},
    {MaterialID::Firework, "Firework"},
    {MaterialID::Lightning, "Lightning"},
    {MaterialID::Portal_In, "PortalIn"},
    {MaterialID::Portal_Out, "PortalOut"},
    {MaterialID::Magic, "Magic"},

    // Page 7: Fantasy
    {MaterialID::Crystal, "Crystal"},
    {MaterialID::Ectoplasm, "Ectoplasm"},
    {MaterialID::Antimatter, "Antimattr"},
    {MaterialID::Fairy_Dust, "FairyDust"},
    {MaterialID::Dragon_Fire, "DragonFire"},
    {MaterialID::Frost, "Frost"},
    {MaterialID::Ember, "Ember"},
    {MaterialID::Stardust, "Stardust"},
    {MaterialID::Void_Dust, "VoidDust"},
};

static const int MATERIALS_PER_PAGE = 10;
static const int TOTAL_MATERIALS = sizeof(ALL_MATERIALS) / sizeof(ALL_MATERIALS[0]);
static const int TOTAL_PAGES = (TOTAL_MATERIALS + MATERIALS_PER_PAGE - 1) / MATERIALS_PER_PAGE;

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
        , current_page_(0) {

        pixel_buffer_.resize(WORLD_WIDTH * WORLD_HEIGHT);
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
    int current_page_;

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

    // Returns: 0 = no click in palette, 1 = material selected, 2 = page changed
    int check_material_palette_click(int32_t mx, int32_t my, MaterialID& clicked_material) {
        // Material palette coordinates (must match render_material_palette)
        int palette_x = WORLD_WIDTH - 160;
        int nav_y = 25;  // Page navigation row
        int materials_start_y = 50;  // Materials start below nav
        int swatch_size = 20;
        int spacing = 28;

        // Check page navigation clicks
        // "< Prev" button area (left side of nav row)
        if (my >= nav_y && my <= nav_y + 15) {
            if (mx >= palette_x && mx <= palette_x + 35 && current_page_ > 0) {
                current_page_--;
                return 2;  // Page changed
            }
            // "Next >" button area (right side of nav row)
            if (mx >= palette_x + 100 && mx <= palette_x + 145 && current_page_ < TOTAL_PAGES - 1) {
                current_page_++;
                return 2;  // Page changed
            }
        }

        // Check material swatch clicks
        int start_idx = current_page_ * MATERIALS_PER_PAGE;
        int end_idx = std::min(start_idx + MATERIALS_PER_PAGE, TOTAL_MATERIALS);

        int item_y = materials_start_y;
        for (int i = start_idx; i < end_idx; i++) {
            // Check if click is within this material's button area (whole row is clickable)
            if (mx >= palette_x - 5 && mx <= palette_x + 150 &&
                my >= item_y && my <= item_y + swatch_size) {
                clicked_material = ALL_MATERIALS[i].id;
                return 1;  // Material selected
            }
            item_y += spacing;
        }

        return 0;  // No click in palette
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

        // Page navigation
        if (input.prev_page) {
            if (current_page_ > 0) {
                current_page_--;
                std::cout << "Page " << (current_page_ + 1) << "/" << TOTAL_PAGES << "\n";
            }
            input.prev_page = false;
        }
        if (input.next_page) {
            if (current_page_ < TOTAL_PAGES - 1) {
                current_page_++;
                std::cout << "Page " << (current_page_ + 1) << "/" << TOTAL_PAGES << "\n";
            }
            input.next_page = false;
        }

        // Check for material palette clicks (only on initial click)
        static bool was_mouse_down = false;
        bool is_mouse_down = input.mouse_left_down;

        if (is_mouse_down && !was_mouse_down) {
            // Mouse just clicked
            MaterialID clicked_mat;
            int click_result = check_material_palette_click(input.mouse_x, input.mouse_y, clicked_mat);
            if (click_result == 1) {
                // Material selected
                input.selected_material = clicked_mat;
                std::cout << "Selected material via UI click\n";
                was_mouse_down = is_mouse_down;
                return;  // Don't place materials when clicking UI
            } else if (click_result == 2) {
                // Page changed
                std::cout << "Page " << (current_page_ + 1) << "/" << TOTAL_PAGES << "\n";
                was_mouse_down = is_mouse_down;
                return;  // Don't place materials when clicking UI
            }
        }

        was_mouse_down = is_mouse_down;

        // Place materials with mouse
        if (input.mouse_left_down || input.mouse_right_down) {
            int32_t x = input.mouse_x;
            int32_t y = input.mouse_y;

            // Don't place materials if clicking in the palette area
            if (x >= WORLD_WIDTH - 165 && y <= 370) {
                return;
            }

            // SPECIAL CASE: People spawn ONE AT A TIME (no brush)
            if (input.mouse_left_down && input.selected_material == MaterialID::Person) {
                // Only spawn person if cell is empty or passable
                if (world_.in_bounds(x, y)) {
                    MaterialID current = world_.get_material(x, y);
                    bool can_spawn = (current == MaterialID::Empty ||
                                     current == MaterialID::Water ||
                                     current == MaterialID::Steam ||
                                     current == MaterialID::Smoke);

                    if (can_spawn) {
                        // Clear cell state
                        Cell& cell = world_.get_cell(x, y);
                        cell.flags = 0;
                        cell.velocity_y = 0;

                        // Spawn single person at cursor
                        world_.set_material(x, y, MaterialID::Person);
                        cell.set_health(100);
                        cell.set_person_facing_right((world_.random_int() & 1) == 0);
                        cell.set_reproduction_cooldown(30);
                    }
                }
                return;  // Skip brush logic for people
            }

            // Use configurable brush for all other materials
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

    void render_material_palette() {
        const uint32_t bg_color = 0xC0000000;  // More opaque black
        const uint32_t text_color = 0xFFFFFFFF;  // White
        const uint32_t selected_border = 0xFFFFFF00;  // Yellow
        const uint32_t highlight_color = 0xFFFFFF00;  // Yellow for selected name
        const uint32_t nav_color = 0xFF00FFFF;  // Cyan for navigation
        const uint32_t dim_color = 0x80808080;  // Dim gray for disabled

        const auto& input = platform_.get_input_state();

        // Material palette - right side of screen
        int palette_x = WORLD_WIDTH - 160;
        int palette_y = 10;
        int swatch_size = 20;
        int spacing = 28;

        // Background panel (extended for all materials)
        draw_filled_rect(palette_x - 5, palette_y - 5, 155, 380, bg_color);

        // Title
        draw_text(palette_x, palette_y, "MATERIALS", text_color);
        palette_y += 15;

        // Page navigation
        // "< Prev" button
        if (current_page_ > 0) {
            draw_text(palette_x, palette_y, "PREV", nav_color);
        } else {
            draw_text(palette_x, palette_y, "PREV", dim_color);
        }

        // Page number
        std::string page_str = std::to_string(current_page_ + 1) + "/" + std::to_string(TOTAL_PAGES);
        draw_text(palette_x + 45, palette_y, page_str, text_color);

        // "Next >" button
        if (current_page_ < TOTAL_PAGES - 1) {
            draw_text(palette_x + 100, palette_y, "NEXT", nav_color);
        } else {
            draw_text(palette_x + 100, palette_y, "NEXT", dim_color);
        }

        palette_y += 25;

        // Materials on current page
        int start_idx = current_page_ * MATERIALS_PER_PAGE;
        int end_idx = std::min(start_idx + MATERIALS_PER_PAGE, TOTAL_MATERIALS);

        for (int i = start_idx; i < end_idx; i++) {
            const MaterialEntry& mat = ALL_MATERIALS[i];

            // Get material color
            Color mat_color = material_system_.get_material(mat.id).base_color;
            uint32_t color = mat_color.to_rgba32();

            // Draw swatch
            draw_filled_rect(palette_x, palette_y, swatch_size, swatch_size, color);

            // Draw border if selected
            if (input.selected_material == mat.id) {
                // Top border
                draw_filled_rect(palette_x - 2, palette_y - 2, swatch_size + 4, 2, selected_border);
                // Bottom border
                draw_filled_rect(palette_x - 2, palette_y + swatch_size, swatch_size + 4, 2, selected_border);
                // Left border
                draw_filled_rect(palette_x - 2, palette_y - 2, 2, swatch_size + 4, selected_border);
                // Right border
                draw_filled_rect(palette_x + swatch_size, palette_y - 2, 2, swatch_size + 4, selected_border);
            }

            // Draw index number (1-based within page)
            int page_idx = i - start_idx;
            std::string idx_str;
            if (page_idx < 10) {
                idx_str = std::to_string(page_idx);
            } else {
                idx_str = " ";
            }
            draw_text(palette_x + swatch_size + 5, palette_y + 2, idx_str, text_color);

            // Draw material name
            draw_text(palette_x + swatch_size + 18, palette_y + 2, std::string(mat.name), text_color);

            palette_y += spacing;
        }

        // Display selected material name prominently at bottom
        palette_y = 355;
        draw_filled_rect(palette_x - 5, palette_y - 5, 155, 25, 0xE0000000);  // Darker background

        // Find selected material name
        const char* selected_name = "Unknown";
        for (int i = 0; i < TOTAL_MATERIALS; i++) {
            if (ALL_MATERIALS[i].id == input.selected_material) {
                selected_name = ALL_MATERIALS[i].name;
                break;
            }
        }

        // Draw "SEL:" label
        draw_text(palette_x, palette_y, "SEL", text_color);

        // Draw selected material name in yellow
        draw_text(palette_x + 32, palette_y, std::string(selected_name), highlight_color);
    }

    void render_debug_gui() {
        const uint32_t bg_color = 0x80000000;  // Semi-transparent black
        const uint32_t text_color = 0xFFFFFFFF;  // White
        const uint32_t warning_color = 0xFF0000FF;  // Red

        // Draw background panel
        draw_filled_rect(5, 5, 250, 100, bg_color);

        // Draw FPS
        int y = 10;
        draw_text(10, y, "FPS: " + std::to_string(static_cast<int>(current_fps_)),
                  current_fps_ < 50.0f ? warning_color : text_color);

        // Draw active chunks
        y += 15;
        draw_text(10, y, "Chunks: " + std::to_string(simulation_.get_active_chunks()), text_color);

        // Draw active cells
        y += 15;
        draw_text(10, y, "Cells: " + std::to_string(active_cells_display_), text_color);

        // Controls help
        y += 20;
        draw_text(10, y, "Tab: Toggle GUI", text_color);
        y += 10;
        draw_text(10, y, "C: Clear World", text_color);

        // Brush info
        y += 15;
        const auto& input = platform_.get_input_state();
        std::string brush_info = "Brush: ";
        brush_info += std::to_string(input.brush_radius);
        brush_info += " ";
        brush_info += (input.brush_shape == BrushShape::Circle) ? "Circle" : "Square";
        draw_text(10, y, brush_info, text_color);
        y += 10;
        draw_text(10, y, "[/]: Size  B: Shape", text_color);
    }

    void render_brush_preview() {
        const auto& input = platform_.get_input_state();
        int32_t x = input.mouse_x;
        int32_t y = input.mouse_y;
        int32_t brush_radius = input.brush_radius;
        BrushShape brush_shape = input.brush_shape;

        // Don't show preview if in UI area
        if (x >= WORLD_WIDTH - 165 && y <= 370) {
            return;
        }

        // Preview color - semi-transparent white
        uint32_t preview_color = 0x80FFFFFF;

        // Draw brush outline
        for (int32_t dy = -brush_radius; dy <= brush_radius; ++dy) {
            for (int32_t dx = -brush_radius; dx <= brush_radius; ++dx) {
                bool is_edge = false;

                if (brush_shape == BrushShape::Circle) {
                    // Circular brush
                    int dist_sq = dx * dx + dy * dy;
                    int radius_sq = brush_radius * brush_radius;

                    // Draw pixel if on the edge (within 1 pixel of radius)
                    if (dist_sq <= radius_sq && dist_sq > (brush_radius - 1) * (brush_radius - 1)) {
                        is_edge = true;
                    }
                } else {
                    // Square brush - draw outline only
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
            pixel_buffer_[y * WORLD_WIDTH + x] = 0xFFFF0000;  // Red center
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
            }
        }
    }

    void render() {
        // Generate color buffer from world state
        world_.generate_color_buffer(pixel_buffer_.data());

        // Enhance people rendering (make them visible with 2x2 size and outline)
        render_enhanced_people();

        // Always show material palette
        render_material_palette();

        // Draw brush preview at cursor
        render_brush_preview();

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
