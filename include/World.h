#pragma once

#include "Types.h"
#include "Material.h"
#include <vector>
#include <memory>
#include <cstdint>

namespace PixelEngine {

// Chunk of cells (64×64 grid)
struct Chunk {
    Cell cells[CHUNK_SIZE * CHUNK_SIZE];
    bool is_active;  // Does this chunk have any moving materials?
    uint32_t sleep_counter;  // Frames since last movement

    Chunk() : is_active(false), sleep_counter(0) {
        // Initialize all cells to empty
        for (int32_t i = 0; i < CHUNK_SIZE * CHUNK_SIZE; ++i) {
            cells[i] = Cell(MaterialID::Empty);
        }
    }

    // Get cell at local chunk coordinates (0-63)
    Cell& get_cell(int32_t local_x, int32_t local_y) {
        return cells[local_y * CHUNK_SIZE + local_x];
    }

    const Cell& get_cell(int32_t local_x, int32_t local_y) const {
        return cells[local_y * CHUNK_SIZE + local_x];
    }
};

// World - manages the entire simulation grid
class World {
public:
    World(int32_t width, int32_t height, MaterialSystem& material_system);
    ~World() = default;

    // Dimensions
    int32_t get_width() const { return width_; }
    int32_t get_height() const { return height_; }

    // Cell access
    Cell& get_cell(int32_t x, int32_t y);
    const Cell& get_cell(int32_t x, int32_t y) const;

    MaterialID get_material(int32_t x, int32_t y) const;
    void set_material(int32_t x, int32_t y, MaterialID material);

    // Bounds checking
    bool in_bounds(int32_t x, int32_t y) const {
        return x >= 0 && x < width_ && y >= 0 && y < height_;
    }

    // Movement and swapping (used by material update functions)
    bool can_move_to(int32_t x, int32_t y, int32_t new_x, int32_t new_y) const;
    bool try_move_cell(int32_t x, int32_t y, int32_t new_x, int32_t new_y);
    void swap_cells(int32_t x1, int32_t y1, int32_t x2, int32_t y2);

    // Chunk access
    Chunk* get_chunk(int32_t chunk_x, int32_t chunk_y);
    const Chunk* get_chunk(int32_t chunk_x, int32_t chunk_y) const;

    void activate_chunk(int32_t chunk_x, int32_t chunk_y);
    void activate_chunk_at_position(int32_t world_x, int32_t world_y);

    int32_t get_chunks_wide() const { return chunks_wide_; }
    int32_t get_chunks_high() const { return chunks_high_; }

    // Clear updated flags (called at end of frame)
    void clear_updated_flags();

    // Clear entire world back to empty
    void clear_world();

    // Random number generator for deterministic simulation
    uint32_t random_int() {
        // Simple xorshift PRNG (fast, deterministic)
        rng_state_ ^= rng_state_ << 13;
        rng_state_ ^= rng_state_ >> 17;
        rng_state_ ^= rng_state_ << 5;
        return rng_state_;
    }

    // Rendering - generate color buffer
    void generate_color_buffer(uint32_t* buffer) const;

    MaterialSystem& get_material_system() { return material_system_; }

private:
    int32_t width_;
    int32_t height_;
    int32_t chunks_wide_;
    int32_t chunks_high_;

    std::vector<Chunk> chunks_;
    MaterialSystem& material_system_;

    uint32_t rng_state_;

    // Convert world coordinates to chunk index
    int32_t world_to_chunk_index(int32_t x, int32_t y) const {
        int32_t chunk_x = x / CHUNK_SIZE;
        int32_t chunk_y = y / CHUNK_SIZE;
        return chunk_y * chunks_wide_ + chunk_x;
    }
};

} // namespace PixelEngine
