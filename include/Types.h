#pragma once

#include <cstdint>

namespace PixelEngine {

// Material identifier (1 byte per cell)
enum class MaterialID : uint8_t {
    // === BASIC (0-9) ===
    Empty = 0,
    Stone = 1,
    Sand = 2,
    Water = 3,
    Steam = 4,
    Oil = 5,
    Fire = 6,
    Wood = 7,
    Acid = 8,
    Lava = 9,

    // === POWDERS (10-19) ===
    Ash = 10,
    Dirt = 11,
    Gravel = 12,
    Snow = 13,
    Gunpowder = 14,
    Salt = 15,
    Coal = 16,
    Rust = 17,
    Sawdust = 18,
    Glass_Powder = 19,

    // === LIQUIDS (20-29) ===
    Honey = 20,
    Mud = 21,
    Blood = 22,
    Poison = 23,
    Slime = 24,
    Milk = 25,
    Alcohol = 26,
    Mercury = 27,
    Petrol = 28,
    Glue = 29,

    // === GASES (30-39) ===
    Smoke = 30,
    Toxic_Gas = 31,
    Hydrogen = 32,
    Helium = 33,
    Methane = 34,
    Spark = 35,
    Plasma = 36,
    Dust = 37,
    Spore = 38,
    Confetti = 39,

    // === SOLIDS (40-49) ===
    Grass = 40,
    Metal = 41,
    Gold = 42,
    Ice = 43,
    Glass = 44,
    Brick = 45,
    Obsidian = 46,
    Diamond = 47,
    Copper = 48,
    Rubber = 49,

    // === ORGANIC (50-59) ===
    Leaf = 50,
    Moss = 51,
    Vine = 52,
    Fungus = 53,
    Seed = 54,
    Flower = 55,
    Algae = 56,
    Coral = 57,
    Wax = 58,
    Flesh = 59,

    // === SPECIAL (60-69) ===
    Person = 60,
    Clone = 61,
    Void = 62,
    Fuse = 63,
    TNT = 64,
    C4 = 65,
    Firework = 66,
    Lightning = 67,
    Portal_In = 68,
    Portal_Out = 69,

    // === FANTASY (70-79) ===
    Magic = 70,
    Crystal = 71,
    Ectoplasm = 72,
    Antimatter = 73,
    Fairy_Dust = 74,
    Dragon_Fire = 75,
    Frost = 76,
    Ember = 77,
    Stardust = 78,
    Void_Dust = 79,

    // === SPAWNERS (80) ===
    Life = 80,       // Falling particle that spawns Person on safe ground

    // === NEW POWDERS (81-85) ===
    Thermite_Powder = 81, // Burns extremely hot when ignited
    Sugar = 82,           // Sweet powder, dissolves in water, flammable
    Iron_Filings = 83,    // Metal shavings, rusts with water
    Chalk = 84,           // White chalk powder
    Calcium = 85,         // Calcium powder, reactive with water

    // === NEW LIQUIDS (86-90) ===
    Tar = 86,        // Very slow black sticky liquid
    Juice = 87,      // Orange liquid, evaporates
    Sap = 88,        // Tree sap, amber colored
    Bleach = 89,     // Corrosive to organics
    Ink = 90,        // Dark liquid for staining

    // === NEW GASES (91-93) ===
    Chlorine = 91,   // Toxic green gas, sinks
    Liquid_Nitrogen = 92, // Freezing gas effect
    Oxygen = 93,     // Makes fires burn brighter

    // === NEW SOLIDS (94-97) ===
    Concrete = 94,   // Strong building material
    Titanium = 95,   // Strong silvery metal
    Clay = 96,       // Moldable, fires into brick
    Charcoal = 97,   // Burnt wood, slow fuel

    // === NEW ORGANIC (98-100) ===
    Bamboo = 98,     // Fast-growing plant
    Honeycomb = 99,  // Solid wax structure
    Bone = 100,      // Skeletal remains

    // === NEW SPECIAL (101-102) ===
    Napalm = 101,    // Sticky spreading fire
    Thermite = 102,  // Extremely hot burning

    COUNT
};

// Material states for physics behavior
enum class MaterialState : uint8_t {
    Empty,      // No material
    Solid,      // Immovable
    Powder,     // Granular - falls and piles
    Liquid,     // Fluid - spreads and falls
    Gas         // Rises and disperses
};

// RGBA color (4 bytes)
struct Color {
    uint8_t r, g, b, a;

