#include "World.h"
#include <cstring>
#include <random>

namespace PixelEngine {

World::World(int32_t width, int32_t height, MaterialSystem& material_system)
    : width_(width)
    , height_(height)
    , material_system_(material_system)
    , rng_state_(std::random_device{}()) {

    // Calculate chunk grid dimensions
    chunks_wide_ = (width + CHUNK_SIZE - 1) / CHUNK_SIZE;
    chunks_high_ = (height + CHUNK_SIZE - 1) / CHUNK_SIZE;

    // Allocate chunks
    chunks_.resize(chunks_wide_ * chunks_high_);
}

Cell& World::get_cell(int32_t x, int32_t y) {
    int32_t chunk_x = x / CHUNK_SIZE;
    int32_t chunk_y = y / CHUNK_SIZE;
    int32_t local_x = x % CHUNK_SIZE;
    int32_t local_y = y % CHUNK_SIZE;

    Chunk& chunk = chunks_[chunk_y * chunks_wide_ + chunk_x];
    return chunk.get_cell(local_x, local_y);
}

const Cell& World::get_cell(int32_t x, int32_t y) const {
    int32_t chunk_x = x / CHUNK_SIZE;
    int32_t chunk_y = y / CHUNK_SIZE;
    int32_t local_x = x % CHUNK_SIZE;
    int32_t local_y = y % CHUNK_SIZE;

    const Chunk& chunk = chunks_[chunk_y * chunks_wide_ + chunk_x];
    return chunk.get_cell(local_x, local_y);
}

MaterialID World::get_material(int32_t x, int32_t y) const {
    if (!in_bounds(x, y)) {
        return MaterialID::Stone;  // Out of bounds = solid wall
    }
    return get_cell(x, y).material_id;
}

void World::set_material(int32_t x, int32_t y, MaterialID material) {
    if (!in_bounds(x, y)) {
        return;
    }

    get_cell(x, y).material_id = material;
    activate_chunk_at_position(x, y);

    // Activate neighboring chunks if on chunk boundary
    if (x % CHUNK_SIZE == 0 && x > 0) {
        activate_chunk_at_position(x - 1, y);
    }
    if (x % CHUNK_SIZE == CHUNK_SIZE - 1 && x < width_ - 1) {
        activate_chunk_at_position(x + 1, y);
    }
    if (y % CHUNK_SIZE == 0 && y > 0) {
        activate_chunk_at_position(x, y - 1);
    }
    if (y % CHUNK_SIZE == CHUNK_SIZE - 1 && y < height_ - 1) {
        activate_chunk_at_position(x, y + 1);
    }
}

bool World::can_move_to(int32_t x, int32_t y, int32_t new_x, int32_t new_y) const {
    if (!in_bounds(new_x, new_y)) {
        return false;
    }

    MaterialID current_material = get_material(x, y);
    MaterialID target_material = get_material(new_x, new_y);

    // Can always move into empty space
    if (target_material == MaterialID::Empty) {
        return true;
    }

    // Get material properties
    const auto& current_def = material_system_.get_material(current_material);
    const auto& target_def = material_system_.get_material(target_material);

    // Solids can't be displaced
    if (target_def.state == MaterialState::Solid) {
        return false;
    }

    // Denser materials displace lighter ones (when moving down)
    if (new_y > y && current_def.density > target_def.density) {
        return true;
    }

    // Lighter materials (gases) displace heavier ones (when moving up)
    if (new_y < y && current_def.density < target_def.density) {
        return true;
    }

    return false;
}

bool World::try_move_cell(int32_t x, int32_t y, int32_t new_x, int32_t new_y) {
    if (!can_move_to(x, y, new_x, new_y)) {
        return false;
    }

    // Don't move if already updated this frame (prevents double-updates)
    if (get_cell(x, y).was_updated()) {
        return false;
    }

    swap_cells(x, y, new_x, new_y);

    // Mark as updated
    get_cell(new_x, new_y).mark_updated();

    // Activate destination chunk
    activate_chunk_at_position(new_x, new_y);

    return true;
}

void World::swap_cells(int32_t x1, int32_t y1, int32_t x2, int32_t y2) {
    Cell& cell1 = get_cell(x1, y1);
    Cell& cell2 = get_cell(x2, y2);

    // Swap entire cell contents (material_id, flags, velocity_y)
    // This preserves all per-cell state like health, lifetime, direction
    Cell temp = cell1;
    cell1 = cell2;
    cell2 = temp;
}

Chunk* World::get_chunk(int32_t chunk_x, int32_t chunk_y) {
    if (chunk_x < 0 || chunk_x >= chunks_wide_ ||
        chunk_y < 0 || chunk_y >= chunks_high_) {
        return nullptr;
    }
    return &chunks_[chunk_y * chunks_wide_ + chunk_x];
}

const Chunk* World::get_chunk(int32_t chunk_x, int32_t chunk_y) const {
    if (chunk_x < 0 || chunk_x >= chunks_wide_ ||
        chunk_y < 0 || chunk_y >= chunks_high_) {
        return nullptr;
    }
    return &chunks_[chunk_y * chunks_wide_ + chunk_x];
}

void World::activate_chunk(int32_t chunk_x, int32_t chunk_y) {
    Chunk* chunk = get_chunk(chunk_x, chunk_y);
    if (chunk) {
        chunk->is_active = true;
        chunk->sleep_counter = 0;
    }
}

void World::activate_chunk_at_position(int32_t world_x, int32_t world_y) {
    if (!in_bounds(world_x, world_y)) {
        return;
    }
    int32_t chunk_x = world_x / CHUNK_SIZE;
    int32_t chunk_y = world_y / CHUNK_SIZE;
    activate_chunk(chunk_x, chunk_y);
}

void World::clear_updated_flags() {
    for (auto& chunk : chunks_) {
        if (!chunk.is_active) {
            continue;
        }

        for (int32_t i = 0; i < CHUNK_SIZE * CHUNK_SIZE; ++i) {
            chunk.cells[i].clear_updated();
        }
    }
}

void World::clear_world() {
    // Clear all chunks - O(active chunks) operation
    for (auto& chunk : chunks_) {
        // Only clear active chunks for performance
        if (chunk.is_active) {
            for (int32_t i = 0; i < CHUNK_SIZE * CHUNK_SIZE; ++i) {
                chunk.cells[i] = Cell(MaterialID::Empty);
            }
        }
        // Deactivate all chunks
        chunk.is_active = false;
        chunk.sleep_counter = 0;
    }
}

void World::generate_color_buffer(uint32_t* buffer) const {
    for (int32_t y = 0; y < height_; ++y) {
        for (int32_t x = 0; x < width_; ++x) {
            MaterialID material = get_material(x, y);
            const auto& mat_def = material_system_.get_material(material);

            // Use base color (no variance for rendering - set during placement)
            Color color = mat_def.base_color;

            // Write to buffer (row-major order)
            buffer[y * width_ + x] = color.to_rgba32();
        }
    }
}

} // namespace PixelEngine
