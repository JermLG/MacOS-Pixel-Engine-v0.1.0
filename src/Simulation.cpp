#include "Simulation.h"
#include <algorithm>

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

    // Pre-calculate bounds
    int32_t max_local_x = std::min(CHUNK_SIZE, world_.get_width() - base_x);
    int32_t max_local_y = std::min(CHUNK_SIZE, world_.get_height() - base_y);

    // Process cells bottom-to-top within chunk
    for (int32_t local_y = max_local_y - 1; local_y >= 0; --local_y) {
        int32_t world_y = base_y + local_y;

        if (scan_direction_) {
            for (int32_t local_x = 0; local_x < max_local_x; ++local_x) {
                Cell& cell = chunk->cells[local_y * CHUNK_SIZE + local_x];
                MaterialID material = cell.material_id;

                // Skip empty and already-updated cells
                if (material == MaterialID::Empty || cell.was_updated()) continue;

                int32_t world_x = base_x + local_x;
                update_cell(world_x, world_y, material);

                // Check if cell changed
                if (chunk->cells[local_y * CHUNK_SIZE + local_x].material_id != material) {
                    chunk_had_movement = true;
                    ++updated_cell_count_;
                }
            }
        } else {
            for (int32_t local_x = max_local_x - 1; local_x >= 0; --local_x) {
                Cell& cell = chunk->cells[local_y * CHUNK_SIZE + local_x];
                MaterialID material = cell.material_id;

                if (material == MaterialID::Empty || cell.was_updated()) continue;

                int32_t world_x = base_x + local_x;
                update_cell(world_x, world_y, material);

                if (chunk->cells[local_y * CHUNK_SIZE + local_x].material_id != material) {
                    chunk_had_movement = true;
                    ++updated_cell_count_;
                }
            }
        }
    }

    // Update chunk sleep state
    if (chunk_had_movement) {
        chunk->sleep_counter = 0;
        chunk->is_active = true;
        world_.activate_chunk(chunk_x - 1, chunk_y);
        world_.activate_chunk(chunk_x + 1, chunk_y);
        world_.activate_chunk(chunk_x, chunk_y - 1);
        world_.activate_chunk(chunk_x, chunk_y + 1);
    } else {
        ++chunk->sleep_counter;
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

        // ====== NEW POWDERS (81-85) ======
        case MaterialID::Thermite_Powder:
            Materials::update_thermite_powder(world_, x, y);
            break;
        case MaterialID::Sugar:
            Materials::update_sugar(world_, x, y);
            break;
        case MaterialID::Iron_Filings:
            Materials::update_iron_filings(world_, x, y);
            break;
        case MaterialID::Chalk:
            Materials::update_chalk(world_, x, y);
            break;
        case MaterialID::Calcium:
            Materials::update_calcium(world_, x, y);
            break;

        // ====== NEW LIQUIDS (86-90) ======
        case MaterialID::Tar:
            Materials::update_tar(world_, x, y);
            break;
        case MaterialID::Juice:
            Materials::update_juice(world_, x, y);
            break;
        case MaterialID::Sap:
            Materials::update_sap(world_, x, y);
            break;
        case MaterialID::Bleach:
            Materials::update_bleach(world_, x, y);
            break;
        case MaterialID::Ink:
            Materials::update_ink(world_, x, y);
            break;

        // ====== NEW GASES (91-93) ======
        case MaterialID::Chlorine:
            Materials::update_chlorine(world_, x, y);
            break;
        case MaterialID::Liquid_Nitrogen:
            Materials::update_liquid_nitrogen(world_, x, y);
            break;
        case MaterialID::Oxygen:
            Materials::update_oxygen(world_, x, y);
            break;

        // ====== NEW SOLIDS (94-97) ======
        case MaterialID::Concrete:
            Materials::update_concrete(world_, x, y);
            break;
        case MaterialID::Titanium:
            Materials::update_titanium(world_, x, y);
            break;
        case MaterialID::Clay:
            Materials::update_clay(world_, x, y);
            break;
        case MaterialID::Charcoal:
            Materials::update_charcoal(world_, x, y);
            break;

        // ====== NEW ORGANIC (98-100) ======
        case MaterialID::Bamboo:
            Materials::update_bamboo(world_, x, y);
            break;
        case MaterialID::Honeycomb:
            Materials::update_honeycomb(world_, x, y);
            break;
        case MaterialID::Bone:
            Materials::update_bone(world_, x, y);
            break;

        // ====== NEW SPECIAL (101-102) ======
        case MaterialID::Napalm:
            Materials::update_napalm(world_, x, y);
            break;
        case MaterialID::Thermite:
            Materials::update_thermite(world_, x, y);
            break;

        // ====== EXPANSION: BASIC (103-112) ======
        case MaterialID::Bedrock:
            Materials::update_bedrock(world_, x, y);
            break;
        case MaterialID::Ceramic:
            Materials::update_ceramic(world_, x, y);
            break;
        case MaterialID::Granite:
            Materials::update_granite(world_, x, y);
            break;
        case MaterialID::Marble:
            Materials::update_marble(world_, x, y);
            break;
        case MaterialID::Sandstone:
            Materials::update_sandstone(world_, x, y);
            break;
        case MaterialID::Limestone:
            Materials::update_limestone(world_, x, y);
            break;
        case MaterialID::Slate:
            Materials::update_slate(world_, x, y);
            break;
        case MaterialID::Basalt:
            Materials::update_basalt(world_, x, y);
            break;
        case MaterialID::Quartz_Block:
            Materials::update_quartz_block(world_, x, y);
            break;
        case MaterialID::Soil:
            Materials::update_soil(world_, x, y);
            break;

        // ====== EXPANSION: POWDERS (113-117) ======
        case MaterialID::Flour:
            Materials::update_flour(world_, x, y);
            break;
        case MaterialID::Sulfur:
            Materials::update_sulfur(world_, x, y);
            break;
        case MaterialID::Cement:
            Materials::update_cement(world_, x, y);
            break;
        case MaterialID::Fertilizer:
            Materials::update_fertilizer(world_, x, y);
            break;
        case MaterialID::Volcanic_Ash:
            Materials::update_volcanic_ash(world_, x, y);
            break;

        // ====== EXPANSION: LIQUIDS (118-122) ======
        case MaterialID::Brine:
            Materials::update_brine(world_, x, y);
            break;
        case MaterialID::Coffee:
            Materials::update_coffee(world_, x, y);
            break;
        case MaterialID::Soap:
            Materials::update_soap(world_, x, y);
            break;
        case MaterialID::Paint:
            Materials::update_paint(world_, x, y);
            break;
        case MaterialID::Sewage:
            Materials::update_sewage(world_, x, y);
            break;

        // ====== EXPANSION: GASES (123-129) ======
        case MaterialID::Ammonia:
            Materials::update_ammonia(world_, x, y);
            break;
        case MaterialID::Carbon_Dioxide:
            Materials::update_carbon_dioxide(world_, x, y);
            break;
        case MaterialID::Nitrous:
            Materials::update_nitrous(world_, x, y);
            break;
        case MaterialID::Steam_Hot:
            Materials::update_steam_hot(world_, x, y);
            break;
        case MaterialID::Miasma:
            Materials::update_miasma(world_, x, y);
            break;
        case MaterialID::Pheromone:
            Materials::update_pheromone(world_, x, y);
            break;
        case MaterialID::Nerve_Gas:
            Materials::update_nerve_gas(world_, x, y);
            break;

        // ====== EXPANSION: SOLIDS (130-136) ======
        case MaterialID::Silver:
            Materials::update_silver(world_, x, y);
            break;
        case MaterialID::Platinum:
            Materials::update_platinum(world_, x, y);
            break;
        case MaterialID::Lead:
            Materials::update_lead(world_, x, y);
            break;
        case MaterialID::Tin:
            Materials::update_tin(world_, x, y);
            break;
        case MaterialID::Zinc:
            Materials::update_zinc(world_, x, y);
            break;
        case MaterialID::Bronze:
            Materials::update_bronze(world_, x, y);
            break;
        case MaterialID::Steel:
            Materials::update_steel(world_, x, y);
            break;

        // ====== EXPANSION: ORGANIC (137-143) ======
        case MaterialID::Pollen:
            Materials::update_pollen(world_, x, y);
            break;
        case MaterialID::Root:
            Materials::update_root(world_, x, y);
            break;
        case MaterialID::Bark:
            Materials::update_bark(world_, x, y);
            break;
        case MaterialID::Fruit:
            Materials::update_fruit(world_, x, y);
            break;
        case MaterialID::Egg:
            Materials::update_egg(world_, x, y);
            break;
        case MaterialID::Web:
            Materials::update_web(world_, x, y);
            break;
        case MaterialID::Mucus:
            Materials::update_mucus(world_, x, y);
            break;

        // ====== EXPANSION: SPECIAL (144-151) ======
        case MaterialID::Bomb:
            Materials::update_bomb(world_, x, y);
            break;
        case MaterialID::Nuke:
            Materials::update_nuke(world_, x, y);
            break;
        case MaterialID::Laser:
            Materials::update_laser(world_, x, y);
            break;
        case MaterialID::Black_Hole:
            Materials::update_black_hole(world_, x, y);
            break;
        case MaterialID::White_Hole:
            Materials::update_white_hole(world_, x, y);
            break;
        case MaterialID::Acid_Gas:
            Materials::update_acid_gas(world_, x, y);
            break;
        case MaterialID::Ice_Bomb:
            Materials::update_ice_bomb(world_, x, y);
            break;
        case MaterialID::Fire_Bomb:
            Materials::update_fire_bomb(world_, x, y);
            break;

        // ====== EXPANSION: FANTASY (152-161) ======
        case MaterialID::Mana:
            Materials::update_mana(world_, x, y);
            break;
        case MaterialID::Mirage:
            Materials::update_mirage(world_, x, y);
            break;
        case MaterialID::Holy_Water:
            Materials::update_holy_water(world_, x, y);
            break;
        case MaterialID::Cursed:
            Materials::update_cursed(world_, x, y);
            break;
        case MaterialID::Blessed:
            Materials::update_blessed(world_, x, y);
            break;
        case MaterialID::Soul:
            Materials::update_soul(world_, x, y);
            break;
        case MaterialID::Spirit:
            Materials::update_spirit(world_, x, y);
            break;
        case MaterialID::Aether:
            Materials::update_aether(world_, x, y);
            break;
        case MaterialID::Nether:
            Materials::update_nether(world_, x, y);
            break;
        case MaterialID::Phoenix_Ash:
            Materials::update_phoenix_ash(world_, x, y);
            break;

        default:
            break;
    }
}

} // namespace PixelEngine
