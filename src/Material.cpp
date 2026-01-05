#include "Material.h"
#include "World.h"
#include <random>
#include <cmath>

namespace PixelEngine {

// ============================================================================
// PERSON AI SYSTEM - Village Building & Lifelike Behavior
// ============================================================================

// AI Goals/States for people
enum class PersonGoal : uint8_t {
    Idle = 0,           // Just wandering around
    Exploring = 1,      // Looking for good building spots
    GatheringMaterial = 2, // Finding/moving toward building materials
    Building = 3,       // Actively constructing
    Socializing = 4,    // Following/interacting with others
    Fleeing = 5,        // Running from danger
    Resting = 6         // Taking a break (regenerating)
};

// Building types
enum class BuildingType : uint8_t {
    None = 0,
    SmallHut = 1,       // 5x4 simple shelter
    House = 2,          // 8x6 proper house with roof
    Tower = 3,          // 4x10 tall structure
    Wall = 4,           // Defensive wall segment
    Platform = 5        // Simple floor/bridge
};

// Village tracking - global state for coordinated building
struct VillageData {
    static constexpr int MAX_BUILDINGS = 20;
    int32_t building_centers_x[MAX_BUILDINGS];
    int32_t building_centers_y[MAX_BUILDINGS];
    BuildingType building_types[MAX_BUILDINGS];
    int building_count;
    int32_t village_center_x;
    int32_t village_center_y;
    bool initialized;