    Color() : r(0), g(0), b(0), a(255) {}
    Color(uint8_t r, uint8_t g, uint8_t b, uint8_t a = 255)
        : r(r), g(g), b(b), a(a) {}

    // Convert to 32-bit RGBA for Metal texture
    uint32_t to_rgba32() const {
        return (r << 0) | (g << 8) | (b << 16) | (a << 24);
    }
};

// Per-cell runtime data (kept minimal for cache performance)
struct Cell {
    MaterialID material_id;
    uint8_t flags;     // Bit 0: updated this frame, Bit 1: flow direction (0=left, 1=right), Bits 2-7: lifetime/reserved
    int8_t velocity_y;  // Vertical velocity (-128 to 127, typically -8 to +8)

    Cell() : material_id(MaterialID::Empty), flags(0), velocity_y(0) {}
    explicit Cell(MaterialID id) : material_id(id), flags(0), velocity_y(0) {}

    bool was_updated() const { return flags & 0x01; }
    void mark_updated() { flags |= 0x01; }
    void clear_updated() { flags &= ~0x01; }

    // Flow direction for liquids (bit 1)
    bool get_flow_direction() const { return (flags & 0x02) != 0; }  // 0=left, 1=right
    void set_flow_direction(bool right) {
        if (right) flags |= 0x02;
        else flags &= ~0x02;
    }

    // Lifetime for temporary materials like fire (bits 2-7, value 0-63)
    uint8_t get_lifetime() const { return (flags >> 2) & 0x3F; }
    void set_lifetime(uint8_t lifetime) {
        flags = (flags & 0x03) | ((lifetime & 0x3F) << 2);
    }
    void decrement_lifetime() {
        uint8_t life = get_lifetime();
        if (life > 0) set_lifetime(life - 1);
    }

    // Velocity helpers
    void add_velocity(int8_t delta) {
        int16_t new_vel = static_cast<int16_t>(velocity_y) + delta;
        if (new_vel > 127) new_vel = 127;
        if (new_vel < -128) new_vel = -128;
        velocity_y = static_cast<int8_t>(new_vel);
    }

    void clamp_velocity(int8_t min_val, int8_t max_val) {
        if (velocity_y < min_val) velocity_y = min_val;
        if (velocity_y > max_val) velocity_y = max_val;
    }

    void reset_velocity() {
        velocity_y = 0;
    }

    // ========================================
    // Person-specific state (reuses existing fields creatively)
    // ========================================

    // Health for people (0-127, stored in velocity_y when person is grounded)
    // When person is falling, velocity_y is used for actual velocity
    // We track if person is grounded using a separate flag
    uint8_t get_health() const {
        // Health stored in positive range of velocity_y
        return static_cast<uint8_t>(velocity_y > 0 ? velocity_y : 0);
    }
    void set_health(uint8_t health) {
        velocity_y = static_cast<int8_t>(health > 127 ? 127 : health);
    }
    void damage_health(uint8_t amount) {
        int16_t new_health = static_cast<int16_t>(velocity_y) - amount;
        velocity_y = static_cast<int8_t>(new_health < 0 ? 0 : new_health);
    }

    // Person facing direction (reuses flow_direction bit 1)
    bool get_person_facing_right() const { return get_flow_direction(); }
    void set_person_facing_right(bool right) { set_flow_direction(right); }

    // Reproduction cooldown (6 bits from lifetime field, 0-63)
    uint8_t get_reproduction_cooldown() const { return get_lifetime(); }
    void set_reproduction_cooldown(uint8_t cooldown) { set_lifetime(cooldown); }
    void decrement_reproduction_cooldown() { decrement_lifetime(); }
};

// 2D position
struct Position {
    int32_t x, y;

    Position() : x(0), y(0) {}
    Position(int32_t x, int32_t y) : x(x), y(y) {}

    bool operator==(const Position& other) const {
        return x == other.x && y == other.y;
    }
};

// Constants
constexpr int32_t CHUNK_SIZE = 64;  // 64×64 cells per chunk
constexpr int32_t WORLD_WIDTH = 800;
constexpr int32_t WORLD_HEIGHT = 600;
constexpr float TARGET_FPS = 60.0f;
constexpr float FIXED_TIMESTEP = 1.0f / TARGET_FPS;

} // namespace PixelEngine
