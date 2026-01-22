#include "DiscoverySystem.h"
#include <cstring>
#include <algorithm>

namespace PixelEngine {

// External reference to combination data (defined in Material.cpp)
// We'll need to expose these from Material.cpp
struct MaterialCombination {
    MaterialID mat_a;
    MaterialID mat_b;
    MaterialID result_a;
    MaterialID result_b;
    int chance;
};

// These will be set by Material.cpp via set_combination_data()
static const MaterialCombination* g_combinations = nullptr;
static int g_num_combinations = 0;

// Called from Material.cpp to share combination data
void set_combination_data(const void* combinations, int count) {
    g_combinations = static_cast<const MaterialCombination*>(combinations);
    g_num_combinations = count;
}

DiscoverySystem::DiscoverySystem() {
    reset_to_starter_set();
}

void DiscoverySystem::reset_to_starter_set() {
    // Clear all unlocks
    unlocked_materials_.reset();

    // Unlock starter materials
    for (int i = 0; i < NUM_STARTER_MATERIALS; i++) {
        unlocked_materials_.set(static_cast<size_t>(STARTER_MATERIALS[i]));
    }

    // Clear discovery log
    discovery_log_.clear();
    new_discoveries_.clear();

    // Initialize combination tracking
    init_combination_tracking();
}

void DiscoverySystem::init_combination_tracking() {
    combination_progress_.clear();

    if (!g_combinations || g_num_combinations == 0) {
        return;
    }

    combination_progress_.reserve(g_num_combinations);

    for (int i = 0; i < g_num_combinations; i++) {
        CombinationProgress progress;
        progress.mat_a = g_combinations[i].mat_a;
        progress.mat_b = g_combinations[i].mat_b;
        progress.result_a = g_combinations[i].result_a;
        progress.result_b = g_combinations[i].result_b;
        progress.hint_level = HintLevel::Hidden;
        progress.nearby_attempts = 0;
        progress.discovered = false;
        combination_progress_.push_back(progress);
    }
}

bool DiscoverySystem::is_material_unlocked(MaterialID id) const {
    return unlocked_materials_.test(static_cast<size_t>(id));
}

int DiscoverySystem::get_unlocked_count() const {
    return static_cast<int>(unlocked_materials_.count());
}

int DiscoverySystem::get_total_materials() const {
    return static_cast<int>(MaterialID::COUNT);
}

bool DiscoverySystem::is_combination_discovered(MaterialID a, MaterialID b) const {
    int idx = find_combination_index(a, b);
    if (idx < 0) return false;
    return combination_progress_[idx].discovered;
}

int DiscoverySystem::get_discovered_combinations() const {
    int count = 0;
    for (const auto& progress : combination_progress_) {
        if (progress.discovered) count++;
    }
    return count;
}

int DiscoverySystem::get_total_combinations() const {
    return static_cast<int>(combination_progress_.size());
}

int DiscoverySystem::find_combination_index(MaterialID a, MaterialID b) const {
    for (size_t i = 0; i < combination_progress_.size(); i++) {
        const auto& p = combination_progress_[i];
        if ((p.mat_a == a && p.mat_b == b) ||
            (p.mat_a == b && p.mat_b == a)) {
            return static_cast<int>(i);
        }
    }
    return -1;
}

bool DiscoverySystem::on_combination_occurred(MaterialID a, MaterialID b,
                                               MaterialID result_a, MaterialID result_b,
                                               uint32_t frame_number) {
    int idx = find_combination_index(a, b);
    if (idx < 0) return false;

    CombinationProgress& progress = combination_progress_[idx];

    if (progress.discovered) {
        return false;  // Already discovered
    }

    // Mark as discovered
    progress.discovered = true;

    // Unlock result materials and create discovery entries
    bool any_new = false;

    if (result_a != MaterialID::Empty && !is_material_unlocked(result_a)) {
        unlock_material(result_a);
        DiscoveryEntry entry(result_a, a, b, frame_number);
        discovery_log_.push_back(entry);
        new_discoveries_.push_back(entry);
        any_new = true;
    }

    if (result_b != MaterialID::Empty && result_b != result_a &&
        !is_material_unlocked(result_b)) {
        unlock_material(result_b);
        DiscoveryEntry entry(result_b, a, b, frame_number);
        discovery_log_.push_back(entry);
        new_discoveries_.push_back(entry);
        any_new = true;
    }

    // Even if no new materials, the combination itself is a discovery
    // (e.g., discovering that Salt + Water = dissolves)
    if (!any_new && (result_a == MaterialID::Empty || is_material_unlocked(result_a)) &&
                    (result_b == MaterialID::Empty || is_material_unlocked(result_b))) {
        // Log the combination discovery even if materials were already unlocked
        DiscoveryEntry entry;
        entry.material_unlocked = MaterialID::Empty;  // No new material
        entry.ingredient_a = a;
        entry.ingredient_b = b;
        entry.frame_number = frame_number;
        entry.discovery_time = std::time(nullptr);
        discovery_log_.push_back(entry);
    }

    // Update hint levels for remaining undiscovered combinations
    update_hint_levels();

    return true;
}

void DiscoverySystem::unlock_material(MaterialID id) {
    unlocked_materials_.set(static_cast<size_t>(id));
}

void DiscoverySystem::unlock_with_popup(MaterialID id, uint32_t frame_number) {
    if (is_material_unlocked(id)) {
        return;  // Already unlocked
    }

    // Unlock the material
    unlock_material(id);

    // Create discovery entry with Empty ingredients (indicates auto-discovered)
    DiscoveryEntry entry(id, MaterialID::Empty, MaterialID::Empty, frame_number);
    discovery_log_.push_back(entry);
    new_discoveries_.push_back(entry);

    // Update hint levels since we have a new material
    update_hint_levels();
}

DiscoveryEntry DiscoverySystem::pop_new_discovery() {
    if (new_discoveries_.empty()) {
        return DiscoveryEntry();
    }
    DiscoveryEntry entry = new_discoveries_.front();
    new_discoveries_.erase(new_discoveries_.begin());
    return entry;
}

void DiscoverySystem::increment_nearby_attempt(MaterialID a, MaterialID b) {
    // Find combinations that use similar materials (same category)
    int cat_a = get_material_category_index(a);
    int cat_b = get_material_category_index(b);

    for (auto& progress : combination_progress_) {
        if (progress.discovered) continue;

        int combo_cat_a = get_material_category_index(progress.mat_a);
        int combo_cat_b = get_material_category_index(progress.mat_b);

        // If categories match (in either order)
        if ((combo_cat_a == cat_a && combo_cat_b == cat_b) ||
            (combo_cat_a == cat_b && combo_cat_b == cat_a)) {
            progress.nearby_attempts++;
        }
    }

    update_hint_levels();
}

void DiscoverySystem::update_hint_levels() {
    for (auto& progress : combination_progress_) {
        if (progress.discovered) continue;

        // Check if player has both required materials
        bool has_both = is_material_unlocked(progress.mat_a) &&
                        is_material_unlocked(progress.mat_b);

        if (has_both) {
            // Player can attempt this - show better hints based on attempts
            if (progress.nearby_attempts > 20) {
                progress.hint_level = HintLevel::Full;
            } else if (progress.nearby_attempts > 10) {
                progress.hint_level = HintLevel::Partial;
            } else {
                progress.hint_level = HintLevel::Category;
            }
        } else if (is_material_unlocked(progress.mat_a) ||
                   is_material_unlocked(progress.mat_b)) {
            // Player has one ingredient
            progress.hint_level = HintLevel::Category;
        } else {
            progress.hint_level = HintLevel::Hidden;
        }
    }
}

std::vector<CombinationProgress> DiscoverySystem::get_available_hints() const {
    std::vector<CombinationProgress> hints;

    for (const auto& progress : combination_progress_) {
        if (!progress.discovered && progress.hint_level != HintLevel::Hidden) {
            hints.push_back(progress);
        }
    }

    // Sort by hint level (better hints first)
    std::sort(hints.begin(), hints.end(),
              [](const CombinationProgress& a, const CombinationProgress& b) {
                  return static_cast<int>(a.hint_level) > static_cast<int>(b.hint_level);
              });

    return hints;
}

std::vector<CombinationProgress> DiscoverySystem::get_hints_for_material(MaterialID mat) const {
    std::vector<CombinationProgress> hints;

    for (const auto& progress : combination_progress_) {
        if (progress.discovered) continue;
        if (progress.mat_a == mat || progress.mat_b == mat) {
            hints.push_back(progress);
        }
    }

    return hints;
}

bool DiscoverySystem::has_potential_discovery(MaterialID mat) const {
    // Check if this material is part of any undiscovered combination
    // where BOTH ingredients are unlocked
    for (const auto& progress : combination_progress_) {
        if (progress.discovered) continue;

        // Check if this material is involved
        bool involves_mat = (progress.mat_a == mat || progress.mat_b == mat);
        if (!involves_mat) continue;

        // Check if both ingredients are unlocked
        bool both_unlocked = is_material_unlocked(progress.mat_a) &&
                             is_material_unlocked(progress.mat_b);
        if (both_unlocked) {
            return true;  // This material can discover something new!
        }
    }
    return false;
}

const char* DiscoverySystem::get_material_category_name(MaterialID id) {
    int idx = static_cast<int>(id);

    // Categories based on MaterialID ranges (matching Types.h organization)
    if (idx == 0) return "Empty";
    if (idx <= 10 || (idx >= 103 && idx <= 112)) return "Basic";
    if ((idx >= 11 && idx <= 20) || (idx >= 81 && idx <= 85) || (idx >= 113 && idx <= 117)) return "Powder";
    if ((idx >= 21 && idx <= 30) || (idx >= 86 && idx <= 90) || (idx >= 118 && idx <= 122)) return "Liquid";
    if ((idx >= 31 && idx <= 40) || (idx >= 91 && idx <= 93) || (idx >= 123 && idx <= 129)) return "Gas";
    if ((idx >= 41 && idx <= 50) || (idx >= 94 && idx <= 97) || (idx >= 130 && idx <= 136)) return "Solid";
    if ((idx >= 51 && idx <= 60) || (idx >= 98 && idx <= 100) || (idx >= 137 && idx <= 143)) return "Organic";
    if ((idx >= 61 && idx <= 70) || (idx >= 101 && idx <= 102) || (idx >= 144 && idx <= 151)) return "Special";
    if ((idx >= 71 && idx <= 80) || (idx >= 152 && idx <= 161)) return "Fantasy";

    return "Unknown";
}

int DiscoverySystem::get_material_category_index(MaterialID id) {
    int idx = static_cast<int>(id);

    if (idx == 0) return -1;  // Empty
    if (idx <= 10 || (idx >= 103 && idx <= 112)) return 0;  // Basic
    if ((idx >= 11 && idx <= 20) || (idx >= 81 && idx <= 85) || (idx >= 113 && idx <= 117)) return 1;  // Powder
    if ((idx >= 21 && idx <= 30) || (idx >= 86 && idx <= 90) || (idx >= 118 && idx <= 122)) return 2;  // Liquid
    if ((idx >= 31 && idx <= 40) || (idx >= 91 && idx <= 93) || (idx >= 123 && idx <= 129)) return 3;  // Gas
    if ((idx >= 41 && idx <= 50) || (idx >= 94 && idx <= 97) || (idx >= 130 && idx <= 136)) return 4;  // Solid
    if ((idx >= 51 && idx <= 60) || (idx >= 98 && idx <= 100) || (idx >= 137 && idx <= 143)) return 5;  // Organic
    if ((idx >= 61 && idx <= 70) || (idx >= 101 && idx <= 102) || (idx >= 144 && idx <= 151)) return 6;  // Special
    if ((idx >= 71 && idx <= 80) || (idx >= 152 && idx <= 161)) return 7;  // Fantasy

    return -1;
}

// Serialization format:
// [4 bytes] num_unlocked_materials
// [32 bytes] unlocked_materials bitset (256 bits)
// [4 bytes] num_discovered_combinations
// [N bytes] discovered combination indices (2 bytes each)
// [4 bytes] num_discovery_log_entries
// [N bytes] discovery log entries

void DiscoverySystem::save_to_buffer(std::vector<uint8_t>& buffer) const {
    // Unlocked materials count
    uint32_t unlocked_count = static_cast<uint32_t>(unlocked_materials_.count());
    buffer.insert(buffer.end(), reinterpret_cast<const uint8_t*>(&unlocked_count),
                  reinterpret_cast<const uint8_t*>(&unlocked_count) + 4);

    // Unlocked materials bitset (32 bytes = 256 bits)
    for (int i = 0; i < 256; i += 8) {
        uint8_t byte = 0;
        for (int j = 0; j < 8; j++) {
            if (unlocked_materials_.test(i + j)) {
                byte |= (1 << j);
            }
        }
        buffer.push_back(byte);
    }

    // Count discovered combinations
    uint32_t discovered_count = 0;
    for (const auto& p : combination_progress_) {
        if (p.discovered) discovered_count++;
    }
    buffer.insert(buffer.end(), reinterpret_cast<const uint8_t*>(&discovered_count),
                  reinterpret_cast<const uint8_t*>(&discovered_count) + 4);

    // Save discovered combination indices
    for (size_t i = 0; i < combination_progress_.size(); i++) {
        if (combination_progress_[i].discovered) {
            uint16_t idx = static_cast<uint16_t>(i);
            buffer.insert(buffer.end(), reinterpret_cast<const uint8_t*>(&idx),
                          reinterpret_cast<const uint8_t*>(&idx) + 2);
        }
    }

    // Discovery log
    uint32_t log_count = static_cast<uint32_t>(discovery_log_.size());
    buffer.insert(buffer.end(), reinterpret_cast<const uint8_t*>(&log_count),
                  reinterpret_cast<const uint8_t*>(&log_count) + 4);

    for (const auto& entry : discovery_log_) {
        buffer.push_back(static_cast<uint8_t>(entry.material_unlocked));
        buffer.push_back(static_cast<uint8_t>(entry.ingredient_a));
        buffer.push_back(static_cast<uint8_t>(entry.ingredient_b));

        uint64_t timestamp = static_cast<uint64_t>(entry.discovery_time);
        buffer.insert(buffer.end(), reinterpret_cast<const uint8_t*>(&timestamp),
                      reinterpret_cast<const uint8_t*>(&timestamp) + 8);

        buffer.insert(buffer.end(), reinterpret_cast<const uint8_t*>(&entry.frame_number),
                      reinterpret_cast<const uint8_t*>(&entry.frame_number) + 4);
    }
}

bool DiscoverySystem::load_from_buffer(const uint8_t* data, size_t size) {
    if (size < 40) return false;  // Minimum size check

    size_t offset = 0;

    // Skip unlocked count (we recalculate from bitset)
    offset += 4;

    // Load unlocked materials bitset
    unlocked_materials_.reset();
    for (int i = 0; i < 256 && offset < size; i += 8) {
        uint8_t byte = data[offset++];
        for (int j = 0; j < 8; j++) {
            if (byte & (1 << j)) {
                unlocked_materials_.set(i + j);
            }
        }
    }

    if (offset + 4 > size) return false;

    // Load discovered combinations
    uint32_t discovered_count;
    std::memcpy(&discovered_count, data + offset, 4);
    offset += 4;

    // Reset combination progress
    init_combination_tracking();

    // Mark discovered combinations
    for (uint32_t i = 0; i < discovered_count && offset + 2 <= size; i++) {
        uint16_t idx;
        std::memcpy(&idx, data + offset, 2);
        offset += 2;

        if (idx < combination_progress_.size()) {
            combination_progress_[idx].discovered = true;
        }
    }

    if (offset + 4 > size) return false;

    // Load discovery log
    uint32_t log_count;
    std::memcpy(&log_count, data + offset, 4);
    offset += 4;

    discovery_log_.clear();
    discovery_log_.reserve(log_count);

    for (uint32_t i = 0; i < log_count && offset + 15 <= size; i++) {
        DiscoveryEntry entry;
        entry.material_unlocked = static_cast<MaterialID>(data[offset++]);
        entry.ingredient_a = static_cast<MaterialID>(data[offset++]);
        entry.ingredient_b = static_cast<MaterialID>(data[offset++]);

        uint64_t timestamp;
        std::memcpy(&timestamp, data + offset, 8);
        entry.discovery_time = static_cast<time_t>(timestamp);
        offset += 8;

        std::memcpy(&entry.frame_number, data + offset, 4);
        offset += 4;

        discovery_log_.push_back(entry);
    }

    // Update hint levels based on loaded state
    update_hint_levels();

    return true;
}

} // namespace PixelEngine

// External function to be called from Material.cpp
extern "C" void PixelEngine_set_combination_data(const void* combinations, int count) {
    PixelEngine::set_combination_data(combinations, count);
}
