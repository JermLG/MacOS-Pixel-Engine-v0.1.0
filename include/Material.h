#pragma once

#include "Types.h"
#include <array>
#include <random>

namespace PixelEngine {

// Forward declaration
class World;

// Material definition (cold data - not accessed in hot loop)
struct MaterialDef {
    MaterialID id;
    MaterialState state;
    float density;          // Higher = sinks below lower density
    Color base_color;
    uint8_t color_variance; // Random color variation (0-255)

    MaterialDef()
        : id(MaterialID::Empty)
        , state(MaterialState::Empty)
        , density(0.0f)
        , base_color()
        , color_variance(0) {}

    MaterialDef(MaterialID id, MaterialState state, float density,
                Color color, uint8_t variance = 0)
        : id(id)
        , state(state)
        , density(density)
        , base_color(color)
        , color_variance(variance) {}

    // Get color with random variation
    Color get_color(std::mt19937& rng) const;
};

// Material system - manages all material definitions
class MaterialSystem {
public:
    MaterialSystem();

    // Get material definition
    const MaterialDef& get_material(MaterialID id) const {
        return materials_[static_cast<size_t>(id)];
    }

    // Get random color for material
    Color get_material_color(MaterialID id);

private:
    std::array<MaterialDef, static_cast<size_t>(MaterialID::COUNT)> materials_;
    std::mt19937 rng_;