    VillageData() : building_count(0), village_center_x(-1), village_center_y(-1), initialized(false) {
        for (int i = 0; i < MAX_BUILDINGS; i++) {
            building_centers_x[i] = -1;
            building_centers_y[i] = -1;
            building_types[i] = BuildingType::None;
        }
    }
};

// Global village data (shared across all people)
static VillageData g_village;

// Helper: Simple deterministic random from seed
static uint32_t person_rand(uint32_t& seed) {
    seed = seed * 1664525u + 1013904223u;
    return seed;
}

// Get color with random variation
Color MaterialDef::get_color(std::mt19937& rng) const {
    if (color_variance == 0) {
        return base_color;
    }

    std::uniform_int_distribution<int> dist(-color_variance, color_variance);

    auto clamp = [](int val) -> uint8_t {
        if (val < 0) return 0;
        if (val > 255) return 255;
        return static_cast<uint8_t>(val);
    };

    return Color(
        clamp(base_color.r + dist(rng)),
        clamp(base_color.g + dist(rng)),
        clamp(base_color.b + dist(rng)),
        base_color.a
    );
}

MaterialSystem::MaterialSystem()
    : rng_(std::random_device{}()) {
    initialize_materials();
}

void MaterialSystem::initialize_materials() {
    // Empty (air)
    materials_[static_cast<size_t>(MaterialID::Empty)] = MaterialDef(
        MaterialID::Empty,
        MaterialState::Empty,
        0.0f,
        Color(0, 0, 0, 0),  // Transparent
        0
    );

    // Stone (immovable solid)
    materials_[static_cast<size_t>(MaterialID::Stone)] = MaterialDef(
        MaterialID::Stone,
        MaterialState::Solid,
        1000.0f,
        Color(100, 100, 100),  // Gray
        15  // Slight color variation
    );

    // Sand (powder - falls and piles)
    materials_[static_cast<size_t>(MaterialID::Sand)] = MaterialDef(
        MaterialID::Sand,
        MaterialState::Powder,
        1.5f,
        Color(194, 178, 128),  // Sandy yellow
        20  // Color variation
    );

    // Water (liquid - flows)
    materials_[static_cast<size_t>(MaterialID::Water)] = MaterialDef(
        MaterialID::Water,
        MaterialState::Liquid,
        1.0f,
        Color(64, 164, 223),  // Blue
        10  // Slight variation
    );

    // Steam (gas - rises)
    materials_[static_cast<size_t>(MaterialID::Steam)] = MaterialDef(
        MaterialID::Steam,
        MaterialState::Gas,
        0.1f,
        Color(220, 220, 220, 180),  // Light gray, semi-transparent
        15
    );

    // Oil (liquid - less dense than water, flammable)
    materials_[static_cast<size_t>(MaterialID::Oil)] = MaterialDef(
        MaterialID::Oil,
        MaterialState::Liquid,
        0.8f,  // Less dense than water (floats)
        Color(40, 35, 20),  // Dark brown/black
        8
    );

    // Fire (gas/energy - rises, spreads, has lifetime)
    materials_[static_cast<size_t>(MaterialID::Fire)] = MaterialDef(
        MaterialID::Fire,
        MaterialState::Gas,
        0.05f,  // Very light
        Color(255, 120, 0),  // Bright orange
        40  // High variation (orange to yellow)
    );

    // Wood (solid - burnable, floats on water)
    materials_[static_cast<size_t>(MaterialID::Wood)] = MaterialDef(
        MaterialID::Wood,
        MaterialState::Solid,
        0.6f,  // Less dense than water (floats)
        Color(101, 67, 33),  // Brown
        15
    );

    // Acid (liquid - corrodes solids)
    materials_[static_cast<size_t>(MaterialID::Acid)] = MaterialDef(
        MaterialID::Acid,
        MaterialState::Liquid,
        1.2f,  // Denser than water
        Color(100, 255, 100),  // Bright green
        20
    );

    // Lava (liquid/fire hybrid - burns, solidifies)
    materials_[static_cast<size_t>(MaterialID::Lava)] = MaterialDef(
        MaterialID::Lava,
        MaterialState::Liquid,
        2.5f,  // Very dense
        Color(255, 80, 0),  // Bright red-orange
        30  // High variation (red to yellow)
    );

    // Ash (powder - rises slowly, settles)
    materials_[static_cast<size_t>(MaterialID::Ash)] = MaterialDef(
        MaterialID::Ash,
        MaterialState::Powder,
        0.3f,  // Light powder
        Color(60, 60, 60),  // Dark gray
        10
    );

    // Grass (solid - burnable)
    materials_[static_cast<size_t>(MaterialID::Grass)] = MaterialDef(
        MaterialID::Grass,
        MaterialState::Solid,
        0.8f,
        Color(34, 139, 34),  // Green
        25
    );

    // Smoke (gas - rises, dissipates)
    materials_[static_cast<size_t>(MaterialID::Smoke)] = MaterialDef(
        MaterialID::Smoke,
        MaterialState::Gas,
        0.08f,  // Very light
        Color(80, 80, 80, 150),  // Gray, semi-transparent
        20
    );

    // Person (autonomous agent - solid-like for update ordering)
    materials_[static_cast<size_t>(MaterialID::Person)] = MaterialDef(
        MaterialID::Person,
        MaterialState::Solid,  // Treated as solid for priority order (updates last)
        1.0f,  // Human density
        Color(255, 50, 255),  // Bright magenta (highly visible)
        20  // Some color variation
    );

    // ============================================================================
    // POWDERS (10-19)
    // ============================================================================

    // Dirt (earthy brown powder)
    materials_[static_cast<size_t>(MaterialID::Dirt)] = MaterialDef(
        MaterialID::Dirt,
        MaterialState::Powder,
        1.4f,
        Color(101, 67, 33),  // Brown
        20
    );

    // Gravel (gray rocky powder)
    materials_[static_cast<size_t>(MaterialID::Gravel)] = MaterialDef(
        MaterialID::Gravel,
        MaterialState::Powder,
        2.0f,
        Color(128, 128, 128),  // Gray
        25
    );

    // Snow (white powder, light)
    materials_[static_cast<size_t>(MaterialID::Snow)] = MaterialDef(
        MaterialID::Snow,
        MaterialState::Powder,
        0.3f,
        Color(240, 248, 255),  // Almost white
        10
    );

    // Gunpowder (dark gray, explosive)
    materials_[static_cast<size_t>(MaterialID::Gunpowder)] = MaterialDef(
        MaterialID::Gunpowder,
        MaterialState::Powder,
        1.2f,
        Color(50, 50, 50),  // Dark gray
        10
    );

    // Salt (white crystals)
    materials_[static_cast<size_t>(MaterialID::Salt)] = MaterialDef(
        MaterialID::Salt,
        MaterialState::Powder,
        1.3f,
        Color(255, 255, 255),  // White
        8
    );

    // Coal (black powder, burns)
    materials_[static_cast<size_t>(MaterialID::Coal)] = MaterialDef(
        MaterialID::Coal,
        MaterialState::Powder,
        1.5f,
        Color(30, 30, 30),  // Almost black
        10
    );

    // Rust (orange-brown powder)
    materials_[static_cast<size_t>(MaterialID::Rust)] = MaterialDef(
        MaterialID::Rust,
        MaterialState::Powder,
        1.8f,
        Color(183, 65, 14),  // Rust orange
        20
    );

    // Sawdust (light tan, flammable)
    materials_[static_cast<size_t>(MaterialID::Sawdust)] = MaterialDef(
        MaterialID::Sawdust,
        MaterialState::Powder,
        0.4f,
        Color(210, 180, 140),  // Tan
        15
    );

    // Glass_Powder (sparkly white/clear)
    materials_[static_cast<size_t>(MaterialID::Glass_Powder)] = MaterialDef(
        MaterialID::Glass_Powder,
        MaterialState::Powder,
        1.6f,
        Color(200, 220, 255),  // Light blue tint
        20
    );

    // ============================================================================
    // LIQUIDS (20-29)
    // ============================================================================

    // Honey (golden viscous)
    materials_[static_cast<size_t>(MaterialID::Honey)] = MaterialDef(
        MaterialID::Honey,
        MaterialState::Liquid,
        1.4f,
        Color(255, 185, 15),  // Golden
        15
    );

    // Mud (brown liquid)
    materials_[static_cast<size_t>(MaterialID::Mud)] = MaterialDef(
        MaterialID::Mud,
        MaterialState::Liquid,
        1.5f,
        Color(80, 60, 40),  // Dark brown
        15
    );

    // Blood (dark red)
    materials_[static_cast<size_t>(MaterialID::Blood)] = MaterialDef(
        MaterialID::Blood,
        MaterialState::Liquid,
        1.05f,
        Color(138, 7, 7),  // Dark red
        20
    );

    // Poison (purple toxic)
    materials_[static_cast<size_t>(MaterialID::Poison)] = MaterialDef(
        MaterialID::Poison,
        MaterialState::Liquid,
        1.1f,
        Color(148, 0, 211),  // Purple
        25
    );

    // Slime (green goop)
    materials_[static_cast<size_t>(MaterialID::Slime)] = MaterialDef(
        MaterialID::Slime,
        MaterialState::Liquid,
        1.3f,
        Color(50, 205, 50),  // Lime green
        20
    );

    // Milk (white liquid)
    materials_[static_cast<size_t>(MaterialID::Milk)] = MaterialDef(
        MaterialID::Milk,
        MaterialState::Liquid,
        1.03f,
        Color(255, 250, 250),  // White
        5
    );

    // Alcohol (clear/light blue)
    materials_[static_cast<size_t>(MaterialID::Alcohol)] = MaterialDef(
        MaterialID::Alcohol,
        MaterialState::Liquid,
        0.79f,  // Lighter than water
        Color(200, 220, 255),  // Light blue
        15
    );

    // Mercury (silver metallic liquid)
    materials_[static_cast<size_t>(MaterialID::Mercury)] = MaterialDef(
        MaterialID::Mercury,
        MaterialState::Liquid,
        13.5f,  // Very heavy!
        Color(192, 192, 192),  // Silver
        15
    );

    // Petrol (light yellow, flammable)
    materials_[static_cast<size_t>(MaterialID::Petrol)] = MaterialDef(
        MaterialID::Petrol,
        MaterialState::Liquid,
        0.75f,  // Lighter than water
        Color(255, 255, 100),  // Light yellow
        20
    );

    // Glue (white sticky)
    materials_[static_cast<size_t>(MaterialID::Glue)] = MaterialDef(
        MaterialID::Glue,
        MaterialState::Liquid,
        1.2f,
        Color(255, 255, 240),  // Off-white
        10
    );

    // ============================================================================
    // GASES (30-39)
    // ============================================================================

    // Toxic_Gas (green gas)
    materials_[static_cast<size_t>(MaterialID::Toxic_Gas)] = MaterialDef(
        MaterialID::Toxic_Gas,
        MaterialState::Gas,
        0.07f,
        Color(50, 150, 50, 150),  // Green, semi-transparent
        20
    );

    // Hydrogen (very light, invisible-ish)
    materials_[static_cast<size_t>(MaterialID::Hydrogen)] = MaterialDef(
        MaterialID::Hydrogen,
        MaterialState::Gas,
        0.02f,  // Very light
        Color(200, 200, 255, 100),  // Very faint blue
        15
    );

    // Helium (light, slightly visible)
    materials_[static_cast<size_t>(MaterialID::Helium)] = MaterialDef(
        MaterialID::Helium,
        MaterialState::Gas,
        0.03f,
        Color(255, 200, 200, 120),  // Very faint pink
        15
    );

    // Methane (invisible-ish, flammable)
    materials_[static_cast<size_t>(MaterialID::Methane)] = MaterialDef(
        MaterialID::Methane,
        MaterialState::Gas,
        0.04f,
        Color(180, 180, 180, 80),  // Very faint
        10
    );

    // Spark (bright electrical)
    materials_[static_cast<size_t>(MaterialID::Spark)] = MaterialDef(
        MaterialID::Spark,
        MaterialState::Gas,
        0.01f,
        Color(255, 255, 0),  // Bright yellow
        40
    );

    // Plasma (hot pink/purple)
    materials_[static_cast<size_t>(MaterialID::Plasma)] = MaterialDef(
        MaterialID::Plasma,
        MaterialState::Gas,
        0.01f,
        Color(255, 0, 255),  // Hot magenta
        50
    );

    // Dust (brown floating)
    materials_[static_cast<size_t>(MaterialID::Dust)] = MaterialDef(
        MaterialID::Dust,
        MaterialState::Gas,
        0.15f,
        Color(139, 119, 101, 180),  // Dusty brown
        20
    );

    // Spore (green biological)
    materials_[static_cast<size_t>(MaterialID::Spore)] = MaterialDef(
        MaterialID::Spore,
        MaterialState::Gas,
        0.12f,
        Color(100, 180, 100, 160),  // Green
        25
    );

    // Confetti (colorful!)
    materials_[static_cast<size_t>(MaterialID::Confetti)] = MaterialDef(
        MaterialID::Confetti,
        MaterialState::Gas,
        0.2f,
        Color(255, 100, 150),  // Pink (but will vary)
        100  // High variance for colorful effect
    );

    // ============================================================================
    // SOLIDS (40-49)
    // ============================================================================

    // Metal (dark gray, heavy)
    materials_[static_cast<size_t>(MaterialID::Metal)] = MaterialDef(
        MaterialID::Metal,
        MaterialState::Solid,
        7.8f,
        Color(120, 120, 130),  // Steel gray
        15
    );

    // Gold (shiny yellow)
    materials_[static_cast<size_t>(MaterialID::Gold)] = MaterialDef(
        MaterialID::Gold,
        MaterialState::Solid,
        19.3f,  // Very heavy
        Color(255, 215, 0),  // Gold
        20
    );

    // Ice (light blue, melts)
    materials_[static_cast<size_t>(MaterialID::Ice)] = MaterialDef(
        MaterialID::Ice,
        MaterialState::Solid,
        0.92f,  // Less dense than water
        Color(173, 216, 230),  // Light blue
        15
    );

    // Glass (transparent blue-white)
    materials_[static_cast<size_t>(MaterialID::Glass)] = MaterialDef(
        MaterialID::Glass,
        MaterialState::Solid,
        2.5f,
        Color(200, 230, 255),  // Very light blue
        10
    );

    // Brick (reddish brown)
    materials_[static_cast<size_t>(MaterialID::Brick)] = MaterialDef(
        MaterialID::Brick,
        MaterialState::Solid,
        1.9f,
        Color(178, 34, 34),  // Brick red
        20
    );

    // Obsidian (black volcanic glass)
    materials_[static_cast<size_t>(MaterialID::Obsidian)] = MaterialDef(
        MaterialID::Obsidian,
        MaterialState::Solid,
        2.4f,
        Color(20, 20, 30),  // Very dark
        10
    );

    // Diamond (sparkly clear)
    materials_[static_cast<size_t>(MaterialID::Diamond)] = MaterialDef(
        MaterialID::Diamond,
        MaterialState::Solid,
        3.5f,
        Color(185, 242, 255),  // Light cyan sparkle
        25
    );

    // Copper (orange-brown metal)
    materials_[static_cast<size_t>(MaterialID::Copper)] = MaterialDef(
        MaterialID::Copper,
        MaterialState::Solid,
        8.9f,
        Color(184, 115, 51),  // Copper brown
        20
    );

    // Rubber (dark bouncy)
    materials_[static_cast<size_t>(MaterialID::Rubber)] = MaterialDef(
        MaterialID::Rubber,
        MaterialState::Solid,
        1.1f,
        Color(30, 30, 30),  // Dark
        15
    );

    // ============================================================================
    // ORGANIC (50-59)
    // ============================================================================

    // Leaf (bright green)
    materials_[static_cast<size_t>(MaterialID::Leaf)] = MaterialDef(
        MaterialID::Leaf,
        MaterialState::Powder,  // Falls like a powder
        0.2f,
        Color(50, 180, 50),  // Green
        30
    );

    // Moss (dark green, grows)
    materials_[static_cast<size_t>(MaterialID::Moss)] = MaterialDef(
        MaterialID::Moss,
        MaterialState::Solid,
        0.5f,
        Color(34, 100, 34),  // Dark green
        25
    );

    // Vine (green, grows down)
    materials_[static_cast<size_t>(MaterialID::Vine)] = MaterialDef(
        MaterialID::Vine,
        MaterialState::Solid,
        0.4f,
        Color(0, 128, 0),  // Green
        20
    );

    // Fungus (purple/tan)
    materials_[static_cast<size_t>(MaterialID::Fungus)] = MaterialDef(
        MaterialID::Fungus,
        MaterialState::Solid,
        0.6f,
        Color(150, 100, 150),  // Purple-tan
        30
    );

    // Seed (brown, grows into plants)
    materials_[static_cast<size_t>(MaterialID::Seed)] = MaterialDef(
        MaterialID::Seed,
        MaterialState::Powder,
        0.8f,
        Color(139, 90, 43),  // Brown
        20
    );

    // Flower (colorful)
    materials_[static_cast<size_t>(MaterialID::Flower)] = MaterialDef(
        MaterialID::Flower,
        MaterialState::Solid,
        0.3f,
        Color(255, 100, 150),  // Pink
        50  // High variance for variety
    );

    // Algae (green water plant)
    materials_[static_cast<size_t>(MaterialID::Algae)] = MaterialDef(
        MaterialID::Algae,
        MaterialState::Liquid,  // Floats in water
        0.95f,
        Color(0, 100, 0),  // Dark green
        25
    );

    // Coral (orange/pink, underwater)
    materials_[static_cast<size_t>(MaterialID::Coral)] = MaterialDef(
        MaterialID::Coral,
        MaterialState::Solid,
        1.5f,
        Color(255, 127, 80),  // Coral color
        30
    );

    // Wax (off-white, melts)
    materials_[static_cast<size_t>(MaterialID::Wax)] = MaterialDef(
        MaterialID::Wax,
        MaterialState::Solid,
        0.9f,
        Color(255, 250, 200),  // Off-white/cream
        15
    );

    // Flesh (pink organic)
    materials_[static_cast<size_t>(MaterialID::Flesh)] = MaterialDef(
        MaterialID::Flesh,
        MaterialState::Solid,
        1.05f,
        Color(255, 182, 193),  // Pink
        20
    );

    // ============================================================================
    // SPECIAL (60-69)
    // ============================================================================

    // Clone (copies adjacent materials)
    materials_[static_cast<size_t>(MaterialID::Clone)] = MaterialDef(
        MaterialID::Clone,
        MaterialState::Solid,
        1.0f,
        Color(200, 200, 200),  // Gray
        10
    );

    // Void (destroys everything)
    materials_[static_cast<size_t>(MaterialID::Void)] = MaterialDef(
        MaterialID::Void,
        MaterialState::Solid,
        1000.0f,
        Color(0, 0, 0),  // Pure black
        0
    );

    // Fuse (burns along a line)
    materials_[static_cast<size_t>(MaterialID::Fuse)] = MaterialDef(
        MaterialID::Fuse,
        MaterialState::Solid,
        0.5f,
        Color(160, 82, 45),  // Brown rope
        15
    );

    // TNT (explosive!)
    materials_[static_cast<size_t>(MaterialID::TNT)] = MaterialDef(
        MaterialID::TNT,
        MaterialState::Solid,
        1.0f,
        Color(255, 0, 0),  // Red
        15
    );

    // C4 (more explosive!)
    materials_[static_cast<size_t>(MaterialID::C4)] = MaterialDef(
        MaterialID::C4,
        MaterialState::Solid,
        1.3f,
        Color(240, 230, 140),  // Khaki/tan
        10
    );

    // Firework (shoots up and explodes)
    materials_[static_cast<size_t>(MaterialID::Firework)] = MaterialDef(
        MaterialID::Firework,
        MaterialState::Solid,
        0.8f,
        Color(255, 50, 50),  // Red
        30
    );

    // Lightning (electrical discharge)
    materials_[static_cast<size_t>(MaterialID::Lightning)] = MaterialDef(
        MaterialID::Lightning,
        MaterialState::Gas,
        0.01f,
        Color(255, 255, 150),  // Bright yellow
        50
    );

    // Portal_In (teleporter entrance)
    materials_[static_cast<size_t>(MaterialID::Portal_In)] = MaterialDef(
        MaterialID::Portal_In,
        MaterialState::Solid,
        1.0f,
        Color(0, 100, 255),  // Blue
        30
    );

    // Portal_Out (teleporter exit)
    materials_[static_cast<size_t>(MaterialID::Portal_Out)] = MaterialDef(
        MaterialID::Portal_Out,
        MaterialState::Solid,
        1.0f,
        Color(255, 100, 0),  // Orange
        30
    );

    // ============================================================================
    // FANTASY (70-79)
    // ============================================================================

    // Magic (sparkly purple energy)
    materials_[static_cast<size_t>(MaterialID::Magic)] = MaterialDef(
        MaterialID::Magic,
        MaterialState::Gas,
        0.05f,
        Color(180, 100, 255),  // Purple
        50
    );

    // Crystal (sparkly gem)
    materials_[static_cast<size_t>(MaterialID::Crystal)] = MaterialDef(
        MaterialID::Crystal,
        MaterialState::Solid,
        2.8f,
        Color(200, 100, 255),  // Purple crystal
        40
    );

    // Ectoplasm (ghostly green goo)
    materials_[static_cast<size_t>(MaterialID::Ectoplasm)] = MaterialDef(
        MaterialID::Ectoplasm,
        MaterialState::Liquid,
        0.5f,
        Color(100, 255, 150, 180),  // Ghostly green
        30
    );

    // Antimatter (inverted colors, destroys on contact)
    materials_[static_cast<size_t>(MaterialID::Antimatter)] = MaterialDef(
        MaterialID::Antimatter,
        MaterialState::Liquid,
        -1.0f,  // Negative density (rises!)
        Color(50, 0, 80),  // Dark purple
        20
    );

    // Fairy_Dust (sparkly pink)
    materials_[static_cast<size_t>(MaterialID::Fairy_Dust)] = MaterialDef(
        MaterialID::Fairy_Dust,
        MaterialState::Powder,
        0.1f,
        Color(255, 182, 255),  // Pink
        50
    );

    // Dragon_Fire (super hot flames)
    materials_[static_cast<size_t>(MaterialID::Dragon_Fire)] = MaterialDef(
        MaterialID::Dragon_Fire,
        MaterialState::Gas,
        0.02f,
        Color(255, 50, 0),  // Deep orange-red
        40
    );

    // Frost (freezing cold particles)
    materials_[static_cast<size_t>(MaterialID::Frost)] = MaterialDef(
        MaterialID::Frost,
        MaterialState::Gas,
        0.08f,
        Color(200, 230, 255),  // Icy blue
        25
    );

    // Ember (glowing hot particles)
    materials_[static_cast<size_t>(MaterialID::Ember)] = MaterialDef(
        MaterialID::Ember,
        MaterialState::Powder,
        0.4f,
        Color(255, 100, 0),  // Orange glow
        35
    );

    // Stardust (sparkly cosmic)
    materials_[static_cast<size_t>(MaterialID::Stardust)] = MaterialDef(
        MaterialID::Stardust,
        MaterialState::Powder,
        0.05f,
        Color(255, 255, 200),  // Pale gold
        60
    );

    // Void_Dust (dark matter particles)
    materials_[static_cast<size_t>(MaterialID::Void_Dust)] = MaterialDef(
        MaterialID::Void_Dust,
        MaterialState::Powder,
        0.15f,
        Color(30, 0, 50),  // Very dark purple
        20
    );

    // Life (falling spawner particle - creates Person on safe ground)
    materials_[static_cast<size_t>(MaterialID::Life)] = MaterialDef(
        MaterialID::Life,
        MaterialState::Powder,  // Falls like a particle
        0.8f,
        Color(255, 200, 255),  // Bright pink/magenta glow
        30  // High variation for sparkle effect
    );
}

Color MaterialSystem::get_material_color(MaterialID id) {
    const auto& mat = materials_[static_cast<size_t>(id)];
    return mat.get_color(rng_);
}

// ============================================================================
// Material Update Functions (Cellular Automata Rules)
// ============================================================================

namespace Materials {

// ============================================================================
// MATERIAL COMBINATION SYSTEM
// ============================================================================
// When certain materials touch, they can combine to create new materials.
// This creates interesting emergent gameplay and chemistry-like interactions.

struct MaterialCombination {
    MaterialID mat_a;
    MaterialID mat_b;
    MaterialID result_a;  // What mat_a becomes (Empty to consume)
    MaterialID result_b;  // What mat_b becomes (Empty to consume)
    int chance;           // 1 in N chance per frame (higher = rarer)
};

// Combination recipes - order doesn't matter for matching
static const MaterialCombination COMBINATIONS[] = {
    // === POWDER + LIQUID COMBINATIONS ===
    // Sand + Water = Mud (wet sand)
    {MaterialID::Sand, MaterialID::Water, MaterialID::Mud, MaterialID::Empty, 8},
    // Dirt + Water = Mud
    {MaterialID::Dirt, MaterialID::Water, MaterialID::Mud, MaterialID::Empty, 4},
    // Salt + Water = Water (dissolves, salt disappears)
    {MaterialID::Salt, MaterialID::Water, MaterialID::Empty, MaterialID::Water, 16},
    // Sand + Lava = Glass
    {MaterialID::Sand, MaterialID::Lava, MaterialID::Glass, MaterialID::Stone, 4},
    // Glass_Powder + Lava = Glass
    {MaterialID::Glass_Powder, MaterialID::Lava, MaterialID::Glass, MaterialID::Stone, 2},

    // === POWDER + POWDER COMBINATIONS ===
    // Coal + Gunpowder = More Gunpowder (amplifies)
    {MaterialID::Coal, MaterialID::Gunpowder, MaterialID::Gunpowder, MaterialID::Gunpowder, 32},
    // Sawdust + Coal = Coal (compresses)
    {MaterialID::Sawdust, MaterialID::Coal, MaterialID::Coal, MaterialID::Empty, 64},
    // Ash + Water = Mud (muddy ash)
    {MaterialID::Ash, MaterialID::Water, MaterialID::Mud, MaterialID::Empty, 8},
    // Snow + Snow = Ice (compacting) - rare
    {MaterialID::Snow, MaterialID::Snow, MaterialID::Ice, MaterialID::Empty, 128},

    // === LIQUID + LIQUID COMBINATIONS ===
    // Water + Lava = Steam + Obsidian
    {MaterialID::Water, MaterialID::Lava, MaterialID::Steam, MaterialID::Obsidian, 2},
    // Oil + Water = stays separate (oil floats - handled by density)
    // Acid + Water = diluted (acid weakens)
    {MaterialID::Acid, MaterialID::Water, MaterialID::Poison, MaterialID::Empty, 16},
    // Blood + Water = diluted blood (becomes water)
    {MaterialID::Blood, MaterialID::Water, MaterialID::Water, MaterialID::Empty, 32},
    // Honey + Water = diluted (becomes slime)
    {MaterialID::Honey, MaterialID::Water, MaterialID::Slime, MaterialID::Empty, 16},
    // Milk + Acid = curdled (becomes slime)
    {MaterialID::Milk, MaterialID::Acid, MaterialID::Slime, MaterialID::Empty, 8},
    // Alcohol + Fire = more fire (spreads)
    {MaterialID::Alcohol, MaterialID::Fire, MaterialID::Fire, MaterialID::Fire, 2},
    // Petrol + Fire = explosion/fire
    {MaterialID::Petrol, MaterialID::Fire, MaterialID::Fire, MaterialID::Fire, 1},
    // Mercury + Acid = Toxic Gas
    {MaterialID::Mercury, MaterialID::Acid, MaterialID::Toxic_Gas, MaterialID::Empty, 8},

    // === ORGANIC COMBINATIONS ===
    // Seed + Water = Flower (growth)
    {MaterialID::Seed, MaterialID::Water, MaterialID::Flower, MaterialID::Empty, 64},
    // Seed + Dirt = Grass
    {MaterialID::Seed, MaterialID::Dirt, MaterialID::Grass, MaterialID::Empty, 32},
    // Leaf + Water = Algae
    {MaterialID::Leaf, MaterialID::Water, MaterialID::Algae, MaterialID::Empty, 64},
    // Fungus + Flesh = more Fungus (infection)
    {MaterialID::Fungus, MaterialID::Flesh, MaterialID::Fungus, MaterialID::Fungus, 16},
    // Moss + Water = Algae
    {MaterialID::Moss, MaterialID::Water, MaterialID::Algae, MaterialID::Empty, 64},

    // === METAL COMBINATIONS ===
    // Copper + Acid = Rust + Toxic Gas
    {MaterialID::Copper, MaterialID::Acid, MaterialID::Rust, MaterialID::Toxic_Gas, 32},
    // Metal + Acid = Rust
    {MaterialID::Metal, MaterialID::Acid, MaterialID::Rust, MaterialID::Empty, 64},
    // Metal + Water = Rust (slow oxidation)
    {MaterialID::Metal, MaterialID::Water, MaterialID::Rust, MaterialID::Water, 256},

    // === FANTASY/MAGIC COMBINATIONS ===
    // Stardust + Water = Magic
    {MaterialID::Stardust, MaterialID::Water, MaterialID::Magic, MaterialID::Empty, 8},
    // Fairy_Dust + Fire = Magic
    {MaterialID::Fairy_Dust, MaterialID::Fire, MaterialID::Magic, MaterialID::Empty, 4},
    // Void_Dust + any light = consumes it
    {MaterialID::Void_Dust, MaterialID::Fire, MaterialID::Void_Dust, MaterialID::Empty, 2},
    {MaterialID::Void_Dust, MaterialID::Spark, MaterialID::Void_Dust, MaterialID::Empty, 2},
    // Crystal + Magic = Diamond
    {MaterialID::Crystal, MaterialID::Magic, MaterialID::Diamond, MaterialID::Empty, 32},
    // Ember + Water = Steam + Ash
    {MaterialID::Ember, MaterialID::Water, MaterialID::Steam, MaterialID::Ash, 4},
    // Frost + Water = Ice
    {MaterialID::Frost, MaterialID::Water, MaterialID::Ice, MaterialID::Empty, 4},
    // Frost + Fire = Steam (cancels out)
    {MaterialID::Frost, MaterialID::Fire, MaterialID::Steam, MaterialID::Empty, 2},
    // Dragon_Fire + Water = Steam (lots of it)
    {MaterialID::Dragon_Fire, MaterialID::Water, MaterialID::Steam, MaterialID::Steam, 1},
    // Ectoplasm + Flesh = more Ectoplasm (ghostly infection)
    {MaterialID::Ectoplasm, MaterialID::Flesh, MaterialID::Ectoplasm, MaterialID::Ectoplasm, 16},
    // Magic + Stone = Crystal
    {MaterialID::Magic, MaterialID::Stone, MaterialID::Crystal, MaterialID::Empty, 32},
    // Magic + Sand = Gold (transmutation!)
    {MaterialID::Magic, MaterialID::Sand, MaterialID::Gold, MaterialID::Empty, 64},
    // Magic + Coal = Diamond
    {MaterialID::Magic, MaterialID::Coal, MaterialID::Diamond, MaterialID::Empty, 48},

    // === EXPLOSIVE COMBINATIONS ===
    // Gunpowder + Spark = Fire (ignition)
    {MaterialID::Gunpowder, MaterialID::Spark, MaterialID::Fire, MaterialID::Fire, 1},
    // Hydrogen + Spark = Fire (explosion)
    {MaterialID::Hydrogen, MaterialID::Spark, MaterialID::Fire, MaterialID::Fire, 1},
    // Methane + Spark = Fire
    {MaterialID::Methane, MaterialID::Spark, MaterialID::Fire, MaterialID::Fire, 1},

    // === SPECIAL COMBINATIONS ===
    // Clone + any material handled separately in update_clone
    // Antimatter combinations handled in update_antimatter
    // Plasma destroys most things - handled in update_plasma
};

static const int NUM_COMBINATIONS = sizeof(COMBINATIONS) / sizeof(COMBINATIONS[0]);

// Check if a material at position (x, y) can combine with any neighbors
// Returns true if a combination occurred
static bool try_material_combination(World& world, int32_t x, int32_t y) {
    MaterialID my_mat = world.get_material(x, y);
    if (my_mat == MaterialID::Empty) return false;

    // Check all 8 neighbors
    for (int dy = -1; dy <= 1; dy++) {
        for (int dx = -1; dx <= 1; dx++) {
            if (dx == 0 && dy == 0) continue;

            int nx = x + dx, ny = y + dy;
            if (!world.in_bounds(nx, ny)) continue;

            MaterialID neighbor_mat = world.get_material(nx, ny);
            if (neighbor_mat == MaterialID::Empty) continue;

            // Check all combination recipes
            for (int i = 0; i < NUM_COMBINATIONS; i++) {
                const MaterialCombination& combo = COMBINATIONS[i];

                bool match_forward = (my_mat == combo.mat_a && neighbor_mat == combo.mat_b);
                bool match_reverse = (my_mat == combo.mat_b && neighbor_mat == combo.mat_a);

                if (match_forward || match_reverse) {
                    // Random chance check
                    if ((world.random_int() % combo.chance) == 0) {
                        if (match_forward) {
                            // Apply combination
                            world.set_material(x, y, combo.result_a);
                            world.set_material(nx, ny, combo.result_b);

                            // Initialize new materials if needed
                            if (combo.result_a == MaterialID::Fire || combo.result_a == MaterialID::Steam ||
                                combo.result_a == MaterialID::Smoke || combo.result_a == MaterialID::Toxic_Gas) {
                                world.get_cell(x, y).set_lifetime(30);
                            }
                            if (combo.result_b == MaterialID::Fire || combo.result_b == MaterialID::Steam ||
                                combo.result_b == MaterialID::Smoke || combo.result_b == MaterialID::Toxic_Gas) {
                                world.get_cell(nx, ny).set_lifetime(30);
                            }
                            if (combo.result_a == MaterialID::Magic) {
                                world.get_cell(x, y).set_lifetime(40);
                            }
                            if (combo.result_b == MaterialID::Magic) {
                                world.get_cell(nx, ny).set_lifetime(40);
                            }
                        } else {
                            // Reverse match - swap results
                            world.set_material(x, y, combo.result_b);
                            world.set_material(nx, ny, combo.result_a);

                            if (combo.result_b == MaterialID::Fire || combo.result_b == MaterialID::Steam ||
                                combo.result_b == MaterialID::Smoke || combo.result_b == MaterialID::Toxic_Gas) {
                                world.get_cell(x, y).set_lifetime(30);
                            }
                            if (combo.result_a == MaterialID::Fire || combo.result_a == MaterialID::Steam ||
                                combo.result_a == MaterialID::Smoke || combo.result_a == MaterialID::Toxic_Gas) {
                                world.get_cell(nx, ny).set_lifetime(30);
                            }
                            if (combo.result_b == MaterialID::Magic) {
                                world.get_cell(x, y).set_lifetime(40);
                            }
                            if (combo.result_a == MaterialID::Magic) {
                                world.get_cell(nx, ny).set_lifetime(40);
                            }
                        }
                        return true;
                    }
                }
            }
        }
    }
    return false;
}

// ============================================================================
// END MATERIAL COMBINATION SYSTEM
// ============================================================================

// Sand: Falls down, slides diagonally if blocked
void update_sand(World& world, int32_t x, int32_t y) {
    // Check for material combinations first
    if (try_material_combination(world, x, y)) return;

    Cell& cell = world.get_cell(x, y);

    // Apply gravity (accelerate downward)
    cell.add_velocity(2);  // Faster gravity
    cell.clamp_velocity(0, 15);  // Max terminal velocity

    // PATH TRACING: Try to move along the entire velocity path
    int target_y = y + cell.velocity_y;

    // Trace from current position to target, stopping at first obstacle
    int best_y = y;
    for (int test_y = y + 1; test_y <= target_y && test_y < y + 600; test_y++) {
        if (world.in_bounds(x, test_y) && world.can_move_to(x, y, x, test_y)) {
            best_y = test_y;
        } else {
            break;  // Hit obstacle, stop here
        }
    }

    // Move to the furthest free position we found
    if (best_y > y) {
        int8_t vel = cell.velocity_y;
        if (world.try_move_cell(x, y, x, best_y)) {
            world.get_cell(x, best_y).velocity_y = vel;
            return;
        }
    }

    // Hit something - reset velocity
    world.get_cell(x, y).reset_velocity();

    // Try to slide diagonally (random direction first)
    bool try_left_first = (world.random_int() & 1) == 0;

    if (try_left_first) {
        if (world.try_move_cell(x, y, x - 1, y + 1)) return;
        if (world.try_move_cell(x, y, x + 1, y + 1)) return;
    } else {
        if (world.try_move_cell(x, y, x + 1, y + 1)) return;
        if (world.try_move_cell(x, y, x - 1, y + 1)) return;
    }

    // Can't move - settled
}

// Water: Fast flowing liquid with path tracing and lateral flow
void update_water(World& world, int32_t x, int32_t y) {
    // Check for material combinations first
    if (try_material_combination(world, x, y)) return;

    Cell& cell = world.get_cell(x, y);

    // Apply gravity
    cell.add_velocity(2);
    cell.clamp_velocity(0, 20);

    // Get current momentum (flow direction)
    bool flow_right = cell.get_flow_direction();
    uint32_t rand = world.random_int();

    // CRITICAL FIX: Check if we can move sideways while falling
    // This prevents tower formation by allowing early lateral dispersion
    bool can_fall = world.in_bounds(x, y + 1) &&
                    world.can_move_to(x, y, x, y + 1);

    // If we can fall AND there's space beside us, randomly spread sideways first
    // This breaks vertical columns before they form
    if (can_fall && (rand & 3) == 0) {  // 25% chance to spread while falling
        int dx = flow_right ? 1 : -1;

        // Try diagonal-down in momentum direction
        if (world.try_move_cell(x, y, x + dx, y + 1)) {
            world.get_cell(x + dx, y + 1).set_flow_direction(flow_right);
            world.get_cell(x + dx, y + 1).velocity_y = cell.velocity_y;
            return;
        }

        // Try opposite diagonal
        if (world.try_move_cell(x, y, x - dx, y + 1)) {
            world.get_cell(x - dx, y + 1).set_flow_direction(!flow_right);
            world.get_cell(x - dx, y + 1).velocity_y = cell.velocity_y;
            return;
        }
    }

    // Path trace downward (but with reduced distance to allow more spreading)
    int max_fall = std::min(static_cast<int>(cell.velocity_y), 8);  // Limit fall distance
    int best_y = y;

    for (int test_y = y + 1; test_y <= y + max_fall && test_y < 600; test_y++) {
        if (world.in_bounds(x, test_y) && world.can_move_to(x, y, x, test_y)) {
            best_y = test_y;
        } else {
            break;
        }
    }

    // Fall if possible
    if (best_y > y) {
        int8_t vel = cell.velocity_y;
        if (world.try_move_cell(x, y, x, best_y)) {
            world.get_cell(x, best_y).velocity_y = vel;
            world.get_cell(x, best_y).set_flow_direction(flow_right);
            return;
        }
    }

    // Hit bottom - keep velocity for horizontal flow
    if (cell.velocity_y > 3) {
        cell.velocity_y /= 2;
    } else {
        cell.velocity_y = 0;
    }

    // Try diagonal movement (with momentum bias)
    int dx = flow_right ? 1 : -1;

    if (world.try_move_cell(x, y, x + dx, y + 1)) {
        world.get_cell(x + dx, y + 1).set_flow_direction(flow_right);
        return;
    }
    if (world.try_move_cell(x, y, x - dx, y + 1)) {
        world.get_cell(x - dx, y + 1).set_flow_direction(!flow_right);
        return;
    }

    // Horizontal spreading with momentum
    // Try momentum direction first, with extended range
    for (int i = 1; i <= 4; i++) {
        if (world.try_move_cell(x, y, x + (dx * i), y)) {
            world.get_cell(x + (dx * i), y).set_flow_direction(flow_right);
            return;
        }
    }

    // Try opposite direction with shorter range
    for (int i = 1; i <= 2; i++) {
        if (world.try_move_cell(x, y, x - (dx * i), y)) {
            // Flip momentum when bouncing off obstacle
            world.get_cell(x - (dx * i), y).set_flow_direction(!flow_right);
            return;
        }
    }

    // Stuck - randomly flip direction for next frame
    if ((rand & 7) == 0) {
        cell.set_flow_direction(!flow_right);
    }
}

// Steam: RISES (opposite of sand!) - uses NEGATIVE velocity to go UP
void update_steam(World& world, int32_t x, int32_t y) {
    Cell& cell = world.get_cell(x, y);

    // Steam stays as a permanent gas - no condensation!

    // Apply NEGATIVE gravity (steam rises!) - opposite of sand/water
    cell.add_velocity(-2);  // Accelerate UPWARD
    cell.clamp_velocity(-20, 2);  // Negative = up, positive = down

    // PATH TRACING: Try to move along the entire velocity path (UPWARD!)
    int target_y = y + cell.velocity_y;  // velocity is negative, so target_y < y

    // Trace from current position UPWARD to target, stopping at first obstacle
    int best_y = y;
    for (int test_y = y - 1; test_y >= target_y && test_y >= 0; test_y--) {
        if (world.in_bounds(x, test_y) && world.can_move_to(x, y, x, test_y)) {
            best_y = test_y;
        } else {
            break;  // Hit obstacle, stop here
        }
    }

    // Move to the furthest free position we found (upward!)
    if (best_y < y) {
        int8_t vel = cell.velocity_y;
        if (world.try_move_cell(x, y, x, best_y)) {
            world.get_cell(x, best_y).velocity_y = vel;
            return;
        }
    }

    // Hit ceiling - reset velocity
    world.get_cell(x, y).reset_velocity();

    // Try diagonal rise with random horizontal drift
    uint32_t rand_val = world.random_int();
    bool try_left_first = (rand_val & 0x10) == 0;
    if (try_left_first) {
        if (world.try_move_cell(x, y, x - 1, y - 1)) return;
        if (world.try_move_cell(x, y, x + 1, y - 1)) return;
    } else {
        if (world.try_move_cell(x, y, x + 1, y - 1)) return;
        if (world.try_move_cell(x, y, x - 1, y - 1)) return;
    }

    // Horizontal dispersal (gases spread horizontally when can't rise)
    bool drift_right = (rand_val & 0x20) != 0;
    if (drift_right) {
        for (int i = 1; i <= 3; i++) {
            if (world.try_move_cell(x, y, x + i, y)) return;
        }
        for (int i = 1; i <= 2; i++) {
            if (world.try_move_cell(x, y, x - i, y)) return;
        }
    } else {
        for (int i = 1; i <= 3; i++) {
            if (world.try_move_cell(x, y, x - i, y)) return;
        }
        for (int i = 1; i <= 2; i++) {
            if (world.try_move_cell(x, y, x + i, y)) return;
        }
    }
}

// Oil: Flows like water but slower, floats on water
void update_oil(World& world, int32_t x, int32_t y) {
    // Oil moves every other frame (slower than water)
    if ((world.random_int() & 1) == 0) {
        return;  // Skip this frame
    }

    // Try to fall straight down
    if (world.try_move_cell(x, y, x, y + 1)) {
        return;
    }

    // Try diagonal fall (similar to water but simpler)
    bool try_left_first = (world.random_int() & 1) == 0;
    if (try_left_first) {
        if (world.try_move_cell(x, y, x - 1, y + 1)) return;
        if (world.try_move_cell(x, y, x + 1, y + 1)) return;
    } else {
        if (world.try_move_cell(x, y, x + 1, y + 1)) return;
        if (world.try_move_cell(x, y, x - 1, y + 1)) return;
    }

    // Spread horizontally (less aggressive than water - only 1 cell)
    if (try_left_first) {
        if (world.try_move_cell(x, y, x - 1, y)) return;
        if (world.try_move_cell(x, y, x + 1, y)) return;
    } else {
        if (world.try_move_cell(x, y, x + 1, y)) return;
        if (world.try_move_cell(x, y, x - 1, y)) return;
    }
}

// Fire: Rises, spreads to flammable materials, has lifetime
void update_fire(World& world, int32_t x, int32_t y) {
    // Check neighbors for fuel sources and interactions
    bool has_fuel = false;

    for (int dy = -1; dy <= 1; dy++) {
        for (int dx = -1; dx <= 1; dx++) {
            if (dx == 0 && dy == 0) continue;
            int nx = x + dx;
            int ny = y + dy;
            if (world.in_bounds(nx, ny)) {
                MaterialID neighbor = world.get_material(nx, ny);

                // Water extinguishes fire and turns to steam
                if (neighbor == MaterialID::Water) {
                    world.set_material(x, y, MaterialID::Steam);
                    world.get_cell(x, y).velocity_y = -5;
                    world.set_material(nx, ny, MaterialID::Steam);
                    world.get_cell(nx, ny).velocity_y = -5;
                    return;
                }

                // Check for fuel (wood, grass, oil, burning wood/grass)
                if (neighbor == MaterialID::Wood || neighbor == MaterialID::Grass ||
                    neighbor == MaterialID::Oil) {
                    has_fuel = true;

                    // Also check if wood/grass is burning (has lifetime)
                    if ((neighbor == MaterialID::Wood || neighbor == MaterialID::Grass) &&
                        world.get_cell(nx, ny).get_lifetime() > 0) {
                        has_fuel = true;
                    }
                }

                // Ignite oil directly (fire spreads fast to oil)
                if (neighbor == MaterialID::Oil && (world.random_int() & 3) == 0) {
                    world.set_material(nx, ny, MaterialID::Fire);
                    world.get_cell(nx, ny).set_lifetime(35);
                }
            }
        }
    }

    // Fire lifetime depends on fuel
    Cell& cell = world.get_cell(x, y);

    if (has_fuel) {
        // Extend lifetime when near fuel (fire keeps burning)
        if (cell.get_lifetime() < 25) {
            cell.set_lifetime(25);
        }
    } else {
        // No fuel - decrement lifetime normally
        cell.decrement_lifetime();
    }

    // If lifetime expired, turn into smoke occasionally
    if (cell.get_lifetime() == 0) {
        // 30% chance to turn into smoke instead of disappearing
        if ((world.random_int() & 7) < 2) {
            world.set_material(x, y, MaterialID::Smoke);
            world.get_cell(x, y).set_lifetime(30);
            world.get_cell(x, y).velocity_y = -3;
        } else {
            world.set_material(x, y, MaterialID::Empty);
        }
        return;
    }

    // Try to rise (fire goes up)
    if (world.try_move_cell(x, y, x, y - 1)) {
        return;
    }

    // Try diagonal rise
    bool try_left_first = (world.random_int() & 1) == 0;
    if (try_left_first) {
        if (world.try_move_cell(x, y, x - 1, y - 1)) return;
        if (world.try_move_cell(x, y, x + 1, y - 1)) return;
    } else {
        if (world.try_move_cell(x, y, x + 1, y - 1)) return;
        if (world.try_move_cell(x, y, x - 1, y - 1)) return;
    }

    // Spread horizontally (fire spreads)
    if (try_left_first) {
        if (world.try_move_cell(x, y, x - 1, y)) return;
        if (world.try_move_cell(x, y, x + 1, y)) return;
    } else {
        if (world.try_move_cell(x, y, x + 1, y)) return;
        if (world.try_move_cell(x, y, x - 1, y)) return;
    }
}

// Wood: Static solid that can burn and float
void update_wood(World& world, int32_t x, int32_t y) {
    Cell& cell = world.get_cell(x, y);

    // Check if wood is currently burning (lifetime > 0 means burning)
    uint8_t burn_progress = cell.get_lifetime();

    if (burn_progress > 0) {
        // Wood is burning - decrement burn timer
        cell.decrement_lifetime();

        // When burn timer reaches 0, convert to fire or ash
        if (cell.get_lifetime() == 0) {
            if ((world.random_int() & 3) == 0) {
                // 25% chance to turn into ash
                world.set_material(x, y, MaterialID::Ash);
                world.get_cell(x, y).velocity_y = -2;
            } else {
                // 75% chance to turn into fire
                world.set_material(x, y, MaterialID::Fire);
                world.get_cell(x, y).set_lifetime(30);
                world.get_cell(x, y).velocity_y = -4;
            }
            return;
        }

        // While burning, occasionally spread fire to adjacent wood
        if ((world.random_int() & 15) == 0) {  // ~6% chance per frame
            for (int dy = -1; dy <= 1; dy++) {
                for (int dx = -1; dx <= 1; dx++) {
                    if (dx == 0 && dy == 0) continue;
                    int nx = x + dx;
                    int ny = y + dy;
                    if (world.in_bounds(nx, ny)) {
                        MaterialID neighbor = world.get_material(nx, ny);

                        // Ignite adjacent wood (with low chance for gradual spread)
                        if (neighbor == MaterialID::Wood) {
                            Cell& neighbor_cell = world.get_cell(nx, ny);
                            if (neighbor_cell.get_lifetime() == 0) {
                                // Start burning this wood
                                neighbor_cell.set_lifetime(40 + (world.random_int() & 15));  // 40-55 frames
                            }
                        }
                    }
                }
            }
        }
    } else {
        // Wood is not burning yet - check for ignition sources
        bool should_ignite = false;

        for (int dy = -1; dy <= 1; dy++) {
            for (int dx = -1; dx <= 1; dx++) {
                if (dx == 0 && dy == 0) continue;
                int nx = x + dx;
                int ny = y + dy;
                if (world.in_bounds(nx, ny)) {
                    MaterialID neighbor = world.get_material(nx, ny);

                    // Fire ignites wood with low probability (gradual spread)
                    if (neighbor == MaterialID::Fire && (world.random_int() & 31) == 0) {
                        should_ignite = true;
                        break;
                    }

                    // Lava ignites wood more reliably
                    if (neighbor == MaterialID::Lava && (world.random_int() & 7) == 0) {
                        should_ignite = true;
                        break;
                    }
                }
            }
            if (should_ignite) break;
        }

        if (should_ignite) {
            // Start burning - set burn timer
            cell.set_lifetime(40 + (world.random_int() & 15));  // 40-55 frames to burn
        }
    }

    // Wood floats on water - try to rise if submerged (and not burning)
    if (burn_progress == 0 && world.in_bounds(x, y - 1)) {
        MaterialID above = world.get_material(x, y - 1);
        if (above == MaterialID::Water && (world.random_int() & 3) == 0) {
            world.try_move_cell(x, y, x, y - 1);
        }
    }
}

// Acid: Corrosive liquid that dissolves solids
void update_acid(World& world, int32_t x, int32_t y) {
    Cell& cell = world.get_cell(x, y);

    // Check for corrosion reactions first
    for (int dy = -1; dy <= 1; dy++) {
        for (int dx = -1; dx <= 1; dx++) {
            if (dx == 0 && dy == 0) continue;
            int nx = x + dx;
            int ny = y + dy;
            if (world.in_bounds(nx, ny)) {
                MaterialID neighbor = world.get_material(nx, ny);

                // Corrode solids (except stone has resistance)
                if (neighbor == MaterialID::Wood || neighbor == MaterialID::Grass) {
                    if ((world.random_int() & 3) == 0) {
                        world.set_material(nx, ny, MaterialID::Empty);
                    }
                } else if (neighbor == MaterialID::Stone && (world.random_int() & 31) == 0) {
                    // Stone corrodes very slowly
                    world.set_material(nx, ny, MaterialID::Empty);
                }

                // React with lava (create smoke/bubbles)
                if (neighbor == MaterialID::Lava && (world.random_int() & 7) == 0) {
                    world.set_material(nx, ny, MaterialID::Smoke);
                    world.get_cell(nx, ny).set_lifetime(40);
                    world.get_cell(nx, ny).velocity_y = -5;
                }
            }
        }
    }

    // Apply gravity (similar to water but slightly slower)
    cell.add_velocity(2);
    cell.clamp_velocity(0, 18);

    // Path trace downward
    int best_y = y;
    for (int test_y = y + 1; test_y <= y + cell.velocity_y && test_y < 600; test_y++) {
        if (world.in_bounds(x, test_y) && world.can_move_to(x, y, x, test_y)) {
            best_y = test_y;
        } else {
            break;
        }
    }

    // Fall if possible
    if (best_y > y) {
        int8_t vel = cell.velocity_y;
        if (world.try_move_cell(x, y, x, best_y)) {
            world.get_cell(x, best_y).velocity_y = vel;
            return;
        }
    }

    // Reset velocity on impact
    if (cell.velocity_y > 3) {
        cell.velocity_y /= 2;
    } else {
        cell.velocity_y = 0;
    }

    // Try diagonal
    bool try_left_first = (world.random_int() & 1) == 0;
    if (try_left_first) {
        if (world.try_move_cell(x, y, x - 1, y + 1)) return;
        if (world.try_move_cell(x, y, x + 1, y + 1)) return;
    } else {
        if (world.try_move_cell(x, y, x + 1, y + 1)) return;
        if (world.try_move_cell(x, y, x - 1, y + 1)) return;
    }

    // Horizontal spreading (less than water)
    if (try_left_first) {
        for (int i = 1; i <= 3; i++) {
            if (world.try_move_cell(x, y, x - i, y)) return;
        }
        for (int i = 1; i <= 2; i++) {
            if (world.try_move_cell(x, y, x + i, y)) return;
        }
    } else {
        for (int i = 1; i <= 3; i++) {
            if (world.try_move_cell(x, y, x + i, y)) return;
        }
        for (int i = 1; i <= 2; i++) {
            if (world.try_move_cell(x, y, x - i, y)) return;
        }
    }
}

// Lava: Heavy liquid that burns and solidifies
void update_lava(World& world, int32_t x, int32_t y) {
    Cell& cell = world.get_cell(x, y);

    // Check for reactions
    for (int dy = -1; dy <= 1; dy++) {
        for (int dx = -1; dx <= 1; dx++) {
            if (dx == 0 && dy == 0) continue;
            int nx = x + dx;
            int ny = y + dy;
            if (world.in_bounds(nx, ny)) {
                MaterialID neighbor = world.get_material(nx, ny);

                // Solidify on contact with water
                if (neighbor == MaterialID::Water) {
                    world.set_material(x, y, MaterialID::Stone);
                    world.set_material(nx, ny, MaterialID::Steam);
                    world.get_cell(nx, ny).velocity_y = -5;
                    return;
                }

                // Burn wood and grass
                if ((neighbor == MaterialID::Wood || neighbor == MaterialID::Grass) &&
                    (world.random_int() & 3) == 0) {
                    world.set_material(nx, ny, MaterialID::Fire);
                    world.get_cell(nx, ny).set_lifetime(20);
                }

                // Ignite oil
                if (neighbor == MaterialID::Oil && (world.random_int() & 1) == 0) {
                    world.set_material(nx, ny, MaterialID::Fire);
                    world.get_cell(nx, ny).set_lifetime(30);
                }
            }
        }
    }

    // Lava flows slowly (slower than water, faster than oil)
    if ((world.random_int() & 1) == 1) {
        return;  // Skip this frame half the time
    }

    // Apply gravity
    cell.add_velocity(1);
    cell.clamp_velocity(0, 10);

    // Path trace downward
    int best_y = y;
    for (int test_y = y + 1; test_y <= y + cell.velocity_y && test_y < 600; test_y++) {
        if (world.in_bounds(x, test_y) && world.can_move_to(x, y, x, test_y)) {
            best_y = test_y;
        } else {
            break;
        }
    }

    // Fall if possible
    if (best_y > y) {
        int8_t vel = cell.velocity_y;
        if (world.try_move_cell(x, y, x, best_y)) {
            world.get_cell(x, best_y).velocity_y = vel;
            return;
        }
    }

    // Reset velocity
    cell.velocity_y = 0;

    // Try diagonal
    bool try_left_first = (world.random_int() & 1) == 0;
    if (try_left_first) {
        if (world.try_move_cell(x, y, x - 1, y + 1)) return;
        if (world.try_move_cell(x, y, x + 1, y + 1)) return;
    } else {
        if (world.try_move_cell(x, y, x + 1, y + 1)) return;
        if (world.try_move_cell(x, y, x - 1, y + 1)) return;
    }

    // Horizontal spreading (minimal)
    if (try_left_first) {
        if (world.try_move_cell(x, y, x - 1, y)) return;
        if (world.try_move_cell(x, y, x + 1, y)) return;
    } else {
        if (world.try_move_cell(x, y, x + 1, y)) return;
        if (world.try_move_cell(x, y, x - 1, y)) return;
    }
}

// Ash: Light powder that rises slowly then settles
void update_ash(World& world, int32_t x, int32_t y) {
    Cell& cell = world.get_cell(x, y);

    // Ash has mixed behavior: rises initially (if has upward velocity), then settles
    if (cell.velocity_y < 0) {
        // Rising phase (like a light gas)
        cell.add_velocity(1);  // Slow deceleration
        cell.clamp_velocity(-10, 5);

        // Try to rise
        if (cell.velocity_y < 0) {
            int target_y = y + cell.velocity_y;
            int best_y = y;

            for (int test_y = y - 1; test_y >= target_y && test_y >= 0; test_y--) {
                if (world.in_bounds(x, test_y) && world.can_move_to(x, y, x, test_y)) {
                    best_y = test_y;
                } else {
                    break;
                }
            }

            if (best_y < y) {
                int8_t vel = cell.velocity_y;
                if (world.try_move_cell(x, y, x, best_y)) {
                    world.get_cell(x, best_y).velocity_y = vel;
                    return;
                }
            }
        }

        // Can't rise, transition to settling
        cell.velocity_y = 0;
    }

    // Settling phase (like sand, but lighter)
    if ((world.random_int() & 3) != 0) {
        return;  // Only move occasionally
    }

    // Try to fall
    if (world.try_move_cell(x, y, x, y + 1)) {
        return;
    }

    // Try diagonal
    bool try_left_first = (world.random_int() & 1) == 0;
    if (try_left_first) {
        if (world.try_move_cell(x, y, x - 1, y + 1)) return;
        if (world.try_move_cell(x, y, x + 1, y + 1)) return;
    } else {
        if (world.try_move_cell(x, y, x + 1, y + 1)) return;
        if (world.try_move_cell(x, y, x - 1, y + 1)) return;
    }
}

// Grass: Static solid that can burn
void update_grass(World& world, int32_t x, int32_t y) {
    Cell& cell = world.get_cell(x, y);

    // Check if grass is currently burning (lifetime > 0 means burning)
    uint8_t burn_progress = cell.get_lifetime();

    if (burn_progress > 0) {
        // Grass is burning - decrement burn timer
        cell.decrement_lifetime();

        // When burn timer reaches 0, convert to fire
        if (cell.get_lifetime() == 0) {
            world.set_material(x, y, MaterialID::Fire);
            world.get_cell(x, y).set_lifetime(15);
            world.get_cell(x, y).velocity_y = -4;
            return;
        }

        // While burning, quickly spread to adjacent grass (grass burns fast!)
        if ((world.random_int() & 7) == 0) {  // ~12% chance per frame
            for (int dy = -1; dy <= 1; dy++) {
                for (int dx = -1; dx <= 1; dx++) {
                    if (dx == 0 && dy == 0) continue;
                    int nx = x + dx;
                    int ny = y + dy;
                    if (world.in_bounds(nx, ny)) {
                        MaterialID neighbor = world.get_material(nx, ny);

                        // Ignite adjacent grass
                        if (neighbor == MaterialID::Grass) {
                            Cell& neighbor_cell = world.get_cell(nx, ny);
                            if (neighbor_cell.get_lifetime() == 0) {
                                // Start burning this grass (burns faster than wood)
                                neighbor_cell.set_lifetime(10 + (world.random_int() & 7));  // 10-17 frames
                            }
                        }
                    }
                }
            }
        }
    } else {
        // Grass is not burning yet - check for ignition sources
        bool should_ignite = false;

        for (int dy = -1; dy <= 1; dy++) {
            for (int dx = -1; dx <= 1; dx++) {
                if (dx == 0 && dy == 0) continue;
                int nx = x + dx;
                int ny = y + dy;
                if (world.in_bounds(nx, ny)) {
                    MaterialID neighbor = world.get_material(nx, ny);

                    // Fire ignites grass easily
                    if (neighbor == MaterialID::Fire && (world.random_int() & 7) == 0) {
                        should_ignite = true;
                        break;
                    }

                    // Lava ignites grass very easily
                    if (neighbor == MaterialID::Lava && (world.random_int() & 3) == 0) {
                        should_ignite = true;
                        break;
                    }
                }
            }
            if (should_ignite) break;
        }

        if (should_ignite) {
            // Start burning - grass burns quickly
            cell.set_lifetime(10 + (world.random_int() & 7));  // 10-17 frames
        }
    }
}

// Smoke: Rises slowly and dissipates over time
void update_smoke(World& world, int32_t x, int32_t y) {
    Cell& cell = world.get_cell(x, y);

    // Decrement lifetime
    cell.decrement_lifetime();
    if (cell.get_lifetime() == 0) {
        world.set_material(x, y, MaterialID::Empty);
        return;
    }

    // Apply negative gravity (smoke rises)
    cell.add_velocity(-1);  // Slower rise than steam
    cell.clamp_velocity(-10, 2);

    // PATH TRACING: Try to move upward
    int target_y = y + cell.velocity_y;
    int best_y = y;

    for (int test_y = y - 1; test_y >= target_y && test_y >= 0; test_y--) {
        if (world.in_bounds(x, test_y) && world.can_move_to(x, y, x, test_y)) {
            best_y = test_y;
        } else {
            break;
        }
    }

    // Move upward if possible
    if (best_y < y) {
        int8_t vel = cell.velocity_y;
        if (world.try_move_cell(x, y, x, best_y)) {
            world.get_cell(x, best_y).velocity_y = vel;
            return;
        }
    }

    // Hit ceiling - reset velocity
    cell.reset_velocity();

    // Try diagonal rise
    uint32_t rand = world.random_int();
    bool try_left_first = (rand & 1) == 0;
    if (try_left_first) {
        if (world.try_move_cell(x, y, x - 1, y - 1)) return;
        if (world.try_move_cell(x, y, x + 1, y - 1)) return;
    } else {
        if (world.try_move_cell(x, y, x + 1, y - 1)) return;
        if (world.try_move_cell(x, y, x - 1, y - 1)) return;
    }

    // Horizontal dispersal
    bool drift_right = (rand & 2) != 0;
    if (drift_right) {
        for (int i = 1; i <= 2; i++) {
            if (world.try_move_cell(x, y, x + i, y)) return;
        }
        if (world.try_move_cell(x, y, x - 1, y)) return;
    } else {
        for (int i = 1; i <= 2; i++) {
            if (world.try_move_cell(x, y, x - i, y)) return;
        }
        if (world.try_move_cell(x, y, x + 1, y)) return;
    }
}

// ============================================================================
// Person: Village-Building AI with Lifelike Movement
// ============================================================================
// People now build houses and form villages! They have distinct personalities,
// realistic movement patterns, and cooperate to construct structures.

// Helper: Check if material is solid ground for walking/standing
static bool is_person_ground(MaterialID m) {
    return m == MaterialID::Stone || m == MaterialID::Wood ||
           m == MaterialID::Grass || m == MaterialID::Sand ||
           m == MaterialID::Brick || m == MaterialID::Dirt ||
           m == MaterialID::Metal || m == MaterialID::Person;
}

// Helper: Check if material is passable for movement
static bool is_passable(MaterialID m) {
    return m == MaterialID::Empty || m == MaterialID::Water ||
           m == MaterialID::Steam || m == MaterialID::Smoke ||
           m == MaterialID::Helium || m == MaterialID::Hydrogen;
}

// Helper: Find ground level at a given x position (scan downward)
// Returns the Y coordinate of the first empty cell that has solid ground below it
static int32_t find_ground_level(World& world, int32_t x, int32_t start_y) {
    // Start from a reasonable height and scan down
    int32_t scan_start = std::max(1, start_y);
    for (int32_t y = scan_start; y < WORLD_HEIGHT - 1; y++) {
        if (world.in_bounds(x, y) && world.in_bounds(x, y + 1)) {
            MaterialID here = world.get_material(x, y);
            MaterialID below = world.get_material(x, y + 1);
            // Found empty space with solid ground below
            if (is_passable(here) && is_person_ground(below)) {
                return y;
            }
        }
    }
    return -1; // No ground found
}

// Helper: Check if an area is clear for building
static bool is_area_clear(World& world, int32_t x, int32_t y, int32_t width, int32_t height) {
    for (int32_t dy = 0; dy < height; dy++) {
        for (int32_t dx = 0; dx < width; dx++) {
            int32_t cx = x + dx;
            int32_t cy = y - dy; // Build upward
            if (!world.in_bounds(cx, cy)) return false;
            MaterialID m = world.get_material(cx, cy);
            if (m != MaterialID::Empty && m != MaterialID::Steam &&
                m != MaterialID::Smoke && m != MaterialID::Water) {
                return false;
            }
        }
    }
    return true;
}

// Helper: Place a block of building material
static void place_building_block(World& world, int32_t x, int32_t y, MaterialID material) {
    if (world.in_bounds(x, y)) {
        MaterialID current = world.get_material(x, y);
        if (current == MaterialID::Empty || current == MaterialID::Steam ||
            current == MaterialID::Smoke) {
            world.set_material(x, y, material);
        }
    }
}

// Build a small hut (5 wide, 4 tall)
static void build_small_hut(World& world, int32_t base_x, int32_t base_y, uint32_t& /*seed*/) {
    // Floor
    for (int dx = 0; dx < 5; dx++) {
        place_building_block(world, base_x + dx, base_y, MaterialID::Wood);
    }

    // Walls (3 tall)
    for (int dy = 1; dy <= 3; dy++) {
        place_building_block(world, base_x, base_y - dy, MaterialID::Wood);
        place_building_block(world, base_x + 4, base_y - dy, MaterialID::Wood);
    }

    // Roof (simple flat roof)
    for (int dx = 0; dx < 5; dx++) {
        place_building_block(world, base_x + dx, base_y - 4, MaterialID::Stone);
    }

    // Door opening (leave middle open)
    // The door is implicitly the gap in the middle of the front wall
}

// Build a proper house (8 wide, 6 tall with peaked roof)
static void build_house(World& world, int32_t base_x, int32_t base_y, uint32_t& /*seed*/) {
    // Foundation
    for (int dx = 0; dx < 8; dx++) {
        place_building_block(world, base_x + dx, base_y, MaterialID::Stone);
    }

    // Walls (4 tall)
    for (int dy = 1; dy <= 4; dy++) {
        // Left wall
        place_building_block(world, base_x, base_y - dy, MaterialID::Brick);
        // Right wall
        place_building_block(world, base_x + 7, base_y - dy, MaterialID::Brick);
        // Back wall sections (with window gap in middle)
        if (dy != 2 && dy != 3) {
            for (int dx = 1; dx < 7; dx++) {
                // Skip door area (middle 2 blocks at bottom)
                if (dy == 1 && (dx == 3 || dx == 4)) continue;
            }
        }
    }

    // Front and back walls with gaps
    for (int dx = 1; dx < 7; dx++) {
        // Back wall top
        place_building_block(world, base_x + dx, base_y - 4, MaterialID::Brick);
        // Skip middle for door at ground level
        if (dx != 3 && dx != 4) {
            place_building_block(world, base_x + dx, base_y - 1, MaterialID::Brick);
        }
    }

    // Peaked roof
    for (int level = 0; level < 3; level++) {
        int start = level;
        int end = 8 - level;
        for (int dx = start; dx < end; dx++) {
            place_building_block(world, base_x + dx, base_y - 5 - level, MaterialID::Wood);
        }
    }
}

// Build a tower (4 wide, 10 tall)
static void build_tower(World& world, int32_t base_x, int32_t base_y, uint32_t& /*seed*/) {
    // Foundation
    for (int dx = 0; dx < 4; dx++) {
        place_building_block(world, base_x + dx, base_y, MaterialID::Stone);
    }

    // Tower walls (8 tall)
    for (int dy = 1; dy <= 8; dy++) {
        place_building_block(world, base_x, base_y - dy, MaterialID::Stone);
        place_building_block(world, base_x + 3, base_y - dy, MaterialID::Stone);
        // Windows every 3 levels
        if (dy % 3 != 0) {
            place_building_block(world, base_x + 1, base_y - dy, MaterialID::Stone);
            place_building_block(world, base_x + 2, base_y - dy, MaterialID::Stone);
        }
    }

    // Crenellations at top
    place_building_block(world, base_x, base_y - 9, MaterialID::Stone);
    place_building_block(world, base_x + 3, base_y - 9, MaterialID::Stone);
    place_building_block(world, base_x, base_y - 10, MaterialID::Stone);
    place_building_block(world, base_x + 3, base_y - 10, MaterialID::Stone);
}

// Build a platform/bridge
static void build_platform(World& world, int32_t base_x, int32_t base_y, int32_t length, uint32_t& /*seed*/) {
    for (int dx = 0; dx < length; dx++) {
        place_building_block(world, base_x + dx, base_y, MaterialID::Wood);
    }
}

// Main person update function - simplified for stable movement
void update_person(World& world, int32_t x, int32_t y) {
    Cell& cell = world.get_cell(x, y);

    // Death check
    if (cell.get_health() == 0) {
        world.set_material(x, y, MaterialID::Smoke);
        world.get_cell(x, y).set_lifetime(15);
        return;
    }

    // Frame counter (0-63) for timing behaviors
    uint8_t frame = cell.get_lifetime();
    cell.set_lifetime((frame + 1) & 63);

    // Get stable personality from health
    uint8_t personality = cell.get_health();
    bool is_jumpy = (personality & 0x02) != 0;

    // Get facing direction
    bool facing_right = cell.get_person_facing_right();

    // ========================================
    // GRAVITY - Always check if we should fall
    // ========================================
    bool grounded = false;
    if (world.in_bounds(x, y + 1)) {
        MaterialID below = world.get_material(x, y + 1);
        grounded = is_person_ground(below);
    }

    if (!grounded) {
        // Try to fall
        if (world.try_move_cell(x, y, x, y + 1)) {
            return;
        }
        // Can't fall - we're stuck somehow, try moving sideways
        int side = facing_right ? 1 : -1;
        if (world.in_bounds(x + side, y + 1) && is_passable(world.get_material(x + side, y + 1))) {
            world.try_move_cell(x, y, x + side, y + 1);
        }
        return;
    }

    // ========================================
    // GROUNDED MOVEMENT - Only move every 4 frames
    // ========================================
    if ((frame & 3) != 0) {
        return;  // Skip this frame - stand still
    }

    int dir = facing_right ? 1 : -1;
    int next_x = x + dir;

    // Bounds check
    if (!world.in_bounds(next_x, y)) {
        cell.set_person_facing_right(!facing_right);
        return;
    }

    MaterialID ahead = world.get_material(next_x, y);

    // ===== CASE 1: Path is clear - walk forward =====
    if (is_passable(ahead)) {
        world.try_move_cell(x, y, next_x, y);
        return;
    }

    // ===== CASE 2: Blocked by another person - wait or go around =====
    if (ahead == MaterialID::Person) {
        // 50% chance to turn around, 50% chance to wait
        if ((frame & 4) != 0) {
            cell.set_person_facing_right(!facing_right);
        }
        return;
    }

    // ===== CASE 3: Blocked by solid - try to climb or jump =====

    // Try climbing 1-3 pixels
    for (int h = 1; h <= 3; h++) {
        int climb_y = y - h;

        // Check space above us is clear
        bool can_go_up = true;
        for (int check_y = y - 1; check_y >= climb_y; check_y--) {
            if (!world.in_bounds(x, check_y) || !is_passable(world.get_material(x, check_y))) {
                can_go_up = false;
                break;
            }
        }
        if (!can_go_up) break;  // Can't go higher

        // Check destination is clear
        if (world.in_bounds(next_x, climb_y) && is_passable(world.get_material(next_x, climb_y))) {
            if (world.try_move_cell(x, y, next_x, climb_y)) {
                return;  // Climbed successfully
            }
        }
    }

    // Try jumping (only on certain frames)
    if ((frame & 15) == 0) {
        int jump_height = is_jumpy ? 4 : 2;

        // Find how high we can jump
        int actual_height = 0;
        for (int h = 1; h <= jump_height; h++) {
            if (world.in_bounds(x, y - h) && is_passable(world.get_material(x, y - h))) {
                actual_height = h;
            } else {
                break;
            }
        }

        if (actual_height >= 2) {
            // Try diagonal jump
            if (world.in_bounds(next_x, y - actual_height) &&
                is_passable(world.get_material(next_x, y - actual_height))) {
                if (world.try_move_cell(x, y, next_x, y - actual_height)) {
                    return;
                }
            }
            // Try straight up jump
            if (world.try_move_cell(x, y, x, y - actual_height)) {
                return;
            }
        }
    }

    // Can't move forward - turn around
    cell.set_person_facing_right(!facing_right);
}

// ============================================================================
// NEW MATERIAL UPDATE IMPLEMENTATIONS
// ============================================================================

// Helper: Generic powder behavior (like sand)
static void generic_powder_update(World& world, int32_t x, int32_t y, int gravity = 2, int max_vel = 15) {
    Cell& cell = world.get_cell(x, y);
    cell.add_velocity(gravity);
    cell.clamp_velocity(0, max_vel);

    int target_y = y + cell.velocity_y;
    int best_y = y;
    for (int test_y = y + 1; test_y <= target_y && test_y < 600; test_y++) {
        if (world.in_bounds(x, test_y) && world.can_move_to(x, y, x, test_y)) {
            best_y = test_y;
        } else {
            break;
        }
    }

    if (best_y > y) {
        int8_t vel = cell.velocity_y;
        if (world.try_move_cell(x, y, x, best_y)) {
            world.get_cell(x, best_y).velocity_y = vel;
            return;
        }
    }

    world.get_cell(x, y).reset_velocity();
    bool try_left = (world.random_int() & 1) == 0;
    if (try_left) {
        if (world.try_move_cell(x, y, x - 1, y + 1)) return;
        if (world.try_move_cell(x, y, x + 1, y + 1)) return;
    } else {
        if (world.try_move_cell(x, y, x + 1, y + 1)) return;
        if (world.try_move_cell(x, y, x - 1, y + 1)) return;
    }
}

// Helper: Generic gas behavior (rises)
static void generic_gas_update(World& world, int32_t x, int32_t y, int rise_speed = -2, int max_vel = -15, bool has_lifetime = false) {
    Cell& cell = world.get_cell(x, y);

    if (has_lifetime) {
        cell.decrement_lifetime();
        if (cell.get_lifetime() == 0) {
            world.set_material(x, y, MaterialID::Empty);
            return;
        }
    }

    cell.add_velocity(rise_speed);
    cell.clamp_velocity(max_vel, 2);

    int target_y = y + cell.velocity_y;
    int best_y = y;
    for (int test_y = y - 1; test_y >= target_y && test_y >= 0; test_y--) {
        if (world.in_bounds(x, test_y) && world.can_move_to(x, y, x, test_y)) {
            best_y = test_y;
        } else {
            break;
        }
    }

    if (best_y < y) {
        int8_t vel = cell.velocity_y;
        if (world.try_move_cell(x, y, x, best_y)) {
            world.get_cell(x, best_y).velocity_y = vel;
            return;
        }
    }

    cell.reset_velocity();
    uint32_t rand = world.random_int();
    bool try_left = (rand & 1) == 0;
    if (try_left) {
        if (world.try_move_cell(x, y, x - 1, y - 1)) return;
        if (world.try_move_cell(x, y, x + 1, y - 1)) return;
    } else {
        if (world.try_move_cell(x, y, x + 1, y - 1)) return;
        if (world.try_move_cell(x, y, x - 1, y - 1)) return;
    }

    // Horizontal dispersion
    bool drift_right = (rand & 2) != 0;
    if (drift_right) {
        for (int i = 1; i <= 2; i++) {
            if (world.try_move_cell(x, y, x + i, y)) return;
        }
        if (world.try_move_cell(x, y, x - 1, y)) return;
    } else {
        for (int i = 1; i <= 2; i++) {
            if (world.try_move_cell(x, y, x - i, y)) return;
        }
        if (world.try_move_cell(x, y, x + 1, y)) return;
    }
}

// Helper: Generic slow liquid behavior
static void generic_slow_liquid_update(World& world, int32_t x, int32_t y, int skip_chance = 1) {
    if ((world.random_int() & skip_chance) != 0) return;

    if (world.try_move_cell(x, y, x, y + 1)) return;

    bool try_left = (world.random_int() & 1) == 0;
    if (try_left) {
        if (world.try_move_cell(x, y, x - 1, y + 1)) return;
        if (world.try_move_cell(x, y, x + 1, y + 1)) return;
        if (world.try_move_cell(x, y, x - 1, y)) return;
        if (world.try_move_cell(x, y, x + 1, y)) return;
    } else {
        if (world.try_move_cell(x, y, x + 1, y + 1)) return;
        if (world.try_move_cell(x, y, x - 1, y + 1)) return;
        if (world.try_move_cell(x, y, x + 1, y)) return;
        if (world.try_move_cell(x, y, x - 1, y)) return;
    }
}

// ============================================================================
// POWDERS (10-19)
// ============================================================================

void update_dirt(World& world, int32_t x, int32_t y) {
    // Check for material combinations (e.g., dirt + water = mud)
    if (try_material_combination(world, x, y)) return;
    generic_powder_update(world, x, y, 2, 12);
}

void update_gravel(World& world, int32_t x, int32_t y) {
    generic_powder_update(world, x, y, 3, 18);  // Heavier, falls faster
}

void update_snow(World& world, int32_t x, int32_t y) {
    // Check for combinations (snow + snow = ice)
    if (try_material_combination(world, x, y)) return;
    // Snow melts near fire/lava
    for (int dy = -1; dy <= 1; dy++) {
        for (int dx = -1; dx <= 1; dx++) {
            int nx = x + dx, ny = y + dy;
            if (world.in_bounds(nx, ny)) {
                MaterialID m = world.get_material(nx, ny);
                if (m == MaterialID::Fire || m == MaterialID::Lava || m == MaterialID::Dragon_Fire) {
                    world.set_material(x, y, MaterialID::Water);
                    return;
                }
            }
        }
    }
    generic_powder_update(world, x, y, 1, 8);  // Light and slow
}

void update_gunpowder(World& world, int32_t x, int32_t y) {
    // Explodes on contact with fire/spark/lava
    for (int dy = -1; dy <= 1; dy++) {
        for (int dx = -1; dx <= 1; dx++) {
            int nx = x + dx, ny = y + dy;
            if (world.in_bounds(nx, ny)) {
                MaterialID m = world.get_material(nx, ny);
                if (m == MaterialID::Fire || m == MaterialID::Spark ||
                    m == MaterialID::Lava || m == MaterialID::Lightning) {
                    // Small explosion - turn into fire and ignite neighbors
                    world.set_material(x, y, MaterialID::Fire);
                    world.get_cell(x, y).set_lifetime(20);
                    // Chain reaction
                    for (int ey = -2; ey <= 2; ey++) {
                        for (int ex = -2; ex <= 2; ex++) {
                            int fx = x + ex, fy = y + ey;
                            if (world.in_bounds(fx, fy)) {
                                MaterialID fm = world.get_material(fx, fy);
                                if (fm == MaterialID::Gunpowder && (world.random_int() & 3) == 0) {
                                    world.set_material(fx, fy, MaterialID::Fire);
                                    world.get_cell(fx, fy).set_lifetime(15);
                                }
                            }
                        }
                    }
                    return;
                }
            }
        }
    }
    generic_powder_update(world, x, y, 2, 12);
}

void update_salt(World& world, int32_t x, int32_t y) {
    // Check for combinations (salt dissolves in water)
    if (try_material_combination(world, x, y)) return;
    // Salt dissolves in water
    for (int dy = -1; dy <= 1; dy++) {
        for (int dx = -1; dx <= 1; dx++) {
            int nx = x + dx, ny = y + dy;
            if (world.in_bounds(nx, ny)) {
                if (world.get_material(nx, ny) == MaterialID::Water) {
                    if ((world.random_int() & 15) == 0) {
                        world.set_material(x, y, MaterialID::Empty);
                        return;
                    }
                }
            }
        }
    }
    generic_powder_update(world, x, y, 2, 14);
}

void update_coal(World& world, int32_t x, int32_t y) {
    // Check for combinations (coal + magic = diamond)
    if (try_material_combination(world, x, y)) return;

    Cell& cell = world.get_cell(x, y);

    // Coal burns like wood when ignited
    if (cell.get_lifetime() > 0) {
        cell.decrement_lifetime();
        if (cell.get_lifetime() == 0) {
            if ((world.random_int() & 3) == 0) {
                world.set_material(x, y, MaterialID::Ash);
            } else {
                world.set_material(x, y, MaterialID::Fire);
                world.get_cell(x, y).set_lifetime(25);
            }
            return;
        }
        // Spread fire to other coal
        if ((world.random_int() & 15) == 0) {
            for (int dy = -1; dy <= 1; dy++) {
                for (int dx = -1; dx <= 1; dx++) {
                    int nx = x + dx, ny = y + dy;
                    if (world.in_bounds(nx, ny) &&
                        world.get_material(nx, ny) == MaterialID::Coal &&
                        world.get_cell(nx, ny).get_lifetime() == 0) {
                        world.get_cell(nx, ny).set_lifetime(50 + (world.random_int() & 15));
                    }
                }
            }
        }
    } else {
        // Check for ignition
        for (int dy = -1; dy <= 1; dy++) {
            for (int dx = -1; dx <= 1; dx++) {
                int nx = x + dx, ny = y + dy;
                if (world.in_bounds(nx, ny)) {
                    MaterialID m = world.get_material(nx, ny);
                    if ((m == MaterialID::Fire || m == MaterialID::Lava) &&
                        (world.random_int() & 31) == 0) {
                        cell.set_lifetime(60);  // Coal burns slowly
                        return;
                    }
                }
            }
        }
    }
    generic_powder_update(world, x, y, 2, 14);
}

void update_rust(World& world, int32_t x, int32_t y) {
    generic_powder_update(world, x, y, 2, 16);
}

void update_sawdust(World& world, int32_t x, int32_t y) {
    // Check for combinations (sawdust + coal = coal)
    if (try_material_combination(world, x, y)) return;
    // Sawdust is very flammable
    for (int dy = -1; dy <= 1; dy++) {
        for (int dx = -1; dx <= 1; dx++) {
            int nx = x + dx, ny = y + dy;
            if (world.in_bounds(nx, ny)) {
                MaterialID m = world.get_material(nx, ny);
                if (m == MaterialID::Fire || m == MaterialID::Lava || m == MaterialID::Spark) {
                    world.set_material(x, y, MaterialID::Fire);
                    world.get_cell(x, y).set_lifetime(15);
                    return;
                }
            }
        }
    }
    generic_powder_update(world, x, y, 1, 10);  // Light powder
}

void update_glass_powder(World& world, int32_t x, int32_t y) {
    // Glass powder melts into glass near extreme heat (lava)
    for (int dy = -1; dy <= 1; dy++) {
        for (int dx = -1; dx <= 1; dx++) {
            int nx = x + dx, ny = y + dy;
            if (world.in_bounds(nx, ny)) {
                if (world.get_material(nx, ny) == MaterialID::Lava) {
                    if ((world.random_int() & 7) == 0) {
                        world.set_material(x, y, MaterialID::Glass);
                        return;
                    }
                }
            }
        }
    }
    generic_powder_update(world, x, y, 2, 15);
}

// ============================================================================
// LIQUIDS (20-29)
// ============================================================================

void update_honey(World& world, int32_t x, int32_t y) {
    // Check for combinations (honey + water = slime)
    if (try_material_combination(world, x, y)) return;
    generic_slow_liquid_update(world, x, y, 3);  // Very slow
}

void update_mud(World& world, int32_t x, int32_t y) {
    // Mud dries out slowly if not touching water
    bool has_water = false;
    for (int dy = -1; dy <= 1; dy++) {
        for (int dx = -1; dx <= 1; dx++) {
            int nx = x + dx, ny = y + dy;
            if (world.in_bounds(nx, ny) && world.get_material(nx, ny) == MaterialID::Water) {
                has_water = true;
                break;
            }
        }
        if (has_water) break;
    }
    if (!has_water && (world.random_int() & 255) == 0) {
        world.set_material(x, y, MaterialID::Dirt);
        return;
    }
    generic_slow_liquid_update(world, x, y, 1);
}

void update_blood(World& world, int32_t x, int32_t y) {
    // Check for combinations (blood + water = diluted)
    if (try_material_combination(world, x, y)) return;
    // Blood behaves like water but slower
    generic_slow_liquid_update(world, x, y, 0);
}

void update_poison(World& world, int32_t x, int32_t y) {
    // Poison kills organic materials
    for (int dy = -1; dy <= 1; dy++) {
        for (int dx = -1; dx <= 1; dx++) {
            int nx = x + dx, ny = y + dy;
            if (world.in_bounds(nx, ny)) {
                MaterialID m = world.get_material(nx, ny);
                if ((m == MaterialID::Grass || m == MaterialID::Wood ||
                     m == MaterialID::Leaf || m == MaterialID::Moss ||
                     m == MaterialID::Vine || m == MaterialID::Flower) &&
                    (world.random_int() & 7) == 0) {
                    world.set_material(nx, ny, MaterialID::Empty);
                }
            }
        }
    }
    update_water(world, x, y);  // Flows like water
}

void update_slime(World& world, int32_t x, int32_t y) {
    generic_slow_liquid_update(world, x, y, 1);  // Thick and slow
}

void update_milk(World& world, int32_t x, int32_t y) {
    // Check for combinations (milk + acid = slime)
    if (try_material_combination(world, x, y)) return;
    update_water(world, x, y);  // Flows like water
}

void update_alcohol(World& world, int32_t x, int32_t y) {
    // Check for combinations (alcohol + fire = more fire)
    if (try_material_combination(world, x, y)) return;
    // Alcohol is flammable
    for (int dy = -1; dy <= 1; dy++) {
        for (int dx = -1; dx <= 1; dx++) {
            int nx = x + dx, ny = y + dy;
            if (world.in_bounds(nx, ny)) {
                MaterialID m = world.get_material(nx, ny);
                if (m == MaterialID::Fire || m == MaterialID::Lava || m == MaterialID::Spark) {
                    world.set_material(x, y, MaterialID::Fire);
                    world.get_cell(x, y).set_lifetime(25);
                    return;
                }
            }
        }
    }
    update_water(world, x, y);
}

void update_mercury(World& world, int32_t x, int32_t y) {
    // Check for combinations (mercury + acid = toxic gas)
    if (try_material_combination(world, x, y)) return;
    // Mercury is very dense - sinks through most things
    // Try to fall faster
    Cell& cell = world.get_cell(x, y);
    cell.add_velocity(4);  // Falls fast
    cell.clamp_velocity(0, 25);

    // Mercury can displace other liquids (sink through them)
    if (world.in_bounds(x, y + 1)) {
        MaterialID below = world.get_material(x, y + 1);
        if (below == MaterialID::Water || below == MaterialID::Oil ||
            below == MaterialID::Acid || below == MaterialID::Blood) {
            // Swap positions
            world.set_material(x, y, below);
            world.set_material(x, y + 1, MaterialID::Mercury);
            return;
        }
    }

    int best_y = y;
    for (int test_y = y + 1; test_y <= y + cell.velocity_y && test_y < 600; test_y++) {
        if (world.in_bounds(x, test_y) && world.can_move_to(x, y, x, test_y)) {
            best_y = test_y;
        } else {
            break;
        }
    }

    if (best_y > y) {
        int8_t vel = cell.velocity_y;
        if (world.try_move_cell(x, y, x, best_y)) {
            world.get_cell(x, best_y).velocity_y = vel;
            return;
        }
    }

    cell.reset_velocity();

    bool try_left = (world.random_int() & 1) == 0;
    if (try_left) {
        if (world.try_move_cell(x, y, x - 1, y + 1)) return;
        if (world.try_move_cell(x, y, x + 1, y + 1)) return;
        if (world.try_move_cell(x, y, x - 1, y)) return;
        if (world.try_move_cell(x, y, x + 1, y)) return;
    } else {
        if (world.try_move_cell(x, y, x + 1, y + 1)) return;
        if (world.try_move_cell(x, y, x - 1, y + 1)) return;
        if (world.try_move_cell(x, y, x + 1, y)) return;
        if (world.try_move_cell(x, y, x - 1, y)) return;
    }
}

void update_petrol(World& world, int32_t x, int32_t y) {
    // Check for combinations (petrol + fire = big fire)
    if (try_material_combination(world, x, y)) return;
    // Petrol is very flammable, like oil but more reactive
    for (int dy = -1; dy <= 1; dy++) {
        for (int dx = -1; dx <= 1; dx++) {
            int nx = x + dx, ny = y + dy;
            if (world.in_bounds(nx, ny)) {
                MaterialID m = world.get_material(nx, ny);
                if (m == MaterialID::Fire || m == MaterialID::Lava ||
                    m == MaterialID::Spark || m == MaterialID::Lightning) {
                    // Explosive ignition
                    world.set_material(x, y, MaterialID::Fire);
                    world.get_cell(x, y).set_lifetime(40);
                    world.get_cell(x, y).velocity_y = -8;
                    return;
                }
            }
        }
    }
    update_oil(world, x, y);  // Flows like oil
}

void update_glue(World& world, int32_t x, int32_t y) {
    // Glue is very slow and sticky - eventually solidifies
    Cell& cell = world.get_cell(x, y);
    if (cell.get_lifetime() == 0) {
        cell.set_lifetime(63);  // Start solidification timer
    }
    cell.decrement_lifetime();
    if (cell.get_lifetime() < 10) {
        // Solidified - no longer moves
        return;
    }
    generic_slow_liquid_update(world, x, y, 7);  // Extremely slow
}

// ============================================================================
// GASES (30-39)
// ============================================================================

void update_toxic_gas(World& world, int32_t x, int32_t y) {
    Cell& cell = world.get_cell(x, y);
    if (cell.get_lifetime() == 0) {
        cell.set_lifetime(60);
    }
    generic_gas_update(world, x, y, -1, -12, true);
}

void update_hydrogen(World& world, int32_t x, int32_t y) {
    // Hydrogen rises fast and is explosive
    for (int dy = -1; dy <= 1; dy++) {
        for (int dx = -1; dx <= 1; dx++) {
            int nx = x + dx, ny = y + dy;
            if (world.in_bounds(nx, ny)) {
                MaterialID m = world.get_material(nx, ny);
                if (m == MaterialID::Fire || m == MaterialID::Spark ||
                    m == MaterialID::Lava || m == MaterialID::Lightning) {
                    // Explosive! Creates fire in radius
                    for (int ey = -2; ey <= 2; ey++) {
                        for (int ex = -2; ex <= 2; ex++) {
                            int fx = x + ex, fy = y + ey;
                            if (world.in_bounds(fx, fy)) {
                                MaterialID fm = world.get_material(fx, fy);
                                if (fm == MaterialID::Empty || fm == MaterialID::Hydrogen) {
                                    world.set_material(fx, fy, MaterialID::Fire);
                                    world.get_cell(fx, fy).set_lifetime(15);
                                }
                            }
                        }
                    }
                    return;
                }
            }
        }
    }
    generic_gas_update(world, x, y, -3, -20, false);  // Rises very fast
}

void update_helium(World& world, int32_t x, int32_t y) {
    generic_gas_update(world, x, y, -3, -25, false);  // Rises even faster than hydrogen
}

void update_methane(World& world, int32_t x, int32_t y) {
    // Methane is flammable like hydrogen
    for (int dy = -1; dy <= 1; dy++) {
        for (int dx = -1; dx <= 1; dx++) {
            int nx = x + dx, ny = y + dy;
            if (world.in_bounds(nx, ny)) {
                MaterialID m = world.get_material(nx, ny);
                if (m == MaterialID::Fire || m == MaterialID::Spark || m == MaterialID::Lava) {
                    world.set_material(x, y, MaterialID::Fire);
                    world.get_cell(x, y).set_lifetime(20);
                    return;
                }
            }
        }
    }
    generic_gas_update(world, x, y, -2, -15, false);
}

void update_spark(World& world, int32_t x, int32_t y) {
    Cell& cell = world.get_cell(x, y);
    if (cell.get_lifetime() == 0) {
        cell.set_lifetime(10);  // Short lived
    }
    cell.decrement_lifetime();
    if (cell.get_lifetime() == 0) {
        world.set_material(x, y, MaterialID::Empty);
        return;
    }

    // Sparks move erratically
    uint32_t rand = world.random_int();
    int dx = ((rand & 3) - 1);  // -1, 0, 0, 1
    int dy = ((rand >> 2) & 3) - 2;  // -2 to 1 (bias upward)
    int nx = x + dx, ny = y + dy;
    if (world.in_bounds(nx, ny) && world.get_material(nx, ny) == MaterialID::Empty) {
        world.try_move_cell(x, y, nx, ny);
    }
}

void update_plasma(World& world, int32_t x, int32_t y) {
    Cell& cell = world.get_cell(x, y);
    if (cell.get_lifetime() == 0) {
        cell.set_lifetime(25);
    }
    cell.decrement_lifetime();
    if (cell.get_lifetime() == 0) {
        world.set_material(x, y, MaterialID::Empty);
        return;
    }

    // Plasma destroys most things and moves erratically
    for (int dy = -1; dy <= 1; dy++) {
        for (int dx = -1; dx <= 1; dx++) {
            int nx = x + dx, ny = y + dy;
            if (world.in_bounds(nx, ny)) {
                MaterialID m = world.get_material(nx, ny);
                if (m != MaterialID::Empty && m != MaterialID::Plasma &&
                    m != MaterialID::Obsidian && m != MaterialID::Diamond &&
                    m != MaterialID::Void && (world.random_int() & 7) == 0) {
                    if (m == MaterialID::Water || m == MaterialID::Ice) {
                        world.set_material(nx, ny, MaterialID::Steam);
                    } else {
                        world.set_material(nx, ny, MaterialID::Fire);
                        world.get_cell(nx, ny).set_lifetime(10);
                    }
                }
            }
        }
    }

    generic_gas_update(world, x, y, -2, -15, false);
}

void update_dust(World& world, int32_t x, int32_t y) {
    Cell& cell = world.get_cell(x, y);
    if (cell.get_lifetime() == 0) {
        cell.set_lifetime(50);
    }
    generic_gas_update(world, x, y, -1, -8, true);
}

void update_spore(World& world, int32_t x, int32_t y) {
    Cell& cell = world.get_cell(x, y);
    if (cell.get_lifetime() == 0) {
        cell.set_lifetime(45);
    }

    // Spores can land and grow into fungus
    if (world.in_bounds(x, y + 1)) {
        MaterialID below = world.get_material(x, y + 1);
        if (below == MaterialID::Dirt || below == MaterialID::Grass ||
            below == MaterialID::Wood) {
            if ((world.random_int() & 31) == 0) {
                world.set_material(x, y, MaterialID::Fungus);
                return;
            }
        }
    }

    generic_gas_update(world, x, y, -1, -10, true);
}

void update_confetti(World& world, int32_t x, int32_t y) {
    Cell& cell = world.get_cell(x, y);
    if (cell.get_lifetime() == 0) {
        cell.set_lifetime(60);
    }

    // Confetti flutters down slowly with lateral movement
    cell.decrement_lifetime();
    if (cell.get_lifetime() == 0) {
        world.set_material(x, y, MaterialID::Empty);
        return;
    }

    uint32_t rand = world.random_int();
    int dx = (rand & 3) - 1;  // -1, 0, 0, 1 (slight drift)

    // Fall slowly
    if ((rand & 7) < 6) {
        if (world.in_bounds(x + dx, y + 1) &&
            world.get_material(x + dx, y + 1) == MaterialID::Empty) {
            world.try_move_cell(x, y, x + dx, y + 1);
        } else if (world.in_bounds(x, y + 1) &&
                   world.get_material(x, y + 1) == MaterialID::Empty) {
            world.try_move_cell(x, y, x, y + 1);
        }
    }
}

// ============================================================================
// SOLIDS (40-49) - Most solids don't move
// ============================================================================

void update_metal(World& world, int32_t x, int32_t y) {
    // Metal conducts electricity (spark/lightning)
    for (int dy = -1; dy <= 1; dy++) {
        for (int dx = -1; dx <= 1; dx++) {
            int nx = x + dx, ny = y + dy;
            if (world.in_bounds(nx, ny)) {
                MaterialID m = world.get_material(nx, ny);
                if (m == MaterialID::Lightning) {
                    // Conduct to other metals
                    for (int cy = -1; cy <= 1; cy++) {
                        for (int cx = -1; cx <= 1; cx++) {
                            int mx = x + cx, my = y + cy;
                            if (world.in_bounds(mx, my) &&
                                world.get_material(mx, my) == MaterialID::Metal &&
                                (world.random_int() & 3) == 0) {
                                // Create spark nearby
                                for (int sy = -1; sy <= 1; sy++) {
                                    for (int sx = -1; sx <= 1; sx++) {
                                        int spx = mx + sx, spy = my + sy;
                                        if (world.in_bounds(spx, spy) &&
                                            world.get_material(spx, spy) == MaterialID::Empty) {
                                            world.set_material(spx, spy, MaterialID::Spark);
                                            world.get_cell(spx, spy).set_lifetime(5);
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    // Static solid
}

void update_gold(World& world, int32_t x, int32_t y) {
    (void)world; (void)x; (void)y;  // Static solid
}

void update_ice(World& world, int32_t x, int32_t y) {
    // Ice melts near heat
    for (int dy = -1; dy <= 1; dy++) {
        for (int dx = -1; dx <= 1; dx++) {
            int nx = x + dx, ny = y + dy;
            if (world.in_bounds(nx, ny)) {
                MaterialID m = world.get_material(nx, ny);
                if (m == MaterialID::Fire || m == MaterialID::Lava ||
                    m == MaterialID::Dragon_Fire || m == MaterialID::Plasma) {
                    world.set_material(x, y, MaterialID::Water);
                    return;
                }
            }
        }
    }
}

void update_glass(World& world, int32_t x, int32_t y) {
    (void)world; (void)x; (void)y;  // Static solid
}

void update_brick(World& world, int32_t x, int32_t y) {
    (void)world; (void)x; (void)y;  // Static solid
}

void update_obsidian(World& world, int32_t x, int32_t y) {
    (void)world; (void)x; (void)y;  // Indestructible solid
}

void update_diamond(World& world, int32_t x, int32_t y) {
    (void)world; (void)x; (void)y;  // Indestructible solid
}

void update_copper(World& world, int32_t x, int32_t y) {
    // Copper oxidizes (turns to rust) when touching water
    for (int dy = -1; dy <= 1; dy++) {
        for (int dx = -1; dx <= 1; dx++) {
            int nx = x + dx, ny = y + dy;
            if (world.in_bounds(nx, ny) && world.get_material(nx, ny) == MaterialID::Water) {
                if ((world.random_int() & 255) == 0) {
                    world.set_material(x, y, MaterialID::Rust);
                    return;
                }
            }
        }
    }
}

void update_rubber(World& world, int32_t x, int32_t y) {
    // Rubber burns
    for (int dy = -1; dy <= 1; dy++) {
        for (int dx = -1; dx <= 1; dx++) {
            int nx = x + dx, ny = y + dy;
            if (world.in_bounds(nx, ny)) {
                MaterialID m = world.get_material(nx, ny);
                if ((m == MaterialID::Fire || m == MaterialID::Lava) &&
                    (world.random_int() & 15) == 0) {
                    world.set_material(x, y, MaterialID::Smoke);
                    world.get_cell(x, y).set_lifetime(30);
                    return;
                }
            }
        }
    }
}

// ============================================================================
// ORGANIC (50-59)
// ============================================================================

void update_leaf(World& world, int32_t x, int32_t y) {
    // Check for combinations (leaf + water = algae)
    if (try_material_combination(world, x, y)) return;

    // Leaves flutter down slowly
    Cell& cell = world.get_cell(x, y);
    uint32_t rand = world.random_int();

    // Burns easily
    for (int dy = -1; dy <= 1; dy++) {
        for (int dx = -1; dx <= 1; dx++) {
            int nx = x + dx, ny = y + dy;
            if (world.in_bounds(nx, ny)) {
                MaterialID m = world.get_material(nx, ny);
                if (m == MaterialID::Fire || m == MaterialID::Lava) {
                    world.set_material(x, y, MaterialID::Fire);
                    world.get_cell(x, y).set_lifetime(10);
                    return;
                }
            }
        }
    }

    // Flutter down slowly
    if ((rand & 3) == 0) {
        int dx = (rand & 4) ? 1 : -1;
        if (world.in_bounds(x + dx, y + 1) &&
            world.get_material(x + dx, y + 1) == MaterialID::Empty) {
            world.try_move_cell(x, y, x + dx, y + 1);
        } else if (world.in_bounds(x, y + 1) &&
                   world.get_material(x, y + 1) == MaterialID::Empty) {
            world.try_move_cell(x, y, x, y + 1);
        }
    }
    (void)cell;
}

void update_moss(World& world, int32_t x, int32_t y) {
    // Check for combinations
    if (try_material_combination(world, x, y)) return;

    // Moss spreads slowly on stone/brick/wood
    if ((world.random_int() & 127) == 0) {
        uint32_t dir = world.random_int() & 3;
        int dx = (dir == 0) ? -1 : (dir == 1) ? 1 : 0;
        int dy = (dir == 2) ? -1 : (dir == 3) ? 1 : 0;
        int nx = x + dx, ny = y + dy;
        if (world.in_bounds(nx, ny)) {
            MaterialID m = world.get_material(nx, ny);
            if (m == MaterialID::Stone || m == MaterialID::Brick || m == MaterialID::Wood) {
                world.set_material(nx, ny, MaterialID::Moss);
            }
        }
    }
    // Burns
    for (int dy = -1; dy <= 1; dy++) {
        for (int dx = -1; dx <= 1; dx++) {
            int nx = x + dx, ny = y + dy;
            if (world.in_bounds(nx, ny)) {
                if (world.get_material(nx, ny) == MaterialID::Fire) {
                    world.set_material(x, y, MaterialID::Fire);
                    world.get_cell(x, y).set_lifetime(10);
                    return;
                }
            }
        }
    }
}

void update_vine(World& world, int32_t x, int32_t y) {
    // Check for combinations
    if (try_material_combination(world, x, y)) return;

    // Vines grow downward slowly
    if ((world.random_int() & 63) == 0) {
        if (world.in_bounds(x, y + 1) && world.get_material(x, y + 1) == MaterialID::Empty) {
            world.set_material(x, y + 1, MaterialID::Vine);
        }
    }
    // Burns
    for (int dy = -1; dy <= 1; dy++) {
        for (int dx = -1; dx <= 1; dx++) {
            int nx = x + dx, ny = y + dy;
            if (world.in_bounds(nx, ny)) {
                if (world.get_material(nx, ny) == MaterialID::Fire) {
                    world.set_material(x, y, MaterialID::Fire);
                    world.get_cell(x, y).set_lifetime(12);
                    return;
                }
            }
        }
    }
}

void update_fungus(World& world, int32_t x, int32_t y) {
    // Check for combinations
    if (try_material_combination(world, x, y)) return;

    // Fungus spreads slowly and releases spores
    if ((world.random_int() & 255) == 0) {
        // Spread
        uint32_t dir = world.random_int() & 3;
        int dx = (dir == 0) ? -1 : (dir == 1) ? 1 : 0;
        int dy = (dir == 2) ? -1 : (dir == 3) ? 1 : 0;
        int nx = x + dx, ny = y + dy;
        if (world.in_bounds(nx, ny)) {
            MaterialID m = world.get_material(nx, ny);
            if (m == MaterialID::Wood || m == MaterialID::Dirt ||
                m == MaterialID::Grass || m == MaterialID::Flesh) {
                world.set_material(nx, ny, MaterialID::Fungus);
            }
        }
    }
    // Release spores
    if ((world.random_int() & 511) == 0) {
        if (world.in_bounds(x, y - 1) && world.get_material(x, y - 1) == MaterialID::Empty) {
            world.set_material(x, y - 1, MaterialID::Spore);
            world.get_cell(x, y - 1).set_lifetime(40);
        }
    }
}

void update_seed(World& world, int32_t x, int32_t y) {
    // Check for combinations (seed + water = flower, seed + dirt = grass)
    if (try_material_combination(world, x, y)) return;
    // Seeds fall and can grow into plants
    if (world.in_bounds(x, y + 1)) {
        MaterialID below = world.get_material(x, y + 1);
        if (below == MaterialID::Dirt || below == MaterialID::Grass) {
            // Grow into a flower or vine
            if ((world.random_int() & 127) == 0) {
                if ((world.random_int() & 1) == 0) {
                    world.set_material(x, y, MaterialID::Flower);
                } else {
                    world.set_material(x, y, MaterialID::Vine);
                }
                return;
            }
        } else if (below == MaterialID::Empty || below == MaterialID::Water) {
            world.try_move_cell(x, y, x, y + 1);
        }
    }
}

void update_flower(World& world, int32_t x, int32_t y) {
    // Check for combinations
    if (try_material_combination(world, x, y)) return;

    // Flowers are static but burn and can release seeds
    for (int dy = -1; dy <= 1; dy++) {
        for (int dx = -1; dx <= 1; dx++) {
            int nx = x + dx, ny = y + dy;
            if (world.in_bounds(nx, ny)) {
                if (world.get_material(nx, ny) == MaterialID::Fire) {
                    world.set_material(x, y, MaterialID::Fire);
                    world.get_cell(x, y).set_lifetime(8);
                    return;
                }
            }
        }
    }
}

void update_algae(World& world, int32_t x, int32_t y) {
    // Check for combinations
    if (try_material_combination(world, x, y)) return;

    // Algae floats in water and slowly spreads
    bool in_water = false;
    for (int dy = -1; dy <= 1; dy++) {
        for (int dx = -1; dx <= 1; dx++) {
            int nx = x + dx, ny = y + dy;
            if (world.in_bounds(nx, ny) && world.get_material(nx, ny) == MaterialID::Water) {
                in_water = true;
                // Spread
                if ((world.random_int() & 255) == 0) {
                    world.set_material(nx, ny, MaterialID::Algae);
                }
            }
        }
    }

    if (!in_water) {
        // Fall if not in water
        if (world.try_move_cell(x, y, x, y + 1)) return;
    }
}

void update_coral(World& world, int32_t x, int32_t y) {
    // Check for combinations
    if (try_material_combination(world, x, y)) return;

    // Coral grows slowly underwater
    bool underwater = false;
    for (int dy = -1; dy <= 1; dy++) {
        for (int dx = -1; dx <= 1; dx++) {
            int nx = x + dx, ny = y + dy;
            if (world.in_bounds(nx, ny) && world.get_material(nx, ny) == MaterialID::Water) {
                underwater = true;
                break;
            }
        }
        if (underwater) break;
    }

    if (underwater && (world.random_int() & 511) == 0) {
        uint32_t dir = world.random_int() & 3;
        int dx = (dir == 0) ? -1 : (dir == 1) ? 1 : 0;
        int dy = (dir == 2) ? -1 : 0;  // Coral grows up and sideways
        int nx = x + dx, ny = y + dy;
        if (world.in_bounds(nx, ny) && world.get_material(nx, ny) == MaterialID::Water) {
            world.set_material(nx, ny, MaterialID::Coral);
        }
    }
}

void update_wax(World& world, int32_t x, int32_t y) {
    // Check for combinations
    if (try_material_combination(world, x, y)) return;

    // Wax melts near fire
    for (int dy = -1; dy <= 1; dy++) {
        for (int dx = -1; dx <= 1; dx++) {
            int nx = x + dx, ny = y + dy;
            if (world.in_bounds(nx, ny)) {
                MaterialID m = world.get_material(nx, ny);
                if (m == MaterialID::Fire || m == MaterialID::Lava) {
                    // Melt into a slow liquid
                    world.set_material(x, y, MaterialID::Oil);  // Melted wax acts like oil
                    return;
                }
            }
        }
    }
}

void update_flesh(World& world, int32_t x, int32_t y) {
    // Flesh burns and can be infected by fungus
    for (int dy = -1; dy <= 1; dy++) {
        for (int dx = -1; dx <= 1; dx++) {
            int nx = x + dx, ny = y + dy;
            if (world.in_bounds(nx, ny)) {
                MaterialID m = world.get_material(nx, ny);
                if (m == MaterialID::Fire || m == MaterialID::Lava) {
                    world.set_material(x, y, MaterialID::Fire);
                    world.get_cell(x, y).set_lifetime(15);
                    return;
                }
                if (m == MaterialID::Acid && (world.random_int() & 7) == 0) {
                    world.set_material(x, y, MaterialID::Empty);
                    return;
                }
            }
        }
    }
}

// ============================================================================
// SPECIAL (60-69)
// ============================================================================

void update_clone(World& world, int32_t x, int32_t y) {
    // Clone copies the first non-clone material it touches
    for (int dy = -1; dy <= 1; dy++) {
        for (int dx = -1; dx <= 1; dx++) {
            if (dx == 0 && dy == 0) continue;
            int nx = x + dx, ny = y + dy;
            if (world.in_bounds(nx, ny)) {
                MaterialID m = world.get_material(nx, ny);
                if (m != MaterialID::Empty && m != MaterialID::Clone && m != MaterialID::Void) {
                    // Spawn a copy of the material in an empty adjacent cell
                    for (int sy = -1; sy <= 1; sy++) {
                        for (int sx = -1; sx <= 1; sx++) {
                            int spx = x + sx, spy = y + sy;
                            if (world.in_bounds(spx, spy) &&
                                world.get_material(spx, spy) == MaterialID::Empty &&
                                (world.random_int() & 7) == 0) {
                                world.set_material(spx, spy, m);
                                return;
                            }
                        }
                    }
                    return;
                }
            }
        }
    }
}

void update_void(World& world, int32_t x, int32_t y) {
    // Void destroys everything it touches
    for (int dy = -1; dy <= 1; dy++) {
        for (int dx = -1; dx <= 1; dx++) {
            if (dx == 0 && dy == 0) continue;
            int nx = x + dx, ny = y + dy;
            if (world.in_bounds(nx, ny)) {
                MaterialID m = world.get_material(nx, ny);
                if (m != MaterialID::Empty && m != MaterialID::Void) {
                    world.set_material(nx, ny, MaterialID::Empty);
                }
            }
        }
    }
}

void update_fuse(World& world, int32_t x, int32_t y) {
    Cell& cell = world.get_cell(x, y);

    // Fuse burns when touched by fire
    if (cell.get_lifetime() > 0) {
        cell.decrement_lifetime();
        if (cell.get_lifetime() == 0) {
            world.set_material(x, y, MaterialID::Fire);
            world.get_cell(x, y).set_lifetime(10);
            // Ignite adjacent fuse
            for (int dy = -1; dy <= 1; dy++) {
                for (int dx = -1; dx <= 1; dx++) {
                    int nx = x + dx, ny = y + dy;
                    if (world.in_bounds(nx, ny) &&
                        world.get_material(nx, ny) == MaterialID::Fuse &&
                        world.get_cell(nx, ny).get_lifetime() == 0) {
                        world.get_cell(nx, ny).set_lifetime(10 + (world.random_int() & 7));
                    }
                }
            }
            return;
        }
    } else {
        // Check for ignition
        for (int dy = -1; dy <= 1; dy++) {
            for (int dx = -1; dx <= 1; dx++) {
                int nx = x + dx, ny = y + dy;
                if (world.in_bounds(nx, ny)) {
                    MaterialID m = world.get_material(nx, ny);
                    if (m == MaterialID::Fire || m == MaterialID::Spark ||
                        m == MaterialID::Lava) {
                        cell.set_lifetime(10);
                        return;
                    }
                }
            }
        }
    }
}

void update_tnt(World& world, int32_t x, int32_t y) {
    // TNT explodes when touched by fire/fuse
    for (int dy = -1; dy <= 1; dy++) {
        for (int dx = -1; dx <= 1; dx++) {
            int nx = x + dx, ny = y + dy;
            if (world.in_bounds(nx, ny)) {
                MaterialID m = world.get_material(nx, ny);
                if (m == MaterialID::Fire || m == MaterialID::Spark ||
                    m == MaterialID::Lava || m == MaterialID::Lightning) {
                    // EXPLOSION!
                    int radius = 8;
                    for (int ey = -radius; ey <= radius; ey++) {
                        for (int ex = -radius; ex <= radius; ex++) {
                            if (ex * ex + ey * ey <= radius * radius) {
                                int fx = x + ex, fy = y + ey;
                                if (world.in_bounds(fx, fy)) {
                                    MaterialID fm = world.get_material(fx, fy);
                                    if (fm != MaterialID::Obsidian && fm != MaterialID::Diamond &&
                                        fm != MaterialID::Void) {
                                        if ((ex * ex + ey * ey) > radius * radius / 2) {
                                            // Outer ring becomes fire
                                            if (fm != MaterialID::Stone) {
                                                world.set_material(fx, fy, MaterialID::Fire);
                                                world.get_cell(fx, fy).set_lifetime(15);
                                            }
                                        } else {
                                            // Inner area destroyed
                                            world.set_material(fx, fy, MaterialID::Empty);
                                        }
                                    }
                                }
                            }
                        }
                    }
                    return;
                }
            }
        }
    }
}

void update_c4(World& world, int32_t x, int32_t y) {
    // C4 is more powerful than TNT
    for (int dy = -1; dy <= 1; dy++) {
        for (int dx = -1; dx <= 1; dx++) {
            int nx = x + dx, ny = y + dy;
            if (world.in_bounds(nx, ny)) {
                MaterialID m = world.get_material(nx, ny);
                if (m == MaterialID::Fire || m == MaterialID::Spark ||
                    m == MaterialID::Lava || m == MaterialID::Lightning) {
                    // BIGGER EXPLOSION!
                    int radius = 15;
                    for (int ey = -radius; ey <= radius; ey++) {
                        for (int ex = -radius; ex <= radius; ex++) {
                            if (ex * ex + ey * ey <= radius * radius) {
                                int fx = x + ex, fy = y + ey;
                                if (world.in_bounds(fx, fy)) {
                                    MaterialID fm = world.get_material(fx, fy);
                                    if (fm != MaterialID::Obsidian && fm != MaterialID::Diamond &&
                                        fm != MaterialID::Void) {
                                        if ((ex * ex + ey * ey) > radius * radius * 3 / 4) {
                                            if (fm != MaterialID::Stone) {
                                                world.set_material(fx, fy, MaterialID::Fire);
                                                world.get_cell(fx, fy).set_lifetime(20);
                                            }
                                        } else {
                                            world.set_material(fx, fy, MaterialID::Empty);
                                        }
                                    }
                                }
                            }
                        }
                    }
                    return;
                }
            }
        }
    }
}

void update_firework(World& world, int32_t x, int32_t y) {
    Cell& cell = world.get_cell(x, y);

    // Fireworks launch when ignited
    if (cell.get_lifetime() > 0) {
        cell.decrement_lifetime();
        // Rising phase
        cell.add_velocity(-3);
        cell.clamp_velocity(-20, 0);

        int target_y = y + cell.velocity_y;
        if (world.in_bounds(x, target_y) &&
            world.get_material(x, target_y) == MaterialID::Empty) {
            world.try_move_cell(x, y, x, target_y);
        } else {
            // Explode on impact or timeout
            cell.set_lifetime(0);
        }

        if (cell.get_lifetime() == 0) {
            // EXPLODE into confetti!
            int radius = 6;
            for (int ey = -radius; ey <= radius; ey++) {
                for (int ex = -radius; ex <= radius; ex++) {
                    if (ex * ex + ey * ey <= radius * radius) {
                        int fx = x + ex, fy = y + ey;
                        if (world.in_bounds(fx, fy) &&
                            world.get_material(fx, fy) == MaterialID::Empty &&
                            (world.random_int() & 3) == 0) {
                            world.set_material(fx, fy, MaterialID::Confetti);
                            world.get_cell(fx, fy).set_lifetime(50 + (world.random_int() & 31));
                        }
                    }
                }
            }
            world.set_material(x, y, MaterialID::Fire);
            world.get_cell(x, y).set_lifetime(15);
        }
    } else {
        // Check for ignition
        for (int dy = -1; dy <= 1; dy++) {
            for (int dx = -1; dx <= 1; dx++) {
                int nx = x + dx, ny = y + dy;
                if (world.in_bounds(nx, ny)) {
                    MaterialID m = world.get_material(nx, ny);
                    if (m == MaterialID::Fire || m == MaterialID::Spark ||
                        m == MaterialID::Lava || m == MaterialID::Fuse) {
                        cell.set_lifetime(40);  // Fuse time before launch
                        cell.velocity_y = 0;
                        return;
                    }
                }
            }
        }
    }
}

void update_lightning(World& world, int32_t x, int32_t y) {
    Cell& cell = world.get_cell(x, y);
    if (cell.get_lifetime() == 0) {
        cell.set_lifetime(5);  // Very short lived
    }
    cell.decrement_lifetime();
    if (cell.get_lifetime() == 0) {
        world.set_material(x, y, MaterialID::Empty);
        return;
    }

    // Lightning moves down erratically and destroys things
    for (int dy = -1; dy <= 1; dy++) {
        for (int dx = -1; dx <= 1; dx++) {
            int nx = x + dx, ny = y + dy;
            if (world.in_bounds(nx, ny)) {
                MaterialID m = world.get_material(nx, ny);
                if (m != MaterialID::Empty && m != MaterialID::Lightning &&
                    m != MaterialID::Metal && m != MaterialID::Stone &&
                    m != MaterialID::Obsidian && m != MaterialID::Diamond &&
                    (world.random_int() & 3) == 0) {
                    world.set_material(nx, ny, MaterialID::Fire);
                    world.get_cell(nx, ny).set_lifetime(10);
                }
            }
        }
    }

    // Move down with some lateral variation
    uint32_t rand = world.random_int();
    int dx = (rand & 3) - 1;
    int dy = 1 + (rand & 1);
    int nx = x + dx, ny = y + dy;
    if (world.in_bounds(nx, ny) &&
        (world.get_material(nx, ny) == MaterialID::Empty ||
         world.get_material(nx, ny) == MaterialID::Water)) {
        world.try_move_cell(x, y, nx, ny);
    }
}

void update_portal_in(World& world, int32_t x, int32_t y) {
    // Portal_In teleports materials touching it to Portal_Out
    // First, find a portal out
    static int portal_out_x = -1, portal_out_y = -1;

    // Scan for portal out (cached for performance)
    if (portal_out_x < 0 || world.get_material(portal_out_x, portal_out_y) != MaterialID::Portal_Out) {
        portal_out_x = -1;
        portal_out_y = -1;
        for (int sy = 0; sy < WORLD_HEIGHT && portal_out_x < 0; sy++) {
            for (int sx = 0; sx < WORLD_WIDTH; sx++) {
                if (world.get_material(sx, sy) == MaterialID::Portal_Out) {
                    portal_out_x = sx;
                    portal_out_y = sy;
                    break;
                }
            }
        }
    }

    if (portal_out_x < 0) return;  // No portal out

    // Teleport materials
    for (int dy = -1; dy <= 1; dy++) {
        for (int dx = -1; dx <= 1; dx++) {
            if (dx == 0 && dy == 0) continue;
            int nx = x + dx, ny = y + dy;
            if (world.in_bounds(nx, ny)) {
                MaterialID m = world.get_material(nx, ny);
                if (m != MaterialID::Empty && m != MaterialID::Portal_In &&
                    m != MaterialID::Portal_Out && m != MaterialID::Stone) {
                    // Find empty spot near portal out
                    for (int oy = -2; oy <= 2; oy++) {
                        for (int ox = -2; ox <= 2; ox++) {
                            int px = portal_out_x + ox, py = portal_out_y + oy;
                            if (world.in_bounds(px, py) &&
                                world.get_material(px, py) == MaterialID::Empty) {
                                // Teleport!
                                Cell& src_cell = world.get_cell(nx, ny);
                                world.set_material(px, py, m);
                                Cell& dst_cell = world.get_cell(px, py);
                                dst_cell.flags = src_cell.flags;
                                dst_cell.velocity_y = src_cell.velocity_y;
                                world.set_material(nx, ny, MaterialID::Empty);
                                return;
                            }
                        }
                    }
                }
            }
        }
    }
}

void update_portal_out(World& world, int32_t x, int32_t y) {
    (void)world; (void)x; (void)y;  // Portal out is passive
}

// ============================================================================
// FANTASY (70-79)
// ============================================================================

void update_magic(World& world, int32_t x, int32_t y) {
    // Check for combinations (magic + sand = gold, magic + coal = diamond, etc.)
    if (try_material_combination(world, x, y)) return;

    Cell& cell = world.get_cell(x, y);
    if (cell.get_lifetime() == 0) {
        cell.set_lifetime(40);
    }

    // Magic transforms random materials
    if ((world.random_int() & 31) == 0) {
        uint32_t dir = world.random_int() & 3;
        int dx = (dir == 0) ? -1 : (dir == 1) ? 1 : 0;
        int dy = (dir == 2) ? -1 : (dir == 3) ? 1 : 0;
        int nx = x + dx, ny = y + dy;
        if (world.in_bounds(nx, ny)) {
            MaterialID m = world.get_material(nx, ny);
            if (m != MaterialID::Empty && m != MaterialID::Magic &&
                m != MaterialID::Void && m != MaterialID::Diamond) {
                // Transform into something magical
                uint32_t transform = world.random_int() & 7;
                switch (transform) {
                    case 0: world.set_material(nx, ny, MaterialID::Gold); break;
                    case 1: world.set_material(nx, ny, MaterialID::Crystal); break;
                    case 2: world.set_material(nx, ny, MaterialID::Fairy_Dust); break;
                    case 3: world.set_material(nx, ny, MaterialID::Stardust); break;
                    default: break;  // Sometimes nothing happens
                }
            }
        }
    }

    generic_gas_update(world, x, y, -1, -12, true);
}

void update_crystal(World& world, int32_t x, int32_t y) {
    // Check for combinations (crystal + magic = diamond)
    if (try_material_combination(world, x, y)) return;
    // Crystal grows slowly
    if ((world.random_int() & 511) == 0) {
        uint32_t dir = world.random_int() & 3;
        int dx = (dir == 0) ? -1 : (dir == 1) ? 1 : 0;
        int dy = (dir == 2) ? -1 : (dir == 3) ? 1 : 0;
        int nx = x + dx, ny = y + dy;
        if (world.in_bounds(nx, ny) && world.get_material(nx, ny) == MaterialID::Empty) {
            world.set_material(nx, ny, MaterialID::Crystal);
        }
    }
}

void update_ectoplasm(World& world, int32_t x, int32_t y) {
    // Ectoplasm floats up slowly
    Cell& cell = world.get_cell(x, y);
    if (cell.get_lifetime() == 0) {
        cell.set_lifetime(60);
    }

    // Can phase through some materials
    if ((world.random_int() & 3) == 0) {
        int dy = -1;
        int dx = ((world.random_int() & 3) - 1);
        int nx = x + dx, ny = y + dy;
        if (world.in_bounds(nx, ny)) {
            MaterialID m = world.get_material(nx, ny);
            if (m == MaterialID::Empty || m == MaterialID::Water ||
                m == MaterialID::Steam) {
                world.try_move_cell(x, y, nx, ny);
            }
        }
    }

    generic_gas_update(world, x, y, -1, -8, true);
}

void update_antimatter(World& world, int32_t x, int32_t y) {
    // Antimatter destroys everything on contact (including itself)
    for (int dy = -1; dy <= 1; dy++) {
        for (int dx = -1; dx <= 1; dx++) {
            if (dx == 0 && dy == 0) continue;
            int nx = x + dx, ny = y + dy;
            if (world.in_bounds(nx, ny)) {
                MaterialID m = world.get_material(nx, ny);
                if (m != MaterialID::Empty && m != MaterialID::Antimatter &&
                    m != MaterialID::Void) {
                    // Annihilation! Both become energy
                    world.set_material(nx, ny, MaterialID::Plasma);
                    world.get_cell(nx, ny).set_lifetime(15);
                    world.set_material(x, y, MaterialID::Plasma);
                    world.get_cell(x, y).set_lifetime(15);
                    return;
                }
            }
        }
    }

    // Antimatter rises (negative density)
    generic_gas_update(world, x, y, -2, -15, false);
}

void update_fairy_dust(World& world, int32_t x, int32_t y) {
    // Check for combinations (fairy_dust + fire = magic)
    if (try_material_combination(world, x, y)) return;
    // Fairy dust floats around and heals people
    for (int dy = -1; dy <= 1; dy++) {
        for (int dx = -1; dx <= 1; dx++) {
            int nx = x + dx, ny = y + dy;
            if (world.in_bounds(nx, ny) &&
                world.get_material(nx, ny) == MaterialID::Person) {
                // Heal the person
                Cell& person = world.get_cell(nx, ny);
                if (person.get_health() < 100) {
                    person.set_health(std::min(100, person.get_health() + 20));
                }
                world.set_material(x, y, MaterialID::Empty);
                return;
            }
        }
    }

    // Flutter around
    uint32_t rand = world.random_int();
    int dx = (rand & 3) - 1;
    int dy = ((rand >> 2) & 3) - 2;  // Bias upward
    int nx = x + dx, ny = y + dy;
    if (world.in_bounds(nx, ny) && world.get_material(nx, ny) == MaterialID::Empty) {
        world.try_move_cell(x, y, nx, ny);
    }
}

void update_dragon_fire(World& world, int32_t x, int32_t y) {
    Cell& cell = world.get_cell(x, y);
    if (cell.get_lifetime() == 0) {
        cell.set_lifetime(35);
    }

    // Dragon fire is more destructive than regular fire
    for (int dy = -1; dy <= 1; dy++) {
        for (int dx = -1; dx <= 1; dx++) {
            int nx = x + dx, ny = y + dy;
            if (world.in_bounds(nx, ny)) {
                MaterialID m = world.get_material(nx, ny);
                // Burns even stone!
                if (m == MaterialID::Wood || m == MaterialID::Grass ||
                    m == MaterialID::Oil || m == MaterialID::Coal) {
                    world.set_material(nx, ny, MaterialID::Dragon_Fire);
                    world.get_cell(nx, ny).set_lifetime(25);
                } else if (m == MaterialID::Stone && (world.random_int() & 31) == 0) {
                    world.set_material(nx, ny, MaterialID::Lava);
                } else if (m == MaterialID::Water) {
                    world.set_material(x, y, MaterialID::Steam);
                    world.get_cell(x, y).velocity_y = -10;
                    world.set_material(nx, ny, MaterialID::Steam);
                    return;
                }
            }
        }
    }

    cell.decrement_lifetime();
    if (cell.get_lifetime() == 0) {
        if ((world.random_int() & 3) == 0) {
            world.set_material(x, y, MaterialID::Smoke);
            world.get_cell(x, y).set_lifetime(25);
        } else {
            world.set_material(x, y, MaterialID::Empty);
        }
        return;
    }

    generic_gas_update(world, x, y, -2, -15, false);
}

void update_frost(World& world, int32_t x, int32_t y) {
    // Check for combinations (frost + water = ice, frost + fire = steam)
    if (try_material_combination(world, x, y)) return;

    Cell& cell = world.get_cell(x, y);
    if (cell.get_lifetime() == 0) {
        cell.set_lifetime(40);
    }

    // Frost freezes water and extinguishes fire
    for (int dy = -1; dy <= 1; dy++) {
        for (int dx = -1; dx <= 1; dx++) {
            int nx = x + dx, ny = y + dy;
            if (world.in_bounds(nx, ny)) {
                MaterialID m = world.get_material(nx, ny);
                if (m == MaterialID::Water) {
                    world.set_material(nx, ny, MaterialID::Ice);
                } else if (m == MaterialID::Fire || m == MaterialID::Lava) {
                    if (m == MaterialID::Lava) {
                        world.set_material(nx, ny, MaterialID::Obsidian);
                    } else {
                        world.set_material(nx, ny, MaterialID::Empty);
                    }
                    world.set_material(x, y, MaterialID::Empty);
                    return;
                }
            }
        }
    }

    generic_gas_update(world, x, y, -1, -10, true);
}

void update_ember(World& world, int32_t x, int32_t y) {
    // Ember is a falling hot particle
    for (int dy = -1; dy <= 1; dy++) {
        for (int dx = -1; dx <= 1; dx++) {
            int nx = x + dx, ny = y + dy;
            if (world.in_bounds(nx, ny)) {
                MaterialID m = world.get_material(nx, ny);
                if (m == MaterialID::Wood || m == MaterialID::Grass ||
                    m == MaterialID::Coal || m == MaterialID::Oil) {
                    world.set_material(nx, ny, MaterialID::Fire);
                    world.get_cell(nx, ny).set_lifetime(20);
                    world.set_material(x, y, MaterialID::Empty);
                    return;
                }
            }
        }
    }

    Cell& cell = world.get_cell(x, y);
    if (cell.get_lifetime() == 0) {
        cell.set_lifetime(30);
    }
    cell.decrement_lifetime();
    if (cell.get_lifetime() == 0) {
        world.set_material(x, y, MaterialID::Ash);
        return;
    }

    generic_powder_update(world, x, y, 1, 8);  // Falls slowly
}

void update_stardust(World& world, int32_t x, int32_t y) {
    // Check for combinations (stardust + water = magic)
    if (try_material_combination(world, x, y)) return;
    // Stardust floats gently and sparkles
    uint32_t rand = world.random_int();
    int dx = (rand & 3) - 1;
    int dy = ((rand >> 2) & 1) ? 1 : -1;  // Gentle drift up and down

    int nx = x + dx, ny = y + dy;
    if (world.in_bounds(nx, ny) && world.get_material(nx, ny) == MaterialID::Empty) {
        world.try_move_cell(x, y, nx, ny);
    }
}

void update_void_dust(World& world, int32_t x, int32_t y) {
    // Check for combinations (void_dust + fire/spark = consumes it)
    if (try_material_combination(world, x, y)) return;
    // Void dust slowly erases things it touches
    for (int dy = -1; dy <= 1; dy++) {
        for (int dx = -1; dx <= 1; dx++) {
            int nx = x + dx, ny = y + dy;
            if (world.in_bounds(nx, ny)) {
                MaterialID m = world.get_material(nx, ny);
                if (m != MaterialID::Empty && m != MaterialID::Void_Dust &&
                    m != MaterialID::Void && m != MaterialID::Obsidian &&
                    m != MaterialID::Diamond && (world.random_int() & 15) == 0) {
                    world.set_material(nx, ny, MaterialID::Empty);
                }
            }
        }
    }

    generic_powder_update(world, x, y, 1, 6);
}

// Helper: Check if a location is safe for spawning a person
static bool is_safe_spawn_location(World& world, int32_t x, int32_t y) {
    // Must have solid ground below
    if (!world.in_bounds(x, y + 1)) return false;
    MaterialID below = world.get_material(x, y + 1);

    // Check for solid ground materials
    bool has_ground = (below == MaterialID::Stone || below == MaterialID::Wood ||
                       below == MaterialID::Grass || below == MaterialID::Sand ||
                       below == MaterialID::Brick || below == MaterialID::Dirt ||
                       below == MaterialID::Metal || below == MaterialID::Ice ||
                       below == MaterialID::Glass || below == MaterialID::Obsidian ||
                       below == MaterialID::Diamond || below == MaterialID::Copper ||
                       below == MaterialID::Gold || below == MaterialID::Crystal ||
                       below == MaterialID::Rubber || below == MaterialID::Coral);

    if (!has_ground) return false;

    // Check that current position is empty/passable
    MaterialID here = world.get_material(x, y);
    if (here != MaterialID::Life && here != MaterialID::Empty &&
        here != MaterialID::Steam && here != MaterialID::Smoke) {
        return false;
    }

    // Check for nearby dangers (fire, lava, acid)
    for (int dy = -2; dy <= 2; dy++) {
        for (int dx = -2; dx <= 2; dx++) {
            int nx = x + dx, ny = y + dy;
            if (world.in_bounds(nx, ny)) {
                MaterialID m = world.get_material(nx, ny);
                if (m == MaterialID::Fire || m == MaterialID::Lava ||
                    m == MaterialID::Acid || m == MaterialID::Dragon_Fire ||
                    m == MaterialID::Plasma) {
                    return false;  // Too dangerous!
                }
            }
        }
    }

    // Check for water (drowning risk)
    for (int dy = -1; dy <= 1; dy++) {
        for (int dx = -1; dx <= 1; dx++) {
            int nx = x + dx, ny = y + dy;
            if (world.in_bounds(nx, ny)) {
                MaterialID m = world.get_material(nx, ny);
                if (m == MaterialID::Water) {
                    return false;  // Would drown
                }
            }
        }
    }

    return true;
}

void update_life(World& world, int32_t x, int32_t y) {
    Cell& cell = world.get_cell(x, y);

    // Lifetime countdown (sparkle effect uses this)
    uint8_t life = cell.get_lifetime();
    if (life == 0) {
        cell.set_lifetime(63);  // Reset sparkle timer
    }
    cell.decrement_lifetime();

    // Check if we can spawn a person here (safe landing)
    if (is_safe_spawn_location(world, x, y)) {
        // Transform into a Person!
        world.set_material(x, y, MaterialID::Person);
        Cell& person = world.get_cell(x, y);
        // Random health between 80-127 gives unique personality per person
        // (health is used as personality seed, so each spawn is different)
        person.set_health(80 + (world.random_int() & 47));
        person.set_person_facing_right((world.random_int() & 1) != 0);
        person.set_lifetime(0);  // Reset frame counter

        // Create a small sparkle effect around spawn point
        for (int i = 0; i < 3; i++) {
            int sx = x + ((world.random_int() & 3) - 1);
            int sy = y - 1 - (world.random_int() & 1);
            if (world.in_bounds(sx, sy) && world.get_material(sx, sy) == MaterialID::Empty) {
                world.set_material(sx, sy, MaterialID::Spark);
                world.get_cell(sx, sy).set_lifetime(5 + (world.random_int() & 7));
            }
        }
        return;
    }

    // If in a dangerous situation, die with a puff
    MaterialID here = world.get_material(x, y);
    if (here != MaterialID::Life) {
        // Somehow displaced - just disappear
        return;
    }

    // Check for immediate danger and die if necessary
    for (int dy = -1; dy <= 1; dy++) {
        for (int dx = -1; dx <= 1; dx++) {
            if (dx == 0 && dy == 0) continue;
            int nx = x + dx, ny = y + dy;
            if (world.in_bounds(nx, ny)) {
                MaterialID m = world.get_material(nx, ny);
                if (m == MaterialID::Fire || m == MaterialID::Lava ||
                    m == MaterialID::Acid || m == MaterialID::Dragon_Fire) {
                    // Die in a puff of smoke
                    world.set_material(x, y, MaterialID::Smoke);
                    world.get_cell(x, y).set_lifetime(10);
                    return;
                }
            }
        }
    }

    // Fall down like a powder but with some floating
    // Try to move down
    if (world.in_bounds(x, y + 1)) {
        MaterialID below = world.get_material(x, y + 1);
        if (below == MaterialID::Empty || below == MaterialID::Steam ||
            below == MaterialID::Smoke || below == MaterialID::Helium ||
            below == MaterialID::Hydrogen) {
            world.try_move_cell(x, y, x, y + 1);
            return;
        }

        // Try diagonal down (like powder)
        int dir = (world.random_int() & 1) ? 1 : -1;
        if (world.in_bounds(x + dir, y + 1)) {
            MaterialID diag = world.get_material(x + dir, y + 1);
            if (diag == MaterialID::Empty || diag == MaterialID::Steam ||
                diag == MaterialID::Smoke) {
                world.try_move_cell(x, y, x + dir, y + 1);
                return;
            }
        }
        // Try other diagonal
        if (world.in_bounds(x - dir, y + 1)) {
            MaterialID diag = world.get_material(x - dir, y + 1);
            if (diag == MaterialID::Empty || diag == MaterialID::Steam ||
                diag == MaterialID::Smoke) {
                world.try_move_cell(x, y, x - dir, y + 1);
                return;
            }
        }
    }

    // If completely stuck on something that's not safe, try to drift sideways
    if ((world.random_int() & 7) == 0) {
        int dir = (world.random_int() & 1) ? 1 : -1;
        if (world.in_bounds(x + dir, y)) {
            MaterialID side = world.get_material(x + dir, y);
            if (side == MaterialID::Empty || side == MaterialID::Steam ||
                side == MaterialID::Smoke) {
                world.try_move_cell(x, y, x + dir, y);
            }
        }
    }
}

} // namespace Materials

} // namespace PixelEngine
