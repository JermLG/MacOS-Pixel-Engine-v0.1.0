#pragma once

#include "Types.h"
#include <bitset>
#include <vector>
#include <array>
#include <ctime>

namespace PixelEngine {

// 256 bits to track all possible materials (MaterialID is uint8_t)
using MaterialBitset = std::bitset<256>;

// Hint progression for undiscovered combinations
enum class HintLevel : uint8_t {
    Hidden = 0,      // No hint shown
    Category = 1,    // "Try mixing [Powder] with [Liquid]"
    Partial = 2,     // Shows one ingredient: "Sand + ???"
    Full = 3         // Shows both ingredients (after many attempts)
};

// Tracks progress toward discovering a specific combination
struct CombinationProgress {
    MaterialID mat_a;
    MaterialID mat_b;
    MaterialID result_a;
    MaterialID result_b;
    HintLevel hint_level;
    uint16_t nearby_attempts;   // Times player tried materials in same categories
    bool discovered;

    CombinationProgress()
        : mat_a(MaterialID::Empty)
        , mat_b(MaterialID::Empty)
        , result_a(MaterialID::Empty)
        , result_b(MaterialID::Empty)
        , hint_level(HintLevel::Hidden)
        , nearby_attempts(0)
        , discovered(false) {}
};

// Journal entry for a discovery
struct DiscoveryEntry {
    MaterialID material_unlocked;   // The new material discovered
    MaterialID ingredient_a;
    MaterialID ingredient_b;
    time_t discovery_time;
    uint32_t frame_number;

    DiscoveryEntry()
        : material_unlocked(MaterialID::Empty)
        , ingredient_a(MaterialID::Empty)
        , ingredient_b(MaterialID::Empty)
        , discovery_time(0)
        , frame_number(0) {}

    DiscoveryEntry(MaterialID unlocked, MaterialID a, MaterialID b, uint32_t frame)
        : material_unlocked(unlocked)
        , ingredient_a(a)
        , ingredient_b(b)
        , discovery_time(std::time(nullptr))
        , frame_number(frame) {}
};

// Starter materials for Story Mode (these are unlocked from the beginning)
// Chosen to enable meaningful early discoveries
static const MaterialID STARTER_MATERIALS[] = {
    MaterialID::Empty,      // Always available (eraser)
    MaterialID::Stone,      // Basic solid for building walls
    MaterialID::Sand,       // Classic powder, makes glass with lava
    MaterialID::Water,      // Essential liquid
    MaterialID::Fire,       // Burning, reactions
    MaterialID::Wood,       // Burns, floats
    MaterialID::Acid,       // Corrosive, many reactions
    MaterialID::Lava,       // Hot, makes obsidian/glass
    MaterialID::Dirt,       // Combines with water for mud
    MaterialID::Snow,       // Compacts to ice
    MaterialID::Salt,       // Dissolves
    MaterialID::Seed,       // Plant growth
    MaterialID::Coal,       // Fuel, combinations
    MaterialID::Oil,        // Floats, burns
    MaterialID::Ice,        // Melts, freezes
    MaterialID::Spark,      // Ignition source
};
static const int NUM_STARTER_MATERIALS = sizeof(STARTER_MATERIALS) / sizeof(STARTER_MATERIALS[0]);

class DiscoverySystem {
public:
    DiscoverySystem();

    // Initialize/reset to starter materials
    void reset_to_starter_set();

    // Query unlocked status
    bool is_material_unlocked(MaterialID id) const;
    int get_unlocked_count() const;
    int get_total_materials() const;

    // Query discovery status
    bool is_combination_discovered(MaterialID a, MaterialID b) const;
    int get_discovered_combinations() const;
    int get_total_combinations() const;

    // Called when a combination successfully occurs
    // Returns true if this was a NEW discovery
    bool on_combination_occurred(MaterialID a, MaterialID b,
                                  MaterialID result_a, MaterialID result_b,
                                  uint32_t frame_number);

    // Manual unlock (for achievements, debugging)
    void unlock_material(MaterialID id);

    // Unlock material and queue discovery popup (for safety net auto-discovery)
    void unlock_with_popup(MaterialID id, uint32_t frame_number);

    // Get newly discovered entries (call and clear)
    bool has_new_discovery() const { return !new_discoveries_.empty(); }
    DiscoveryEntry pop_new_discovery();
    const std::vector<DiscoveryEntry>& get_all_discoveries() const { return discovery_log_; }

    // Hint system
    std::vector<CombinationProgress> get_available_hints() const;
    std::vector<CombinationProgress> get_hints_for_material(MaterialID mat) const;
    void increment_nearby_attempt(MaterialID a, MaterialID b);

    // Check if a material can be combined with another unlocked material
    // to discover something new (for UI glow effect)
    bool has_potential_discovery(MaterialID mat) const;

    // Get category name for a material (for hints)
    static const char* get_material_category_name(MaterialID id);
    static int get_material_category_index(MaterialID id);

    // Serialization
    void save_to_buffer(std::vector<uint8_t>& buffer) const;
    bool load_from_buffer(const uint8_t* data, size_t size);

private:
    MaterialBitset unlocked_materials_;
    std::vector<CombinationProgress> combination_progress_;
    std::vector<DiscoveryEntry> discovery_log_;
    std::vector<DiscoveryEntry> new_discoveries_;  // Queue for popups

    void init_combination_tracking();
    int find_combination_index(MaterialID a, MaterialID b) const;
    void update_hint_levels();
};

// Initialize combination data from Material.cpp
void set_combination_data(const void* combinations, int count);

} // namespace PixelEngine
