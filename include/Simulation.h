#pragma once

#include "World.h"
#include "Material.h"

namespace PixelEngine {

// Simulation system - runs the cellular automata update loop
class Simulation {
public:
    explicit Simulation(World& world);

    // Run one simulation step
    void update();

    // Statistics
    uint64_t get_frame_count() const { return frame_count_; }
    uint32_t get_active_chunks() const { return active_chunk_count_; }
    uint32_t get_updated_cells() const { return updated_cell_count_; }

private:
    World& world_;
    MaterialSystem& material_system_;

    uint64_t frame_count_;
    uint32_t active_chunk_count_;
    uint32_t updated_cell_count_;

    bool scan_direction_;  // Alternate scan direction each frame

    // Update a single chunk
    void update_chunk(Chunk* chunk, int32_t chunk_x, int32_t chunk_y);

    // Update a single cell based on its material type
    void update_cell(int32_t x, int32_t y, MaterialID material);

    // Sleep inactive chunks after N frames
    static constexpr uint32_t CHUNK_SLEEP_THRESHOLD = 120;  // ~2 seconds
};

} // namespace PixelEngine
