#include "Simulation.h"

namespace PixelEngine {

Simulation::Simulation(World& world)
    : world_(world)
    , material_system_(world.get_material_system())
    , frame_count_(0)
    , active_chunk_count_(0)
    , updated_cell_count_(0)
    , scan_direction_(false) {
}

void Simulation::update() {
    ++frame_count_;
    active_chunk_count_ = 0;
    updated_cell_count_ = 0;

    // Scan bottom-to-top (gravity simulation)
    // Alternate left-right scan direction each frame for better dispersion
    scan_direction_ = !scan_direction_;

    for (int32_t chunk_y = world_.get_chunks_high() - 1; chunk_y >= 0; --chunk_y) {
        if (scan_direction_) {
            // Scan left to right
            for (int32_t chunk_x = 0; chunk_x < world_.get_chunks_wide(); ++chunk_x) {
                Chunk* chunk = world_.get_chunk(chunk_x, chunk_y);
                if (chunk && chunk->is_active) {
                    update_chunk(chunk, chunk_x, chunk_y);
                    ++active_chunk_count_;
                }
            }
        } else {
            // Scan right to left
            for (int32_t chunk_x = world_.get_chunks_wide() - 1; chunk_x >= 0; --chunk_x) {
                Chunk* chunk = world_.get_chunk(chunk_x, chunk_y);
                if (chunk && chunk->is_active) {
                    update_chunk(chunk, chunk_x, chunk_y);
                    ++active_chunk_count_;
                }
            }
        }
    }

    // Clear updated flags for next frame
    world_.clear_updated_flags();
}

void Simulation::update_chunk(Chunk* chunk, int32_t chunk_x, int32_t chunk_y) {
    bool chunk_had_movement = false;

    // Calculate world-space base coordinates for this chunk
    int32_t base_x = chunk_x * CHUNK_SIZE;
    int32_t base_y = chunk_y * CHUNK_SIZE;

    // Update cells in priority order: Gases → Liquids → Powders → Solids
    // Each pass processes bottom-to-top within the chunk
    MaterialState priority_order[] = {
        MaterialState::Gas,
        MaterialState::Liquid,
        MaterialState::Powder,
        MaterialState::Solid
    };

    for (MaterialState target_state : priority_order) {
        // Update cells bottom-to-top within chunk for this material type
        for (int32_t local_y = CHUNK_SIZE - 1; local_y >= 0; --local_y) {
            // Alternate scan direction for better dispersion
            if (scan_direction_) {
                // Left to right
                for (int32_t local_x = 0; local_x < CHUNK_SIZE; ++local_x) {
                    int32_t world_x = base_x + local_x;
                    int32_t world_y = base_y + local_y;

                    // Skip if out of world bounds
                    if (!world_.in_bounds(world_x, world_y)) {
                        continue;
                    }

                    Cell& cell = chunk->get_cell(local_x, local_y);
                    MaterialID material = cell.material_id;

                    // Skip empty cells and already-updated cells
                    if (material == MaterialID::Empty || cell.was_updated()) {
                        continue;
                    }

                    // Get material state
                    const auto& mat_def = material_system_.get_material(material);

                    // Only update materials of the current priority type
                    if (mat_def.state == target_state) {
                        // Store pre-update state
                        MaterialID old_material = world_.get_material(world_x, world_y);

                        // Update cell based on material type
                        update_cell(world_x, world_y, material);

                        // Check if cell changed (moved or transformed)
                        MaterialID new_material = world_.get_material(world_x, world_y);
                        if (new_material != old_material) {
                            chunk_had_movement = true;
                            ++updated_cell_count_;
                        }
                    }
                }
            } else {
                // Right to left
                for (int32_t local_x = CHUNK_SIZE - 1; local_x >= 0; --local_x) {
                    int32_t world_x = base_x + local_x;
                    int32_t world_y = base_y + local_y;

                    if (!world_.in_bounds(world_x, world_y)) {
                        continue;
                    }

                    Cell& cell = chunk->get_cell(local_x, local_y);
                    MaterialID material = cell.material_id;

                    if (material == MaterialID::Empty || cell.was_updated()) {
                        continue;
                    }

                    const auto& mat_def = material_system_.get_material(material);

                    // Only update materials of the current priority type
                    if (mat_def.state == target_state) {
                        // Store pre-update state
                        MaterialID old_material = world_.get_material(world_x, world_y);

                        // Update cell based on material type
                        update_cell(world_x, world_y, material);

                        // Check if cell changed (moved or transformed)
                        MaterialID new_material = world_.get_material(world_x, world_y);
                        if (new_material != old_material) {
                            chunk_had_movement = true;
                            ++updated_cell_count_;
                        }
                    }
                }
            }
        }
    }

    // Update chunk sleep state
    if (chunk_had_movement) {
        chunk->sleep_counter = 0;
        chunk->is_active = true;

        // Activate neighboring chunks (movement may affect them)
        world_.activate_chunk(chunk_x - 1, chunk_y);
        world_.activate_chunk(chunk_x + 1, chunk_y);
        world_.activate_chunk(chunk_x, chunk_y - 1);
        world_.activate_chunk(chunk_x, chunk_y + 1);
    } else {
        ++chunk->sleep_counter;

        // Put chunk to sleep if no movement for a while
        if (chunk->sleep_counter >= CHUNK_SLEEP_THRESHOLD) {
            chunk->is_active = false;
        }
    }
}

void Simulation::update_cell(int32_t x, int32_t y, MaterialID material) {
    // Call the appropriate update function based on material type
    switch (material) {
        case MaterialID::Empty:
            Materials::update_empty(world_, x, y);
            break;

        case MaterialID::Stone:
            Materials::update_stone(world_, x, y);
            break;

        case MaterialID::Sand:
            Materials::update_sand(world_, x, y);
            break;

        case MaterialID::Water:
            Materials::update_water(world_, x, y);
            break;

        case MaterialID::Steam:
            Materials::update_steam(world_, x, y);
            break;

        case MaterialID::Oil:
            Materials::update_oil(world_, x, y);
            break;

        case MaterialID::Fire:
            Materials::update_fire(world_, x, y);
            break;

        case MaterialID::Wood:
            Materials::update_wood(world_, x, y);
            break;

        case MaterialID::Acid:
            Materials::update_acid(world_, x, y);
            break;

        case MaterialID::Lava:
            Materials::update_lava(world_, x, y);
            break;

        case MaterialID::Ash:
            Materials::update_ash(world_, x, y);
            break;

        case MaterialID::Grass:
            Materials::update_grass(world_, x, y);
            break;

        case MaterialID::Smoke:
            Materials::update_smoke(world_, x, y);
            break;

        case MaterialID::Person:
            Materials::update_person(world_, x, y);
            break;

        // ====== POWDERS (10-19) ======
        case MaterialID::Dirt:
            Materials::update_dirt(world_, x, y);
            break;
        case MaterialID::Gravel:
            Materials::update_gravel(world_, x, y);
            break;
        case MaterialID::Snow:
            Materials::update_snow(world_, x, y);
            break;
        case MaterialID::Gunpowder:
            Materials::update_gunpowder(world_, x, y);
            break;
        case MaterialID::Salt:
            Materials::update_salt(world_, x, y);
            break;
        case MaterialID::Coal:
            Materials::update_coal(world_, x, y);
            break;
        case MaterialID::Rust:
            Materials::update_rust(world_, x, y);
            break;
        case MaterialID::Sawdust:
            Materials::update_sawdust(world_, x, y);
            break;
        case MaterialID::Glass_Powder:
            Materials::update_glass_powder(world_, x, y);
            break;

        // ====== LIQUIDS (20-29) ======
        case MaterialID::Honey:
            Materials::update_honey(world_, x, y);
            break;
        case MaterialID::Mud:
            Materials::update_mud(world_, x, y);
            break;
        case MaterialID::Blood:
            Materials::update_blood(world_, x, y);
            break;
        case MaterialID::Poison:
            Materials::update_poison(world_, x, y);
            break;
        case MaterialID::Slime:
            Materials::update_slime(world_, x, y);
            break;
        case MaterialID::Milk:
            Materials::update_milk(world_, x, y);
            break;
        case MaterialID::Alcohol:
            Materials::update_alcohol(world_, x, y);
            break;
        case MaterialID::Mercury:
            Materials::update_mercury(world_, x, y);
            break;
        case MaterialID::Petrol:
            Materials::update_petrol(world_, x, y);
            break;
        case MaterialID::Glue:
            Materials::update_glue(world_, x, y);
            break;

        // ====== GASES (30-39) ======
        case MaterialID::Toxic_Gas:
            Materials::update_toxic_gas(world_, x, y);
            break;
        case MaterialID::Hydrogen:
            Materials::update_hydrogen(world_, x, y);
            break;
        case MaterialID::Helium:
            Materials::update_helium(world_, x, y);
            break;
        case MaterialID::Methane:
            Materials::update_methane(world_, x, y);
            break;
        case MaterialID::Spark:
            Materials::update_spark(world_, x, y);
            break;
        case MaterialID::Plasma:
            Materials::update_plasma(world_, x, y);
            break;
        case MaterialID::Dust:
            Materials::update_dust(world_, x, y);
            break;
        case MaterialID::Spore:
            Materials::update_spore(world_, x, y);
            break;
        case MaterialID::Confetti:
            Materials::update_confetti(world_, x, y);
            break;

        // ====== SOLIDS (40-49) ======
        case MaterialID::Metal:
            Materials::update_metal(world_, x, y);
            break;
        case MaterialID::Gold:
            Materials::update_gold(world_, x, y);
            break;
        case MaterialID::Ice:
            Materials::update_ice(world_, x, y);
            break;
        case MaterialID::Glass:
            Materials::update_glass(world_, x, y);
            break;
        case MaterialID::Brick:
            Materials::update_brick(world_, x, y);
            break;
        case MaterialID::Obsidian:
            Materials::update_obsidian(world_, x, y);
            break;
        case MaterialID::Diamond:
            Materials::update_diamond(world_, x, y);
            break;
        case MaterialID::Copper:
            Materials::update_copper(world_, x, y);
            break;
        case MaterialID::Rubber:
            Materials::update_rubber(world_, x, y);
            break;

        // ====== ORGANIC (50-59) ======
        case MaterialID::Leaf:
            Materials::update_leaf(world_, x, y);
            break;
        case MaterialID::Moss:
            Materials::update_moss(world_, x, y);
            break;
        case MaterialID::Vine:
            Materials::update_vine(world_, x, y);
            break;
        case MaterialID::Fungus:
            Materials::update_fungus(world_, x, y);
            break;
        case MaterialID::Seed:
            Materials::update_seed(world_, x, y);
            break;
        case MaterialID::Flower:
            Materials::update_flower(world_, x, y);
            break;
        case MaterialID::Algae:
            Materials::update_algae(world_, x, y);
            break;
        case MaterialID::Coral:
            Materials::update_coral(world_, x, y);
            break;
        case MaterialID::Wax:
            Materials::update_wax(world_, x, y);
            break;
        case MaterialID::Flesh:
            Materials::update_flesh(world_, x, y);
            break;

        // ====== SPECIAL (60-69) ======
        case MaterialID::Clone:
            Materials::update_clone(world_, x, y);
            break;
        case MaterialID::Void:
            Materials::update_void(world_, x, y);
            break;
        case MaterialID::Fuse:
            Materials::update_fuse(world_, x, y);
            break;
        case MaterialID::TNT:
            Materials::update_tnt(world_, x, y);
            break;
        case MaterialID::C4:
            Materials::update_c4(world_, x, y);
            break;
        case MaterialID::Firework:
            Materials::update_firework(world_, x, y);
            break;
        case MaterialID::Lightning:
            Materials::update_lightning(world_, x, y);
            break;
        case MaterialID::Portal_In:
            Materials::update_portal_in(world_, x, y);
            break;
        case MaterialID::Portal_Out:
            Materials::update_portal_out(world_, x, y);
            break;

        // ====== FANTASY (70-79) ======
        case MaterialID::Magic:
            Materials::update_magic(world_, x, y);
            break;
        case MaterialID::Crystal:
            Materials::update_crystal(world_, x, y);
            break;
        case MaterialID::Ectoplasm:
            Materials::update_ectoplasm(world_, x, y);
            break;
        case MaterialID::Antimatter:
            Materials::update_antimatter(world_, x, y);
            break;
        case MaterialID::Fairy_Dust:
            Materials::update_fairy_dust(world_, x, y);
            break;
        case MaterialID::Dragon_Fire:
            Materials::update_dragon_fire(world_, x, y);
            break;
        case MaterialID::Frost:
            Materials::update_frost(world_, x, y);
            break;
        case MaterialID::Ember:
            Materials::update_ember(world_, x, y);
            break;
        case MaterialID::Stardust:
            Materials::update_stardust(world_, x, y);
            break;
        case MaterialID::Void_Dust:
            Materials::update_void_dust(world_, x, y);
            break;
        case MaterialID::Life:
            Materials::update_life(world_, x, y);
            break;

        default:
            break;
    }
}

} // namespace PixelEngine