    void initialize_materials();
};

// Material update functions (simulation rules)
// These are called during simulation for each active cell

namespace Materials {

// Empty does nothing
inline void update_empty(World& world, int32_t x, int32_t y) {
    // No-op
    (void)world;
    (void)x;
    (void)y;
}

// Stone is immovable solid
inline void update_stone(World& world, int32_t x, int32_t y) {
    // Solids don't move
    (void)world;
    (void)x;
    (void)y;
}

// Sand falls down, slides diagonally if blocked
void update_sand(World& world, int32_t x, int32_t y);

// Water falls down, spreads horizontally
void update_water(World& world, int32_t x, int32_t y);

// Steam rises and disperses
void update_steam(World& world, int32_t x, int32_t y);

// Oil flows like water but slower, floats on water
void update_oil(World& world, int32_t x, int32_t y);

// Fire rises, spreads to flammable materials, has lifetime
void update_fire(World& world, int32_t x, int32_t y);

// Wood is static, but can burn and float
void update_wood(World& world, int32_t x, int32_t y);

// Acid corrodes solids and flows like water
void update_acid(World& world, int32_t x, int32_t y);

// Lava burns and solidifies on contact with water
void update_lava(World& world, int32_t x, int32_t y);

// Ash rises slowly then settles
void update_ash(World& world, int32_t x, int32_t y);

// Grass is static but can burn
void update_grass(World& world, int32_t x, int32_t y);

// Smoke rises and dissipates
void update_smoke(World& world, int32_t x, int32_t y);

// Person: Autonomous agent that walks, falls, reproduces
void update_person(World& world, int32_t x, int32_t y);

// ============================================================================
// NEW MATERIAL UPDATE FUNCTIONS
// ============================================================================

// Powders (10-19)
void update_dirt(World& world, int32_t x, int32_t y);
void update_gravel(World& world, int32_t x, int32_t y);
void update_snow(World& world, int32_t x, int32_t y);
void update_gunpowder(World& world, int32_t x, int32_t y);
void update_salt(World& world, int32_t x, int32_t y);
void update_coal(World& world, int32_t x, int32_t y);
void update_sawdust(World& world, int32_t x, int32_t y);
void update_glass_powder(World& world, int32_t x, int32_t y);

// Liquids (20-29)
void update_honey(World& world, int32_t x, int32_t y);
void update_mud(World& world, int32_t x, int32_t y);
void update_blood(World& world, int32_t x, int32_t y);
void update_poison(World& world, int32_t x, int32_t y);
void update_slime(World& world, int32_t x, int32_t y);
void update_milk(World& world, int32_t x, int32_t y);
void update_alcohol(World& world, int32_t x, int32_t y);
void update_mercury(World& world, int32_t x, int32_t y);
void update_petrol(World& world, int32_t x, int32_t y);
void update_glue(World& world, int32_t x, int32_t y);

// Gases (30-39)
void update_toxic_gas(World& world, int32_t x, int32_t y);
void update_hydrogen(World& world, int32_t x, int32_t y);
void update_helium(World& world, int32_t x, int32_t y);
void update_methane(World& world, int32_t x, int32_t y);
void update_spark(World& world, int32_t x, int32_t y);
void update_plasma(World& world, int32_t x, int32_t y);
void update_dust(World& world, int32_t x, int32_t y);
void update_spore(World& world, int32_t x, int32_t y);
void update_confetti(World& world, int32_t x, int32_t y);

// Solids (40-49)
void update_metal(World& world, int32_t x, int32_t y);
void update_gold(World& world, int32_t x, int32_t y);
void update_ice(World& world, int32_t x, int32_t y);
void update_glass(World& world, int32_t x, int32_t y);
void update_brick(World& world, int32_t x, int32_t y);
void update_obsidian(World& world, int32_t x, int32_t y);
void update_diamond(World& world, int32_t x, int32_t y);
void update_copper(World& world, int32_t x, int32_t y);
void update_rubber(World& world, int32_t x, int32_t y);

// Organic (50-59)
void update_leaf(World& world, int32_t x, int32_t y);
void update_moss(World& world, int32_t x, int32_t y);
void update_vine(World& world, int32_t x, int32_t y);
void update_fungus(World& world, int32_t x, int32_t y);
void update_seed(World& world, int32_t x, int32_t y);
void update_flower(World& world, int32_t x, int32_t y);
void update_algae(World& world, int32_t x, int32_t y);
void update_coral(World& world, int32_t x, int32_t y);
void update_wax(World& world, int32_t x, int32_t y);
void update_flesh(World& world, int32_t x, int32_t y);

// Special (60-69)
void update_clone(World& world, int32_t x, int32_t y);
void update_void(World& world, int32_t x, int32_t y);
void update_fuse(World& world, int32_t x, int32_t y);
void update_tnt(World& world, int32_t x, int32_t y);
void update_c4(World& world, int32_t x, int32_t y);
void update_firework(World& world, int32_t x, int32_t y);
void update_lightning(World& world, int32_t x, int32_t y);
void update_portal_in(World& world, int32_t x, int32_t y);
void update_portal_out(World& world, int32_t x, int32_t y);

// Fantasy (70-79)
void update_magic(World& world, int32_t x, int32_t y);
void update_crystal(World& world, int32_t x, int32_t y);
void update_ectoplasm(World& world, int32_t x, int32_t y);
void update_antimatter(World& world, int32_t x, int32_t y);
void update_fairy_dust(World& world, int32_t x, int32_t y);
void update_dragon_fire(World& world, int32_t x, int32_t y);
void update_frost(World& world, int32_t x, int32_t y);
void update_ember(World& world, int32_t x, int32_t y);
void update_stardust(World& world, int32_t x, int32_t y);
void update_void_dust(World& world, int32_t x, int32_t y);

// Rust (powder) - missing from above
void update_rust(World& world, int32_t x, int32_t y);

// Life (spawner particle) - falls and creates Person on safe ground
void update_life(World& world, int32_t x, int32_t y);

// ============================================================================
// NEW MATERIALS (81-102)
// ============================================================================

// New Powders (81-85)
void update_thermite_powder(World& world, int32_t x, int32_t y);
void update_sugar(World& world, int32_t x, int32_t y);
void update_iron_filings(World& world, int32_t x, int32_t y);
void update_chalk(World& world, int32_t x, int32_t y);
void update_calcium(World& world, int32_t x, int32_t y);

// New Liquids (86-90)
void update_tar(World& world, int32_t x, int32_t y);
void update_juice(World& world, int32_t x, int32_t y);
void update_sap(World& world, int32_t x, int32_t y);
void update_bleach(World& world, int32_t x, int32_t y);
void update_ink(World& world, int32_t x, int32_t y);

// New Gases (91-93)
void update_chlorine(World& world, int32_t x, int32_t y);
void update_liquid_nitrogen(World& world, int32_t x, int32_t y);
void update_oxygen(World& world, int32_t x, int32_t y);

// New Solids (94-97)
void update_concrete(World& world, int32_t x, int32_t y);
void update_titanium(World& world, int32_t x, int32_t y);
void update_clay(World& world, int32_t x, int32_t y);
void update_charcoal(World& world, int32_t x, int32_t y);

// New Organic (98-100)
void update_bamboo(World& world, int32_t x, int32_t y);
void update_honeycomb(World& world, int32_t x, int32_t y);
void update_bone(World& world, int32_t x, int32_t y);

// New Special (101-102)
void update_napalm(World& world, int32_t x, int32_t y);
void update_thermite(World& world, int32_t x, int32_t y);

} // namespace Materials

} // namespace PixelEngine
