#include "Material.h"
#include "World.h"
#include <random>
#include <cmath>

namespace PixelEngine {

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

    // ========================================================================
    // NEW POWDERS (81-85)
    // ========================================================================

    // Thermite Powder - burns extremely hot when ignited
    materials_[static_cast<size_t>(MaterialID::Thermite_Powder)] = MaterialDef(
        MaterialID::Thermite_Powder,
        MaterialState::Powder,
        4.5f,  // Heavy iron oxide powder
        Color(139, 69, 19),  // Rusty brown
        15
    );

    // Sugar - sweet powder, dissolves in water, highly flammable
    materials_[static_cast<size_t>(MaterialID::Sugar)] = MaterialDef(
        MaterialID::Sugar,
        MaterialState::Powder,
        1.55f,
        Color(255, 250, 240),  // Off-white
        5
    );

    // Iron Filings - metal shavings, rusts with water
    materials_[static_cast<size_t>(MaterialID::Iron_Filings)] = MaterialDef(
        MaterialID::Iron_Filings,
        MaterialState::Powder,
        7.8f,  // Dense iron
        Color(70, 70, 75),  // Dark metallic gray
        10
    );

    // Chalk - white powder for marking
    materials_[static_cast<size_t>(MaterialID::Chalk)] = MaterialDef(
        MaterialID::Chalk,
        MaterialState::Powder,
        2.7f,
        Color(245, 245, 245),  // White
        8
    );

    // Calcium - calcium powder, reactive with water (fizzes)
    materials_[static_cast<size_t>(MaterialID::Calcium)] = MaterialDef(
        MaterialID::Calcium,
        MaterialState::Powder,
        1.55f,
        Color(230, 230, 210),  // Pale cream
        10
    );

    // ========================================================================
    // NEW LIQUIDS (86-90)
    // ========================================================================

    // Tar - very slow black sticky liquid, flammable
    materials_[static_cast<size_t>(MaterialID::Tar)] = MaterialDef(
        MaterialID::Tar,
        MaterialState::Liquid,
        1.2f,
        Color(20, 15, 10),  // Almost black
        5
    );

    // Juice - orange liquid, evaporates slowly
    materials_[static_cast<size_t>(MaterialID::Juice)] = MaterialDef(
        MaterialID::Juice,
        MaterialState::Liquid,
        1.05f,
        Color(255, 165, 0),  // Orange
        20
    );

    // Sap - tree sap, amber colored, can solidify
    materials_[static_cast<size_t>(MaterialID::Sap)] = MaterialDef(
        MaterialID::Sap,
        MaterialState::Liquid,
        1.3f,
        Color(218, 165, 32),  // Amber/goldenrod
        15
    );

    // Bleach - corrosive cleaning liquid, destroys organics
    materials_[static_cast<size_t>(MaterialID::Bleach)] = MaterialDef(
        MaterialID::Bleach,
        MaterialState::Liquid,
        1.1f,
        Color(240, 255, 240),  // Very pale green-white
        8
    );

    // Ink - dark liquid for staining
    materials_[static_cast<size_t>(MaterialID::Ink)] = MaterialDef(
        MaterialID::Ink,
        MaterialState::Liquid,
        1.0f,
        Color(10, 10, 30),  // Very dark blue-black
        5
    );

    // ========================================================================
    // NEW GASES (91-93)
    // ========================================================================

    // Chlorine - toxic green gas, heavier than air (sinks!)
    materials_[static_cast<size_t>(MaterialID::Chlorine)] = MaterialDef(
        MaterialID::Chlorine,
        MaterialState::Gas,
        2.5f,  // Heavier than air - this gas sinks!
        Color(144, 238, 144, 180),  // Light green, semi-transparent
        20
    );

    // Liquid Nitrogen - freezing effect, rises as gas
    materials_[static_cast<size_t>(MaterialID::Liquid_Nitrogen)] = MaterialDef(
        MaterialID::Liquid_Nitrogen,
        MaterialState::Gas,
        0.08f,  // Very light
        Color(200, 220, 255, 160),  // Pale blue, semi-transparent
        15
    );

    // Oxygen - makes fires burn brighter, rises
    materials_[static_cast<size_t>(MaterialID::Oxygen)] = MaterialDef(
        MaterialID::Oxygen,
        MaterialState::Gas,
        0.09f,
        Color(180, 200, 255, 120),  // Very pale blue, mostly transparent
        10
    );

    // ========================================================================
    // NEW SOLIDS (94-97)
    // ========================================================================

    // Concrete - strong building material
    materials_[static_cast<size_t>(MaterialID::Concrete)] = MaterialDef(
        MaterialID::Concrete,
        MaterialState::Solid,
        2.4f,
        Color(128, 128, 128),  // Gray
        12
    );

    // Titanium - strong silvery metal
    materials_[static_cast<size_t>(MaterialID::Titanium)] = MaterialDef(
        MaterialID::Titanium,
        MaterialState::Solid,
        4.5f,
        Color(180, 185, 190),  // Silvery
        8
    );

    // Clay - moldable material (solid until heated)
    materials_[static_cast<size_t>(MaterialID::Clay)] = MaterialDef(
        MaterialID::Clay,
        MaterialState::Solid,
        1.8f,
        Color(165, 113, 78),  // Terracotta brown
        15
    );

    // Charcoal - burnt wood, slow burning fuel
    materials_[static_cast<size_t>(MaterialID::Charcoal)] = MaterialDef(
        MaterialID::Charcoal,
        MaterialState::Solid,
        0.5f,  // Light like wood
        Color(40, 35, 30),  // Very dark gray
        10
    );

    // ========================================================================
    // NEW ORGANIC (98-100)
    // ========================================================================

    // Bamboo - fast-growing plant material
    materials_[static_cast<size_t>(MaterialID::Bamboo)] = MaterialDef(
        MaterialID::Bamboo,
        MaterialState::Solid,
        0.7f,
        Color(144, 190, 109),  // Light green
        20
    );

    // Honeycomb - solid wax structure
    materials_[static_cast<size_t>(MaterialID::Honeycomb)] = MaterialDef(
        MaterialID::Honeycomb,
        MaterialState::Solid,
        0.9f,
        Color(255, 200, 60),  // Golden yellow
        15
    );

    // Bone - skeletal remains
    materials_[static_cast<size_t>(MaterialID::Bone)] = MaterialDef(
        MaterialID::Bone,
        MaterialState::Solid,
        1.9f,
        Color(230, 220, 200),  // Off-white/ivory
        12
    );

    // ========================================================================
    // NEW SPECIAL (101-102)
    // ========================================================================

    // Napalm - sticky spreading fire
    materials_[static_cast<size_t>(MaterialID::Napalm)] = MaterialDef(
        MaterialID::Napalm,
        MaterialState::Liquid,
        0.9f,
        Color(255, 100, 0),  // Bright orange
        30
    );

    // Thermite - extremely hot burning compound (ignited form)
    materials_[static_cast<size_t>(MaterialID::Thermite)] = MaterialDef(
        MaterialID::Thermite,
        MaterialState::Liquid,  // Molten when burning
        7.0f,  // Heavy molten iron
        Color(255, 255, 200),  // Blindingly bright white-yellow
        40
    );

    // ========================================================================
    // EXPANSION: BASIC (103-112)
    // ========================================================================

    materials_[static_cast<size_t>(MaterialID::Bedrock)] = MaterialDef(
        MaterialID::Bedrock, MaterialState::Solid, 1000.0f,
        Color(30, 30, 35), 5
    );

    materials_[static_cast<size_t>(MaterialID::Ceramic)] = MaterialDef(
        MaterialID::Ceramic, MaterialState::Solid, 2.5f,
        Color(210, 180, 140), 15
    );

    materials_[static_cast<size_t>(MaterialID::Granite)] = MaterialDef(
        MaterialID::Granite, MaterialState::Solid, 2.7f,
        Color(130, 120, 110), 25
    );

    materials_[static_cast<size_t>(MaterialID::Marble)] = MaterialDef(
        MaterialID::Marble, MaterialState::Solid, 2.7f,
        Color(240, 240, 245), 10
    );

    materials_[static_cast<size_t>(MaterialID::Sandstone)] = MaterialDef(
        MaterialID::Sandstone, MaterialState::Solid, 2.3f,
        Color(210, 180, 140), 20
    );

    materials_[static_cast<size_t>(MaterialID::Limestone)] = MaterialDef(
        MaterialID::Limestone, MaterialState::Solid, 2.5f,
        Color(220, 215, 200), 15
    );

    materials_[static_cast<size_t>(MaterialID::Slate)] = MaterialDef(
        MaterialID::Slate, MaterialState::Solid, 2.8f,
        Color(80, 85, 90), 10
    );

    materials_[static_cast<size_t>(MaterialID::Basalt)] = MaterialDef(
        MaterialID::Basalt, MaterialState::Solid, 3.0f,
        Color(50, 50, 55), 8
    );

    materials_[static_cast<size_t>(MaterialID::Quartz_Block)] = MaterialDef(
        MaterialID::Quartz_Block, MaterialState::Solid, 2.6f,
        Color(250, 245, 250), 5
    );

    materials_[static_cast<size_t>(MaterialID::Soil)] = MaterialDef(
        MaterialID::Soil, MaterialState::Solid, 1.5f,
        Color(90, 60, 40), 20
    );

    // ========================================================================
    // EXPANSION: POWDERS (113-117)
    // ========================================================================

    materials_[static_cast<size_t>(MaterialID::Flour)] = MaterialDef(
        MaterialID::Flour, MaterialState::Powder, 0.6f,
        Color(250, 245, 230), 5
    );

    materials_[static_cast<size_t>(MaterialID::Sulfur)] = MaterialDef(
        MaterialID::Sulfur, MaterialState::Powder, 2.0f,
        Color(230, 220, 50), 15
    );

    materials_[static_cast<size_t>(MaterialID::Cement)] = MaterialDef(
        MaterialID::Cement, MaterialState::Powder, 1.5f,
        Color(160, 160, 155), 10
    );

    materials_[static_cast<size_t>(MaterialID::Fertilizer)] = MaterialDef(
        MaterialID::Fertilizer, MaterialState::Powder, 1.2f,
        Color(80, 50, 30), 15
    );

    materials_[static_cast<size_t>(MaterialID::Volcanic_Ash)] = MaterialDef(
        MaterialID::Volcanic_Ash, MaterialState::Powder, 1.4f,
        Color(70, 65, 60), 12
    );

    // ========================================================================
    // EXPANSION: LIQUIDS (118-122)
    // ========================================================================

    materials_[static_cast<size_t>(MaterialID::Brine)] = MaterialDef(
        MaterialID::Brine, MaterialState::Liquid, 1.03f,
        Color(100, 150, 180), 10
    );

    materials_[static_cast<size_t>(MaterialID::Coffee)] = MaterialDef(
        MaterialID::Coffee, MaterialState::Liquid, 1.0f,
        Color(70, 45, 25), 10
    );

    materials_[static_cast<size_t>(MaterialID::Soap)] = MaterialDef(
        MaterialID::Soap, MaterialState::Liquid, 0.95f,
        Color(200, 220, 255), 15
    );

    materials_[static_cast<size_t>(MaterialID::Paint)] = MaterialDef(
        MaterialID::Paint, MaterialState::Liquid, 1.3f,
        Color(200, 50, 50), 40  // High variance for multi-color
    );

    materials_[static_cast<size_t>(MaterialID::Sewage)] = MaterialDef(
        MaterialID::Sewage, MaterialState::Liquid, 1.05f,
        Color(80, 70, 50), 15
    );

    // ========================================================================
    // EXPANSION: GASES (123-129)
    // ========================================================================

    materials_[static_cast<size_t>(MaterialID::Ammonia)] = MaterialDef(
        MaterialID::Ammonia, MaterialState::Gas, 0.6f,
        Color(200, 255, 200, 140), 15
    );

    materials_[static_cast<size_t>(MaterialID::Carbon_Dioxide)] = MaterialDef(
        MaterialID::Carbon_Dioxide, MaterialState::Gas, 1.5f,  // Heavier than air
        Color(180, 180, 180, 100), 10
    );

    materials_[static_cast<size_t>(MaterialID::Nitrous)] = MaterialDef(
        MaterialID::Nitrous, MaterialState::Gas, 0.5f,
        Color(200, 200, 255, 120), 10
    );

    materials_[static_cast<size_t>(MaterialID::Steam_Hot)] = MaterialDef(
        MaterialID::Steam_Hot, MaterialState::Gas, 0.05f,
        Color(255, 255, 255, 160), 10
    );

    materials_[static_cast<size_t>(MaterialID::Miasma)] = MaterialDef(
        MaterialID::Miasma, MaterialState::Gas, 0.8f,
        Color(100, 80, 60, 150), 20
    );

    materials_[static_cast<size_t>(MaterialID::Pheromone)] = MaterialDef(
        MaterialID::Pheromone, MaterialState::Gas, 0.3f,
        Color(255, 200, 220, 100), 15
    );

    materials_[static_cast<size_t>(MaterialID::Nerve_Gas)] = MaterialDef(
        MaterialID::Nerve_Gas, MaterialState::Gas, 1.2f,
        Color(180, 255, 180, 130), 15
    );

    // ========================================================================
    // EXPANSION: SOLIDS (130-136)
    // ========================================================================

    materials_[static_cast<size_t>(MaterialID::Silver)] = MaterialDef(
        MaterialID::Silver, MaterialState::Solid, 10.5f,
        Color(192, 192, 200), 8
    );

    materials_[static_cast<size_t>(MaterialID::Platinum)] = MaterialDef(
        MaterialID::Platinum, MaterialState::Solid, 21.5f,
        Color(220, 220, 230), 5
    );

    materials_[static_cast<size_t>(MaterialID::Lead)] = MaterialDef(
        MaterialID::Lead, MaterialState::Solid, 11.3f,
        Color(90, 90, 100), 8
    );

    materials_[static_cast<size_t>(MaterialID::Tin)] = MaterialDef(
        MaterialID::Tin, MaterialState::Solid, 7.3f,
        Color(180, 180, 175), 8
    );

    materials_[static_cast<size_t>(MaterialID::Zinc)] = MaterialDef(
        MaterialID::Zinc, MaterialState::Solid, 7.1f,
        Color(160, 170, 180), 10
    );

    materials_[static_cast<size_t>(MaterialID::Bronze)] = MaterialDef(
        MaterialID::Bronze, MaterialState::Solid, 8.7f,
        Color(180, 130, 70), 12
    );

    materials_[static_cast<size_t>(MaterialID::Steel)] = MaterialDef(
        MaterialID::Steel, MaterialState::Solid, 7.8f,
        Color(140, 145, 150), 8
    );

    // ========================================================================
    // EXPANSION: ORGANIC (137-143)
    // ========================================================================

    materials_[static_cast<size_t>(MaterialID::Pollen)] = MaterialDef(
        MaterialID::Pollen, MaterialState::Powder, 0.3f,
        Color(255, 220, 80), 20
    );

    materials_[static_cast<size_t>(MaterialID::Root)] = MaterialDef(
        MaterialID::Root, MaterialState::Solid, 0.9f,
        Color(120, 80, 50), 15
    );

    materials_[static_cast<size_t>(MaterialID::Bark)] = MaterialDef(
        MaterialID::Bark, MaterialState::Solid, 0.7f,
        Color(100, 70, 45), 20
    );

    materials_[static_cast<size_t>(MaterialID::Fruit)] = MaterialDef(
        MaterialID::Fruit, MaterialState::Solid, 0.9f,
        Color(255, 80, 80), 30
    );

    materials_[static_cast<size_t>(MaterialID::Egg)] = MaterialDef(
        MaterialID::Egg, MaterialState::Solid, 1.0f,
        Color(250, 245, 230), 10
    );

    materials_[static_cast<size_t>(MaterialID::Web)] = MaterialDef(
        MaterialID::Web, MaterialState::Solid, 0.1f,
        Color(240, 240, 245, 200), 5
    );

    materials_[static_cast<size_t>(MaterialID::Mucus)] = MaterialDef(
        MaterialID::Mucus, MaterialState::Liquid, 1.1f,
        Color(180, 220, 150, 200), 15
    );

    // ========================================================================
    // EXPANSION: SPECIAL (144-151)
    // ========================================================================

    materials_[static_cast<size_t>(MaterialID::Bomb)] = MaterialDef(
        MaterialID::Bomb, MaterialState::Solid, 3.0f,
        Color(50, 50, 50), 5
    );

    materials_[static_cast<size_t>(MaterialID::Nuke)] = MaterialDef(
        MaterialID::Nuke, MaterialState::Solid, 15.0f,
        Color(40, 60, 40), 5
    );

    materials_[static_cast<size_t>(MaterialID::Laser)] = MaterialDef(
        MaterialID::Laser, MaterialState::Gas, 0.0f,
        Color(255, 0, 0), 20
    );

    materials_[static_cast<size_t>(MaterialID::Black_Hole)] = MaterialDef(
        MaterialID::Black_Hole, MaterialState::Solid, 1000.0f,
        Color(10, 0, 20), 5
    );

    materials_[static_cast<size_t>(MaterialID::White_Hole)] = MaterialDef(
        MaterialID::White_Hole, MaterialState::Solid, 0.0f,
        Color(255, 255, 255), 5
    );

    materials_[static_cast<size_t>(MaterialID::Acid_Gas)] = MaterialDef(
        MaterialID::Acid_Gas, MaterialState::Gas, 1.1f,
        Color(150, 255, 100, 150), 15
    );

    materials_[static_cast<size_t>(MaterialID::Ice_Bomb)] = MaterialDef(
        MaterialID::Ice_Bomb, MaterialState::Solid, 2.0f,
        Color(150, 200, 255), 10
    );

    materials_[static_cast<size_t>(MaterialID::Fire_Bomb)] = MaterialDef(
        MaterialID::Fire_Bomb, MaterialState::Solid, 2.0f,
        Color(255, 100, 50), 15
    );

    // ========================================================================
    // EXPANSION: FANTASY (152-161)
    // ========================================================================

    materials_[static_cast<size_t>(MaterialID::Mana)] = MaterialDef(
        MaterialID::Mana, MaterialState::Liquid, 0.5f,
        Color(100, 150, 255), 20
    );

    materials_[static_cast<size_t>(MaterialID::Mirage)] = MaterialDef(
        MaterialID::Mirage, MaterialState::Gas, 0.01f,
        Color(255, 220, 180, 80), 30
    );

    materials_[static_cast<size_t>(MaterialID::Holy_Water)] = MaterialDef(
        MaterialID::Holy_Water, MaterialState::Liquid, 1.0f,
        Color(220, 240, 255), 10
    );

    materials_[static_cast<size_t>(MaterialID::Cursed)] = MaterialDef(
        MaterialID::Cursed, MaterialState::Solid, 2.0f,
        Color(50, 20, 60), 15
    );

    materials_[static_cast<size_t>(MaterialID::Blessed)] = MaterialDef(
        MaterialID::Blessed, MaterialState::Solid, 1.0f,
        Color(255, 250, 200), 10
    );

    materials_[static_cast<size_t>(MaterialID::Soul)] = MaterialDef(
        MaterialID::Soul, MaterialState::Gas, 0.1f,
        Color(200, 220, 255, 120), 20
    );

    materials_[static_cast<size_t>(MaterialID::Spirit)] = MaterialDef(
        MaterialID::Spirit, MaterialState::Gas, 0.05f,
        Color(180, 200, 255, 100), 25
    );

    materials_[static_cast<size_t>(MaterialID::Aether)] = MaterialDef(
        MaterialID::Aether, MaterialState::Gas, 0.01f,
        Color(255, 255, 200, 80), 15
    );

    materials_[static_cast<size_t>(MaterialID::Nether)] = MaterialDef(
        MaterialID::Nether, MaterialState::Gas, 2.0f,  // Sinks
        Color(80, 20, 100, 150), 20
    );

    materials_[static_cast<size_t>(MaterialID::Phoenix_Ash)] = MaterialDef(
        MaterialID::Phoenix_Ash, MaterialState::Powder, 0.5f,
        Color(255, 150, 50), 25
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

    // === NEW MATERIAL COMBINATIONS ===
    // Sugar + Water = dissolves (handled in update, but also here)
    {MaterialID::Sugar, MaterialID::Water, MaterialID::Empty, MaterialID::Water, 8},
    // Sugar + Fire = quick bright fire
    {MaterialID::Sugar, MaterialID::Fire, MaterialID::Fire, MaterialID::Fire, 2},
    // Iron_Filings + Water = Rust
    {MaterialID::Iron_Filings, MaterialID::Water, MaterialID::Rust, MaterialID::Water, 32},
    // Chalk + Water = dissolves slowly
    {MaterialID::Chalk, MaterialID::Water, MaterialID::Empty, MaterialID::Water, 64},
    // Chalk + Acid = fizzy reaction
    {MaterialID::Chalk, MaterialID::Acid, MaterialID::Empty, MaterialID::Steam, 8},
    // Calcium + Water = Hydrogen (reactive!)
    {MaterialID::Calcium, MaterialID::Water, MaterialID::Hydrogen, MaterialID::Steam, 4},
    // Tar + Fire = long burning fire
    {MaterialID::Tar, MaterialID::Fire, MaterialID::Fire, MaterialID::Fire, 4},
    // Tar + Petrol = more flammable
    {MaterialID::Tar, MaterialID::Petrol, MaterialID::Petrol, MaterialID::Petrol, 16},
    // Juice + Sugar = Honey (sweet concentration)
    {MaterialID::Juice, MaterialID::Sugar, MaterialID::Honey, MaterialID::Empty, 32},
    // Sap + Fire = Fire + Smoke
    {MaterialID::Sap, MaterialID::Fire, MaterialID::Fire, MaterialID::Smoke, 4},
    // Bleach + Acid = Toxic_Gas (dangerous mix!)
    {MaterialID::Bleach, MaterialID::Acid, MaterialID::Toxic_Gas, MaterialID::Toxic_Gas, 2},
    // Ink + Water = diluted (becomes water)
    {MaterialID::Ink, MaterialID::Water, MaterialID::Water, MaterialID::Empty, 32},
    // Chlorine + Water = slight poison
    {MaterialID::Chlorine, MaterialID::Water, MaterialID::Poison, MaterialID::Empty, 16},
    // Chlorine + Hydrogen = explosive (handled as fire)
    {MaterialID::Chlorine, MaterialID::Hydrogen, MaterialID::Fire, MaterialID::Fire, 4},
    // Liquid_Nitrogen + Water = Ice
    {MaterialID::Liquid_Nitrogen, MaterialID::Water, MaterialID::Ice, MaterialID::Empty, 2},
    // Liquid_Nitrogen + Lava = Obsidian + Steam
    {MaterialID::Liquid_Nitrogen, MaterialID::Lava, MaterialID::Obsidian, MaterialID::Steam, 1},
    // Oxygen + Fire = more fire
    {MaterialID::Oxygen, MaterialID::Fire, MaterialID::Fire, MaterialID::Fire, 2},
    // Oxygen + Ember = Fire
    {MaterialID::Oxygen, MaterialID::Ember, MaterialID::Fire, MaterialID::Empty, 2},
    // Clay + Fire = Brick (firing clay)
    {MaterialID::Clay, MaterialID::Fire, MaterialID::Brick, MaterialID::Smoke, 16},
    // Clay + Lava = Brick
    {MaterialID::Clay, MaterialID::Lava, MaterialID::Brick, MaterialID::Stone, 8},
    // Charcoal + Fire = more fire + ember
    {MaterialID::Charcoal, MaterialID::Fire, MaterialID::Fire, MaterialID::Ember, 8},
    // Bamboo + Fire = Fire + Ash
    {MaterialID::Bamboo, MaterialID::Fire, MaterialID::Fire, MaterialID::Ash, 4},
    // Honeycomb + Fire = Honey (melts)
    {MaterialID::Honeycomb, MaterialID::Fire, MaterialID::Honey, MaterialID::Smoke, 8},
    // Bone + Acid = dissolves
    {MaterialID::Bone, MaterialID::Acid, MaterialID::Empty, MaterialID::Toxic_Gas, 16},
    // Napalm spreads to oil
    {MaterialID::Napalm, MaterialID::Oil, MaterialID::Napalm, MaterialID::Napalm, 4},
    // Thermite + Metal = Lava (melts through!)
    {MaterialID::Thermite, MaterialID::Metal, MaterialID::Thermite, MaterialID::Lava, 4},
    // Thermite_Powder + Spark = Thermite
    {MaterialID::Thermite_Powder, MaterialID::Spark, MaterialID::Thermite, MaterialID::Empty, 1},
    // Thermite_Powder + Fire = Thermite
    {MaterialID::Thermite_Powder, MaterialID::Fire, MaterialID::Thermite, MaterialID::Fire, 2},
};

static const int NUM_COMBINATIONS = sizeof(COMBINATIONS) / sizeof(COMBINATIONS[0]);

// ============================================================================
// OPTIMIZED COMBINATION LOOKUP SYSTEM
// ============================================================================
// Uses O(1) hash lookup instead of O(n) iteration through all recipes.
// This is critical for performance when handling 100k+ particles.

// Max material ID for lookup table sizing
static constexpr int MAX_MATERIAL_ID = 256;

// Lookup table: combo_lookup[mat_a][mat_b] = recipe index + 1 (0 = no recipe)
// Using int16_t to save memory (65KB table instead of 256KB)
static int16_t combo_lookup[MAX_MATERIAL_ID][MAX_MATERIAL_ID] = {};

// Bitset of materials that participate in any combination
static bool has_combinations[MAX_MATERIAL_ID] = {};

// Initialize lookup tables once at startup
static bool init_combination_lookup() {
    // Build lookup table for O(1) recipe lookup
    for (int i = 0; i < NUM_COMBINATIONS; i++) {
        const MaterialCombination& combo = COMBINATIONS[i];
        int a = static_cast<int>(combo.mat_a);
        int b = static_cast<int>(combo.mat_b);

        // Store recipe index + 1 (0 means no recipe)
        // Note: if multiple recipes exist for same pair, last one wins
        combo_lookup[a][b] = i + 1;
        combo_lookup[b][a] = i + 1;  // Also store reverse for quick lookup

        // Mark materials as having combinations
        has_combinations[a] = true;
        has_combinations[b] = true;
    }
    return true;
}

// Static initialization - runs once at program start
static bool combo_lookup_initialized = init_combination_lookup();

// Helper to apply combination results and initialize special materials
static inline void apply_combination_result(World& world, int32_t x, int32_t y, MaterialID result) {
    world.set_material(x, y, result);
    if (result == MaterialID::Fire || result == MaterialID::Steam ||
        result == MaterialID::Smoke || result == MaterialID::Toxic_Gas) {
        world.get_cell(x, y).set_lifetime(30);
    } else if (result == MaterialID::Magic) {
        world.get_cell(x, y).set_lifetime(40);
    }
}

// Check if a material at position (x, y) can combine with any neighbors
// Returns true if a combination occurred
// OPTIMIZED: Uses O(1) lookup instead of O(n) iteration
static bool try_material_combination(World& world, int32_t x, int32_t y) {
    MaterialID my_mat = world.get_material(x, y);
    if (my_mat == MaterialID::Empty) return false;

    int my_mat_idx = static_cast<int>(my_mat);

    // Fast early exit: if this material has no recipes, skip entirely
    if (!has_combinations[my_mat_idx]) return false;

    // Check all 8 neighbors
    for (int dy = -1; dy <= 1; dy++) {
        for (int dx = -1; dx <= 1; dx++) {
            if (dx == 0 && dy == 0) continue;

            int nx = x + dx, ny = y + dy;
            if (!world.in_bounds(nx, ny)) continue;

            MaterialID neighbor_mat = world.get_material(nx, ny);
            if (neighbor_mat == MaterialID::Empty) continue;

            int neighbor_idx = static_cast<int>(neighbor_mat);

            // O(1) lookup for recipe
            int recipe_idx = combo_lookup[my_mat_idx][neighbor_idx];
            if (recipe_idx == 0) continue;  // No recipe for this pair

            const MaterialCombination& combo = COMBINATIONS[recipe_idx - 1];

            // Random chance check
            if ((world.random_int() % combo.chance) != 0) continue;

            // Determine if we matched forward (my_mat == mat_a) or reverse
            bool forward = (my_mat == combo.mat_a);

            if (forward) {
                apply_combination_result(world, x, y, combo.result_a);
                apply_combination_result(world, nx, ny, combo.result_b);
            } else {
                apply_combination_result(world, x, y, combo.result_b);
                apply_combination_result(world, nx, ny, combo.result_a);
            }
            return true;
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

// ============================================================================
// VILLAGE BUILDING SYSTEM - Complex structures with multiple materials
// ============================================================================

// Building type enumeration - expanded with complex structures
enum class BuildingType : uint8_t {
    // === Basic Village (0-9) ===
    Cottage = 0,      // Small wooden cottage with peaked roof
    StoneHouse = 1,   // Larger stone house with windows
    WatchTower = 2,   // Tall stone watchtower
    Barn = 3,         // Large wooden barn
    Well = 4,         // Village well
    Bridge = 5,       // Wooden bridge/platform
    Fence = 6,        // Simple fence line
    Shrine = 7,       // Small decorative shrine with crystal
    Windmill = 8,     // Windmill tower
    Inn = 9,          // Two-story inn

    // === Advanced Structures (10-19) ===
    Castle = 10,      // Multi-tower castle with battlements
    Church = 11,      // Tall church with steeple
    Market = 12,      // Open-air market stalls
    Lighthouse = 13,  // Tall lighthouse with light
    Tavern = 14,      // Ground-level tavern
    Stairs = 15,      // Climbable stairs structure
    Ladder = 16,      // Vertical ladder for climbing
    SkywalkBridge = 17, // Elevated bridge between buildings
    GrandHall = 18,   // Large gathering hall
    Observatory = 19, // Domed observatory tower

    // === Decorative/Infrastructure (20-24) ===
    Fountain = 20,    // Decorative fountain
    Statue = 21,      // Stone/copper statue
    Garden = 22,      // Small walled garden
    Dock = 23,        // Wooden dock extending out
    Tower = 24,       // Simple climbable tower with platforms

    // === VERTICAL STRUCTURES (25-39) - Super climbable! ===
    SpiralTower = 25,     // Spiral staircase tower (very tall)
    Scaffolding = 26,     // Construction scaffolding (easy to climb)
    Skyscraper = 27,      // Multi-story building with floors
    ClimbingWall = 28,    // Dense climbable wall
    TreeHouse = 29,       // Elevated platform with ladder
    MegaTower = 30,       // Extremely tall tower with many platforms
    ZigzagStairs = 31,    // Zigzag stairs going up
    Elevator = 32,        // Vertical shaft with platforms
    Apartment = 33,       // Multi-unit vertical building
    Pyramid = 34,         // Step pyramid (climbable sides)
    Pagoda = 35,          // Multi-tier pagoda tower
    Aqueduct = 36,        // Tall arched aqueduct
    BellTower = 37,       // Very tall bell tower
    Crane = 38,           // Construction crane (very tall)
    SkyPlatform = 39,     // Floating platforms with ladders

    COUNT
};

// Build a cozy wooden cottage (6 wide, 5 tall with peaked roof)
static void build_cottage(World& world, int32_t base_x, int32_t base_y, uint32_t seed) {
    // Stone foundation
    for (int dx = 0; dx < 6; dx++) {
        place_building_block(world, base_x + dx, base_y, MaterialID::Stone);
    }

    // Wooden walls (3 tall)
    for (int dy = 1; dy <= 3; dy++) {
        place_building_block(world, base_x, base_y - dy, MaterialID::Wood);
        place_building_block(world, base_x + 5, base_y - dy, MaterialID::Wood);
    }

    // Front wall with door (2-wide opening in middle)
    for (int dx = 1; dx < 5; dx++) {
        if (dx != 2 && dx != 3) {  // Door opening
            place_building_block(world, base_x + dx, base_y - 1, MaterialID::Wood);
        }
        place_building_block(world, base_x + dx, base_y - 2, MaterialID::Wood);
        place_building_block(world, base_x + dx, base_y - 3, MaterialID::Wood);
    }

    // Window (glass in back wall)
    if ((seed & 1) != 0) {
        place_building_block(world, base_x + 2, base_y - 2, MaterialID::Glass);
        place_building_block(world, base_x + 3, base_y - 2, MaterialID::Glass);
    }

    // Peaked wooden roof
    for (int dx = -1; dx <= 6; dx++) {
        place_building_block(world, base_x + dx, base_y - 4, MaterialID::Wood);
    }
    for (int dx = 0; dx <= 5; dx++) {
        place_building_block(world, base_x + dx, base_y - 5, MaterialID::Wood);
    }
    for (int dx = 1; dx <= 4; dx++) {
        place_building_block(world, base_x + dx, base_y - 6, MaterialID::Wood);
    }
    // Roof peak
    place_building_block(world, base_x + 2, base_y - 7, MaterialID::Wood);
    place_building_block(world, base_x + 3, base_y - 7, MaterialID::Wood);

    // Chimney on side (brick)
    if ((seed & 2) != 0) {
        place_building_block(world, base_x + 5, base_y - 5, MaterialID::Brick);
        place_building_block(world, base_x + 5, base_y - 6, MaterialID::Brick);
        place_building_block(world, base_x + 5, base_y - 7, MaterialID::Brick);
    }
}

// Build a stone house with windows (9 wide, 7 tall)
static void build_stone_house(World& world, int32_t base_x, int32_t base_y, uint32_t seed) {
    // Double-thick stone foundation
    for (int dx = 0; dx < 9; dx++) {
        place_building_block(world, base_x + dx, base_y, MaterialID::Stone);
        place_building_block(world, base_x + dx, base_y + 1, MaterialID::Stone);
    }

    // Brick walls (5 tall)
    for (int dy = 1; dy <= 5; dy++) {
        place_building_block(world, base_x, base_y - dy, MaterialID::Brick);
        place_building_block(world, base_x + 8, base_y - dy, MaterialID::Brick);
    }

    // Front/back walls with windows
    for (int dx = 1; dx < 8; dx++) {
        for (int dy = 1; dy <= 5; dy++) {
            // Door opening (middle 2 blocks, bottom 3 rows)
            if ((dx == 3 || dx == 4) && dy <= 3) continue;
            // Window openings
            bool is_window = (dy == 3 || dy == 4) && (dx == 1 || dx == 2 || dx == 5 || dx == 6);
            if (is_window) {
                place_building_block(world, base_x + dx, base_y - dy, MaterialID::Glass);
            } else {
                place_building_block(world, base_x + dx, base_y - dy, MaterialID::Brick);
            }
        }
    }

    // Flat stone roof with overhang
    for (int dx = -1; dx <= 9; dx++) {
        place_building_block(world, base_x + dx, base_y - 6, MaterialID::Stone);
    }

    // Optional second floor indicator (darker brick trim)
    if ((seed & 4) != 0) {
        for (int dx = 0; dx <= 8; dx++) {
            place_building_block(world, base_x + dx, base_y - 7, MaterialID::Obsidian);
        }
    }
}

// Build a watchtower (5 wide, 12 tall)
static void build_watchtower(World& world, int32_t base_x, int32_t base_y, uint32_t seed) {
    (void)seed;

    // Wide stone base
    for (int dx = -1; dx <= 5; dx++) {
        place_building_block(world, base_x + dx, base_y, MaterialID::Stone);
    }

    // Tower body (10 tall)
    for (int dy = 1; dy <= 10; dy++) {
        place_building_block(world, base_x, base_y - dy, MaterialID::Stone);
        place_building_block(world, base_x + 4, base_y - dy, MaterialID::Stone);

        // Fill middle with stone, leave windows every 3 levels
        bool is_window_level = (dy % 3 == 0);
        if (!is_window_level) {
            place_building_block(world, base_x + 1, base_y - dy, MaterialID::Stone);
            place_building_block(world, base_x + 2, base_y - dy, MaterialID::Stone);
            place_building_block(world, base_x + 3, base_y - dy, MaterialID::Stone);
        } else {
            // Windows
            place_building_block(world, base_x + 1, base_y - dy, MaterialID::Glass);
            place_building_block(world, base_x + 3, base_y - dy, MaterialID::Glass);
        }
    }

    // Door at ground level
    // (leave base_x+2, base_y-1 and base_y-2 empty)

    // Crenellated top
    for (int dx = -1; dx <= 5; dx++) {
        place_building_block(world, base_x + dx, base_y - 11, MaterialID::Stone);
    }
    // Merlons (alternating)
    place_building_block(world, base_x - 1, base_y - 12, MaterialID::Stone);
    place_building_block(world, base_x + 1, base_y - 12, MaterialID::Stone);
    place_building_block(world, base_x + 3, base_y - 12, MaterialID::Stone);
    place_building_block(world, base_x + 5, base_y - 12, MaterialID::Stone);
}

// Build a barn (12 wide, 8 tall)
static void build_barn(World& world, int32_t base_x, int32_t base_y, uint32_t seed) {
    (void)seed;

    // Stone foundation
    for (int dx = 0; dx < 12; dx++) {
        place_building_block(world, base_x + dx, base_y, MaterialID::Stone);
    }

    // Wooden walls (5 tall)
    for (int dy = 1; dy <= 5; dy++) {
        place_building_block(world, base_x, base_y - dy, MaterialID::Wood);
        place_building_block(world, base_x + 11, base_y - dy, MaterialID::Wood);
        // Partial front wall with large door opening
        if (dy > 3) {
            for (int dx = 1; dx < 11; dx++) {
                place_building_block(world, base_x + dx, base_y - dy, MaterialID::Wood);
            }
        } else {
            // Leave big barn door in middle (5 wide)
            for (int dx = 1; dx < 4; dx++) {
                place_building_block(world, base_x + dx, base_y - dy, MaterialID::Wood);
            }
            for (int dx = 8; dx < 11; dx++) {
                place_building_block(world, base_x + dx, base_y - dy, MaterialID::Wood);
            }
        }
    }

    // Gambrel roof (barn-style)
    for (int dx = 0; dx < 12; dx++) {
        place_building_block(world, base_x + dx, base_y - 6, MaterialID::Wood);
    }
    for (int dx = 1; dx < 11; dx++) {
        place_building_block(world, base_x + dx, base_y - 7, MaterialID::Wood);
    }
    for (int dx = 3; dx < 9; dx++) {
        place_building_block(world, base_x + dx, base_y - 8, MaterialID::Wood);
    }

    // Hay loft window
    place_building_block(world, base_x + 5, base_y - 7, MaterialID::Empty);
    place_building_block(world, base_x + 6, base_y - 7, MaterialID::Empty);
}

// Build a village well (5 wide, 4 tall)
static void build_well(World& world, int32_t base_x, int32_t base_y, uint32_t seed) {
    (void)seed;

    // Stone base ring
    for (int dx = 0; dx < 5; dx++) {
        place_building_block(world, base_x + dx, base_y, MaterialID::Stone);
    }

    // Well walls (3 tall ring)
    for (int dy = 1; dy <= 3; dy++) {
        place_building_block(world, base_x, base_y - dy, MaterialID::Stone);
        place_building_block(world, base_x + 4, base_y - dy, MaterialID::Stone);
        // Only place edges for the ring, leave inside for "water" view
        if (dy == 3) {
            place_building_block(world, base_x + 1, base_y - dy, MaterialID::Stone);
            place_building_block(world, base_x + 2, base_y - dy, MaterialID::Stone);
            place_building_block(world, base_x + 3, base_y - dy, MaterialID::Stone);
        }
    }

    // Well posts (wood)
    place_building_block(world, base_x, base_y - 4, MaterialID::Wood);
    place_building_block(world, base_x + 4, base_y - 4, MaterialID::Wood);
    place_building_block(world, base_x, base_y - 5, MaterialID::Wood);
    place_building_block(world, base_x + 4, base_y - 5, MaterialID::Wood);

    // Roof beam
    for (int dx = 0; dx < 5; dx++) {
        place_building_block(world, base_x + dx, base_y - 6, MaterialID::Wood);
    }
}

// Build a wooden bridge/platform
static void build_bridge(World& world, int32_t base_x, int32_t base_y, uint32_t seed) {
    int length = 8 + (seed & 7);  // 8-15 long

    // Main platform
    for (int dx = 0; dx < length; dx++) {
        place_building_block(world, base_x + dx, base_y, MaterialID::Wood);
    }

    // Railings
    for (int dx = 0; dx < length; dx += 2) {
        place_building_block(world, base_x + dx, base_y - 1, MaterialID::Wood);
    }

    // Support posts at ends
    for (int dy = 1; dy <= 3; dy++) {
        place_building_block(world, base_x, base_y + dy, MaterialID::Wood);
        place_building_block(world, base_x + length - 1, base_y + dy, MaterialID::Wood);
    }
}

// Build a simple fence line
static void build_fence(World& world, int32_t base_x, int32_t base_y, uint32_t seed) {
    int length = 6 + (seed & 7);  // 6-13 long

    for (int dx = 0; dx < length; dx++) {
        place_building_block(world, base_x + dx, base_y, MaterialID::Wood);
        // Fence posts every 3 blocks
        if (dx % 3 == 0) {
            place_building_block(world, base_x + dx, base_y - 1, MaterialID::Wood);
            place_building_block(world, base_x + dx, base_y - 2, MaterialID::Wood);
        } else {
            place_building_block(world, base_x + dx, base_y - 1, MaterialID::Wood);
        }
    }
}

// Build a decorative shrine (4 wide, 6 tall)
static void build_shrine(World& world, int32_t base_x, int32_t base_y, uint32_t seed) {
    (void)seed;

    // Stone platform
    for (int dx = -1; dx <= 4; dx++) {
        place_building_block(world, base_x + dx, base_y, MaterialID::Stone);
    }

    // Pillars
    for (int dy = 1; dy <= 4; dy++) {
        place_building_block(world, base_x, base_y - dy, MaterialID::Stone);
        place_building_block(world, base_x + 3, base_y - dy, MaterialID::Stone);
    }

    // Roof
    for (int dx = -1; dx <= 4; dx++) {
        place_building_block(world, base_x + dx, base_y - 5, MaterialID::Stone);
    }
    place_building_block(world, base_x + 1, base_y - 6, MaterialID::Stone);
    place_building_block(world, base_x + 2, base_y - 6, MaterialID::Stone);

    // Crystal centerpiece
    place_building_block(world, base_x + 1, base_y - 1, MaterialID::Crystal);
    place_building_block(world, base_x + 2, base_y - 1, MaterialID::Crystal);
    place_building_block(world, base_x + 1, base_y - 2, MaterialID::Crystal);
    place_building_block(world, base_x + 2, base_y - 2, MaterialID::Crystal);
}

// Build a windmill (6 wide, 14 tall)
static void build_windmill(World& world, int32_t base_x, int32_t base_y, uint32_t seed) {
    (void)seed;

    // Stone foundation
    for (int dx = 0; dx < 6; dx++) {
        place_building_block(world, base_x + dx, base_y, MaterialID::Stone);
    }

    // Tapered brick tower
    for (int dy = 1; dy <= 4; dy++) {
        for (int dx = 0; dx < 6; dx++) {
            if (dx == 2 || dx == 3) {
                // Door at ground, windows higher
                if (dy > 2) {
                    place_building_block(world, base_x + dx, base_y - dy, MaterialID::Brick);
                }
            } else {
                place_building_block(world, base_x + dx, base_y - dy, MaterialID::Brick);
            }
        }
    }

    // Narrower upper section
    for (int dy = 5; dy <= 8; dy++) {
        place_building_block(world, base_x + 1, base_y - dy, MaterialID::Brick);
        place_building_block(world, base_x + 4, base_y - dy, MaterialID::Brick);
        // Window in middle
        if (dy != 6 && dy != 7) {
            place_building_block(world, base_x + 2, base_y - dy, MaterialID::Brick);
            place_building_block(world, base_x + 3, base_y - dy, MaterialID::Brick);
        } else {
            place_building_block(world, base_x + 2, base_y - dy, MaterialID::Glass);
            place_building_block(world, base_x + 3, base_y - dy, MaterialID::Glass);
        }
    }

    // Top cap
    for (int dx = 1; dx <= 4; dx++) {
        place_building_block(world, base_x + dx, base_y - 9, MaterialID::Wood);
    }
    place_building_block(world, base_x + 2, base_y - 10, MaterialID::Wood);
    place_building_block(world, base_x + 3, base_y - 10, MaterialID::Wood);

    // Windmill blades (simple cross pattern with wood)
    // Horizontal blade
    for (int dx = -3; dx <= 8; dx++) {
        place_building_block(world, base_x + dx, base_y - 7, MaterialID::Wood);
    }
    // Vertical blade
    for (int dy = 4; dy <= 10; dy++) {
        place_building_block(world, base_x + 2, base_y - dy, MaterialID::Wood);
        place_building_block(world, base_x + 3, base_y - dy, MaterialID::Wood);
    }
}

// Build a two-story inn (14 wide, 10 tall)
static void build_inn(World& world, int32_t base_x, int32_t base_y, uint32_t seed) {
    (void)seed;

    // Large stone foundation
    for (int dx = 0; dx < 14; dx++) {
        place_building_block(world, base_x + dx, base_y, MaterialID::Stone);
    }

    // First floor walls (brick)
    for (int dy = 1; dy <= 4; dy++) {
        place_building_block(world, base_x, base_y - dy, MaterialID::Brick);
        place_building_block(world, base_x + 13, base_y - dy, MaterialID::Brick);
        for (int dx = 1; dx < 13; dx++) {
            // Main door (3 wide)
            if ((dx >= 5 && dx <= 7) && dy <= 3) continue;
            // Windows
            bool is_window = (dy == 2 || dy == 3) && (dx == 2 || dx == 3 || dx == 10 || dx == 11);
            if (is_window) {
                place_building_block(world, base_x + dx, base_y - dy, MaterialID::Glass);
            } else {
                place_building_block(world, base_x + dx, base_y - dy, MaterialID::Brick);
            }
        }
    }

    // Second floor (slightly inset)
    for (int dy = 5; dy <= 7; dy++) {
        place_building_block(world, base_x + 1, base_y - dy, MaterialID::Wood);
        place_building_block(world, base_x + 12, base_y - dy, MaterialID::Wood);
        for (int dx = 2; dx < 12; dx++) {
            // Windows on second floor
            bool is_window = (dy == 5 || dy == 6) && (dx == 3 || dx == 4 || dx == 9 || dx == 10);
            if (is_window) {
                place_building_block(world, base_x + dx, base_y - dy, MaterialID::Glass);
            } else {
                place_building_block(world, base_x + dx, base_y - dy, MaterialID::Wood);
            }
        }
    }

    // Floor separator (stone trim)
    for (int dx = 0; dx < 14; dx++) {
        place_building_block(world, base_x + dx, base_y - 5, MaterialID::Stone);
    }

    // Peaked roof
    for (int level = 0; level < 3; level++) {
        int start = level;
        int end = 14 - level;
        for (int dx = start; dx < end; dx++) {
            place_building_block(world, base_x + dx, base_y - 8 - level, MaterialID::Wood);
        }
    }

    // Chimney
    place_building_block(world, base_x + 11, base_y - 9, MaterialID::Brick);
    place_building_block(world, base_x + 11, base_y - 10, MaterialID::Brick);
    place_building_block(world, base_x + 11, base_y - 11, MaterialID::Brick);

    // Sign post (copper decoration)
    place_building_block(world, base_x + 4, base_y - 4, MaterialID::Copper);
    place_building_block(world, base_x + 3, base_y - 4, MaterialID::Copper);
}

// ============================================================================
// ADVANCED STRUCTURES - Complex multi-level buildings with climbable features
// ============================================================================

// Build a grand castle with towers and battlements (25 wide, 20 tall)
static void build_castle(World& world, int32_t base_x, int32_t base_y, uint32_t seed) {
    (void)seed;

    // Main foundation
    for (int dx = 0; dx < 25; dx++) {
        place_building_block(world, base_x + dx, base_y, MaterialID::Stone);
        place_building_block(world, base_x + dx, base_y + 1, MaterialID::Stone);
    }

    // Main castle walls (10 tall)
    for (int dy = 1; dy <= 10; dy++) {
        // Outer walls
        place_building_block(world, base_x, base_y - dy, MaterialID::Stone);
        place_building_block(world, base_x + 24, base_y - dy, MaterialID::Stone);

        // Fill walls with windows
        for (int dx = 1; dx < 24; dx++) {
            // Gate opening (middle 5 blocks, bottom 5 rows)
            if (dx >= 10 && dx <= 14 && dy <= 5) continue;
            // Arrow slit windows
            bool is_window = (dy == 6 || dy == 8) && (dx % 4 == 2);
            if (is_window) {
                place_building_block(world, base_x + dx, base_y - dy, MaterialID::Glass);
            } else {
                place_building_block(world, base_x + dx, base_y - dy, MaterialID::Stone);
            }
        }
    }

    // Battlements/crenellations on top
    for (int dx = 0; dx < 25; dx += 2) {
        place_building_block(world, base_x + dx, base_y - 11, MaterialID::Stone);
        place_building_block(world, base_x + dx, base_y - 12, MaterialID::Stone);
    }

    // Left tower (taller)
    for (int dy = 10; dy <= 16; dy++) {
        for (int dx = 0; dx < 5; dx++) {
            place_building_block(world, base_x + dx, base_y - dy, MaterialID::Stone);
        }
    }
    // Tower crenellations
    for (int dx = 0; dx < 5; dx += 2) {
        place_building_block(world, base_x + dx, base_y - 17, MaterialID::Stone);
        place_building_block(world, base_x + dx, base_y - 18, MaterialID::Stone);
    }

    // Right tower
    for (int dy = 10; dy <= 16; dy++) {
        for (int dx = 20; dx < 25; dx++) {
            place_building_block(world, base_x + dx, base_y - dy, MaterialID::Stone);
        }
    }
    for (int dx = 20; dx < 25; dx += 2) {
        place_building_block(world, base_x + dx, base_y - 17, MaterialID::Stone);
        place_building_block(world, base_x + dx, base_y - 18, MaterialID::Stone);
    }

    // Center keep (above gate)
    for (int dy = 10; dy <= 14; dy++) {
        for (int dx = 8; dx < 17; dx++) {
            if (dx == 12 && dy <= 12) continue;  // Keep door
            place_building_block(world, base_x + dx, base_y - dy, MaterialID::Stone);
        }
    }
    // Keep roof
    for (int level = 0; level < 3; level++) {
        for (int dx = 9 + level; dx < 16 - level; dx++) {
            place_building_block(world, base_x + dx, base_y - 15 - level, MaterialID::Wood);
        }
    }

    // Interior stairs for climbing (people can use these!)
    for (int step = 0; step < 8; step++) {
        place_building_block(world, base_x + 2 + step, base_y - 1 - step, MaterialID::Stone);
        place_building_block(world, base_x + 22 - step, base_y - 1 - step, MaterialID::Stone);
    }

    // Walkway inside castle walls
    for (int dx = 4; dx < 21; dx++) {
        place_building_block(world, base_x + dx, base_y - 8, MaterialID::Stone);
    }
}

// Build a church with tall steeple (10 wide, 18 tall)
static void build_church(World& world, int32_t base_x, int32_t base_y, uint32_t seed) {
    (void)seed;

    // Foundation
    for (int dx = 0; dx < 10; dx++) {
        place_building_block(world, base_x + dx, base_y, MaterialID::Stone);
    }

    // Main building walls (6 tall)
    for (int dy = 1; dy <= 6; dy++) {
        place_building_block(world, base_x, base_y - dy, MaterialID::Stone);
        place_building_block(world, base_x + 9, base_y - dy, MaterialID::Stone);
        for (int dx = 1; dx < 9; dx++) {
            // Doorway
            if ((dx == 4 || dx == 5) && dy <= 3) continue;
            // Stained glass windows
            bool is_window = (dy >= 3 && dy <= 5) && (dx == 2 || dx == 7);
            if (is_window) {
                place_building_block(world, base_x + dx, base_y - dy, MaterialID::Crystal);  // Colorful!
            } else {
                place_building_block(world, base_x + dx, base_y - dy, MaterialID::Stone);
            }
        }
    }

    // Peaked roof
    for (int level = 0; level < 4; level++) {
        for (int dx = level; dx < 10 - level; dx++) {
            place_building_block(world, base_x + dx, base_y - 7 - level, MaterialID::Wood);
        }
    }

    // Steeple/bell tower (centered)
    for (int dy = 11; dy <= 16; dy++) {
        place_building_block(world, base_x + 4, base_y - dy, MaterialID::Stone);
        place_building_block(world, base_x + 5, base_y - dy, MaterialID::Stone);
    }
    // Steeple point
    place_building_block(world, base_x + 4, base_y - 17, MaterialID::Copper);
    place_building_block(world, base_x + 5, base_y - 17, MaterialID::Copper);
    place_building_block(world, base_x + 4, base_y - 18, MaterialID::Gold);  // Cross top

    // Bell opening
    place_building_block(world, base_x + 4, base_y - 14, MaterialID::Empty);
    place_building_block(world, base_x + 5, base_y - 14, MaterialID::Empty);
}

// Build market stalls (18 wide, 6 tall)
static void build_market(World& world, int32_t base_x, int32_t base_y, uint32_t seed) {
    // 3 connected stalls
    for (int stall = 0; stall < 3; stall++) {
        int sx = base_x + stall * 6;

        // Counter
        for (int dx = 0; dx < 5; dx++) {
            place_building_block(world, sx + dx, base_y, MaterialID::Wood);
            place_building_block(world, sx + dx, base_y - 1, MaterialID::Wood);
        }

        // Support posts
        place_building_block(world, sx, base_y - 2, MaterialID::Wood);
        place_building_block(world, sx, base_y - 3, MaterialID::Wood);
        place_building_block(world, sx + 4, base_y - 2, MaterialID::Wood);
        place_building_block(world, sx + 4, base_y - 3, MaterialID::Wood);

        // Awning/roof
        MaterialID awning = (stall == 0) ? MaterialID::Brick :
                           (stall == 1) ? MaterialID::Wood : MaterialID::Stone;
        for (int dx = -1; dx < 6; dx++) {
            place_building_block(world, sx + dx, base_y - 4, awning);
        }

        // Goods display (varied by seed)
        MaterialID goods = ((seed >> stall) & 3) == 0 ? MaterialID::Crystal :
                          ((seed >> stall) & 3) == 1 ? MaterialID::Gold :
                          ((seed >> stall) & 3) == 2 ? MaterialID::Copper : MaterialID::Glass;
        place_building_block(world, sx + 1, base_y - 2, goods);
        place_building_block(world, sx + 3, base_y - 2, goods);
    }
}

// Build a lighthouse (6 wide, 22 tall)
static void build_lighthouse(World& world, int32_t base_x, int32_t base_y, uint32_t seed) {
    (void)seed;

    // Wide stone base
    for (int dx = -1; dx <= 6; dx++) {
        place_building_block(world, base_x + dx, base_y, MaterialID::Stone);
        place_building_block(world, base_x + dx, base_y + 1, MaterialID::Stone);
    }

    // Tapered tower - starts 6 wide, narrows to 4
    for (int dy = 1; dy <= 6; dy++) {
        for (int dx = 0; dx < 6; dx++) {
            if (dx == 2 || dx == 3) {
                if (dy <= 3) continue;  // Door
            }
            place_building_block(world, base_x + dx, base_y - dy, MaterialID::Brick);
        }
    }

    // Middle section (narrower)
    for (int dy = 7; dy <= 14; dy++) {
        place_building_block(world, base_x + 1, base_y - dy, MaterialID::Brick);
        place_building_block(world, base_x + 4, base_y - dy, MaterialID::Brick);
        // Spiral windows
        bool is_window = ((dy - 7) % 3 == 1);
        if (!is_window) {
            place_building_block(world, base_x + 2, base_y - dy, MaterialID::Brick);
            place_building_block(world, base_x + 3, base_y - dy, MaterialID::Brick);
        } else {
            place_building_block(world, base_x + 2, base_y - dy, MaterialID::Glass);
            place_building_block(world, base_x + 3, base_y - dy, MaterialID::Glass);
        }
    }

    // Light chamber
    for (int dx = 0; dx < 6; dx++) {
        place_building_block(world, base_x + dx, base_y - 15, MaterialID::Stone);
    }
    // Glass walls of light chamber
    for (int dy = 16; dy <= 18; dy++) {
        place_building_block(world, base_x + 1, base_y - dy, MaterialID::Glass);
        place_building_block(world, base_x + 4, base_y - dy, MaterialID::Glass);
        place_building_block(world, base_x + 2, base_y - dy, MaterialID::Glass);
        place_building_block(world, base_x + 3, base_y - dy, MaterialID::Glass);
    }

    // Roof cap
    for (int dx = 0; dx < 6; dx++) {
        place_building_block(world, base_x + dx, base_y - 19, MaterialID::Copper);
    }
    place_building_block(world, base_x + 2, base_y - 20, MaterialID::Copper);
    place_building_block(world, base_x + 3, base_y - 20, MaterialID::Copper);
    place_building_block(world, base_x + 2, base_y - 21, MaterialID::Gold);  // Light!
    place_building_block(world, base_x + 3, base_y - 21, MaterialID::Gold);

    // Interior stairs for climbing
    for (int step = 0; step < 5; step++) {
        place_building_block(world, base_x + 2, base_y - 1 - step, MaterialID::Stone);
    }
}

// Build a small tavern (8 wide, 5 tall)
static void build_tavern(World& world, int32_t base_x, int32_t base_y, uint32_t seed) {
    (void)seed;

    // Foundation
    for (int dx = 0; dx < 8; dx++) {
        place_building_block(world, base_x + dx, base_y, MaterialID::Stone);
    }

    // Walls
    for (int dy = 1; dy <= 4; dy++) {
        place_building_block(world, base_x, base_y - dy, MaterialID::Wood);
        place_building_block(world, base_x + 7, base_y - dy, MaterialID::Wood);
        for (int dx = 1; dx < 7; dx++) {
            if ((dx == 3 || dx == 4) && dy <= 2) continue;  // Door
            bool is_window = (dy == 2 || dy == 3) && (dx == 1 || dx == 6);
            if (is_window) {
                place_building_block(world, base_x + dx, base_y - dy, MaterialID::Glass);
            } else {
                place_building_block(world, base_x + dx, base_y - dy, MaterialID::Wood);
            }
        }
    }

    // Flat roof with overhang
    for (int dx = -1; dx <= 8; dx++) {
        place_building_block(world, base_x + dx, base_y - 5, MaterialID::Wood);
    }

    // Sign (like a pub sign)
    place_building_block(world, base_x - 1, base_y - 3, MaterialID::Wood);
    place_building_block(world, base_x - 1, base_y - 4, MaterialID::Copper);
}

// Build climbable stairs (8 wide, variable height)
static void build_stairs(World& world, int32_t base_x, int32_t base_y, uint32_t seed) {
    int height = 6 + (seed & 7);  // 6-13 steps

    for (int step = 0; step < height; step++) {
        // Each step is 1 wide, going up and right
        for (int w = 0; w < 2; w++) {  // 2 blocks wide for easier climbing
            place_building_block(world, base_x + step, base_y - step, MaterialID::Stone);
            // Add railing on one side
            if (step % 2 == 0) {
                place_building_block(world, base_x + step, base_y - step - 1, MaterialID::Wood);
            }
        }
    }

    // Platform at top
    for (int dx = 0; dx < 4; dx++) {
        place_building_block(world, base_x + height + dx - 1, base_y - height + 1, MaterialID::Stone);
    }
}

// Build a vertical ladder (2 wide, variable height)
static void build_ladder(World& world, int32_t base_x, int32_t base_y, uint32_t seed) {
    int height = 8 + (seed & 7);  // 8-15 tall

    // Ladder rails
    for (int dy = 0; dy < height; dy++) {
        place_building_block(world, base_x, base_y - dy, MaterialID::Wood);
        place_building_block(world, base_x + 1, base_y - dy, MaterialID::Wood);
    }

    // Platform at top
    for (int dx = -1; dx <= 3; dx++) {
        place_building_block(world, base_x + dx, base_y - height, MaterialID::Wood);
    }
}

// Build elevated skywalk bridge (variable length, 8 blocks up)
static void build_skywalk_bridge(World& world, int32_t base_x, int32_t base_y, uint32_t seed) {
    int length = 12 + (seed & 7);  // 12-19 long
    int elevation = 6 + (seed & 3);  // 6-9 blocks high

    // Support pillars at ends
    for (int dy = 0; dy <= elevation; dy++) {
        place_building_block(world, base_x + 1, base_y - dy, MaterialID::Stone);
        place_building_block(world, base_x + length - 2, base_y - dy, MaterialID::Stone);
    }

    // Bridge deck
    for (int dx = 0; dx < length; dx++) {
        place_building_block(world, base_x + dx, base_y - elevation, MaterialID::Wood);
    }

    // Railings
    for (int dx = 0; dx < length; dx += 2) {
        place_building_block(world, base_x + dx, base_y - elevation - 1, MaterialID::Wood);
    }

    // Stairs up on left side
    for (int step = 0; step < elevation; step++) {
        place_building_block(world, base_x - 1 - step, base_y - step, MaterialID::Stone);
    }
}

// Build a grand hall (20 wide, 12 tall)
static void build_grand_hall(World& world, int32_t base_x, int32_t base_y, uint32_t seed) {
    (void)seed;

    // Large foundation
    for (int dx = 0; dx < 20; dx++) {
        place_building_block(world, base_x + dx, base_y, MaterialID::Stone);
    }

    // Main walls
    for (int dy = 1; dy <= 8; dy++) {
        place_building_block(world, base_x, base_y - dy, MaterialID::Stone);
        place_building_block(world, base_x + 19, base_y - dy, MaterialID::Stone);

        for (int dx = 1; dx < 19; dx++) {
            // Large doorway
            if (dx >= 8 && dx <= 11 && dy <= 5) continue;
            // Tall windows
            bool is_window = (dy >= 3 && dy <= 7) && (dx == 3 || dx == 6 || dx == 13 || dx == 16);
            if (is_window) {
                place_building_block(world, base_x + dx, base_y - dy, MaterialID::Glass);
            } else {
                place_building_block(world, base_x + dx, base_y - dy, MaterialID::Stone);
            }
        }
    }

    // Arched roof
    for (int level = 0; level < 4; level++) {
        for (int dx = level; dx < 20 - level; dx++) {
            place_building_block(world, base_x + dx, base_y - 9 - level, MaterialID::Wood);
        }
    }

    // Interior pillars (people can climb between them)
    for (int pillar = 0; pillar < 3; pillar++) {
        int px = base_x + 4 + pillar * 6;
        for (int dy = 1; dy <= 7; dy++) {
            place_building_block(world, px, base_y - dy, MaterialID::Stone);
        }
    }

    // Upper walkway (for people to explore)
    for (int dx = 2; dx < 18; dx++) {
        place_building_block(world, base_x + dx, base_y - 6, MaterialID::Wood);
    }
}

// Build an observatory with dome (8 wide, 16 tall)
static void build_observatory(World& world, int32_t base_x, int32_t base_y, uint32_t seed) {
    (void)seed;

    // Foundation
    for (int dx = 0; dx < 8; dx++) {
        place_building_block(world, base_x + dx, base_y, MaterialID::Stone);
    }

    // Tower base (brick)
    for (int dy = 1; dy <= 8; dy++) {
        for (int dx = 0; dx < 8; dx++) {
            if ((dx == 3 || dx == 4) && dy <= 3) continue;  // Door
            bool is_edge = (dx == 0 || dx == 7);
            if (is_edge) {
                place_building_block(world, base_x + dx, base_y - dy, MaterialID::Brick);
            } else {
                // Windows on certain levels
                bool is_window = (dy == 5 || dy == 7) && (dx == 2 || dx == 5);
                if (is_window) {
                    place_building_block(world, base_x + dx, base_y - dy, MaterialID::Glass);
                } else {
                    place_building_block(world, base_x + dx, base_y - dy, MaterialID::Brick);
                }
            }
        }
    }

    // Observation platform
    for (int dx = -1; dx <= 8; dx++) {
        place_building_block(world, base_x + dx, base_y - 9, MaterialID::Stone);
    }

    // Dome structure
    // Bottom ring
    for (int dx = 1; dx <= 6; dx++) {
        place_building_block(world, base_x + dx, base_y - 10, MaterialID::Copper);
    }
    // Middle ring
    for (int dx = 2; dx <= 5; dx++) {
        place_building_block(world, base_x + dx, base_y - 11, MaterialID::Copper);
        place_building_block(world, base_x + dx, base_y - 12, MaterialID::Copper);
    }
    // Top
    place_building_block(world, base_x + 3, base_y - 13, MaterialID::Copper);
    place_building_block(world, base_x + 4, base_y - 13, MaterialID::Copper);
    place_building_block(world, base_x + 3, base_y - 14, MaterialID::Glass);  // Telescope opening
    place_building_block(world, base_x + 4, base_y - 14, MaterialID::Glass);

    // Crystal telescope
    place_building_block(world, base_x + 3, base_y - 15, MaterialID::Crystal);
    place_building_block(world, base_x + 4, base_y - 15, MaterialID::Crystal);

    // Interior spiral stairs
    for (int step = 0; step < 7; step++) {
        int sx = (step % 2 == 0) ? 2 : 5;
        place_building_block(world, base_x + sx, base_y - 1 - step, MaterialID::Stone);
    }
}

// Build a decorative fountain (7 wide, 5 tall)
static void build_fountain(World& world, int32_t base_x, int32_t base_y, uint32_t seed) {
    (void)seed;

    // Base pool
    for (int dx = 0; dx < 7; dx++) {
        place_building_block(world, base_x + dx, base_y, MaterialID::Stone);
        if (dx > 0 && dx < 6) {
            place_building_block(world, base_x + dx, base_y - 1, MaterialID::Stone);
        }
    }
    // Pool walls
    place_building_block(world, base_x, base_y - 1, MaterialID::Stone);
    place_building_block(world, base_x, base_y - 2, MaterialID::Stone);
    place_building_block(world, base_x + 6, base_y - 1, MaterialID::Stone);
    place_building_block(world, base_x + 6, base_y - 2, MaterialID::Stone);

    // Center pillar
    place_building_block(world, base_x + 3, base_y - 1, MaterialID::Stone);
    place_building_block(world, base_x + 3, base_y - 2, MaterialID::Stone);
    place_building_block(world, base_x + 3, base_y - 3, MaterialID::Stone);

    // Top basin
    place_building_block(world, base_x + 2, base_y - 4, MaterialID::Copper);
    place_building_block(world, base_x + 3, base_y - 4, MaterialID::Copper);
    place_building_block(world, base_x + 4, base_y - 4, MaterialID::Copper);
}

// Build a statue (4 wide, 8 tall)
static void build_statue(World& world, int32_t base_x, int32_t base_y, uint32_t seed) {
    // Pedestal
    for (int dx = 0; dx < 4; dx++) {
        place_building_block(world, base_x + dx, base_y, MaterialID::Stone);
        place_building_block(world, base_x + dx, base_y - 1, MaterialID::Stone);
    }
    place_building_block(world, base_x + 1, base_y - 2, MaterialID::Stone);
    place_building_block(world, base_x + 2, base_y - 2, MaterialID::Stone);

    // Figure (varied material by seed)
    MaterialID statue_mat = (seed & 1) ? MaterialID::Copper : MaterialID::Stone;

    // Body
    place_building_block(world, base_x + 1, base_y - 3, statue_mat);
    place_building_block(world, base_x + 2, base_y - 3, statue_mat);
    place_building_block(world, base_x + 1, base_y - 4, statue_mat);
    place_building_block(world, base_x + 2, base_y - 4, statue_mat);
    place_building_block(world, base_x + 1, base_y - 5, statue_mat);
    place_building_block(world, base_x + 2, base_y - 5, statue_mat);

    // Head
    place_building_block(world, base_x + 1, base_y - 6, statue_mat);
    place_building_block(world, base_x + 2, base_y - 6, statue_mat);
    place_building_block(world, base_x + 1, base_y - 7, statue_mat);
    place_building_block(world, base_x + 2, base_y - 7, statue_mat);

    // Arms (extended)
    if (seed & 2) {
        place_building_block(world, base_x, base_y - 5, statue_mat);
        place_building_block(world, base_x + 3, base_y - 5, statue_mat);
    }
}

// Build a walled garden (12 wide, 4 tall)
static void build_garden(World& world, int32_t base_x, int32_t base_y, uint32_t seed) {
    // Stone walls
    for (int dx = 0; dx < 12; dx++) {
        place_building_block(world, base_x + dx, base_y, MaterialID::Stone);
        if (dx == 0 || dx == 11 || dx == 5 || dx == 6) {
            // Wall posts taller
            place_building_block(world, base_x + dx, base_y - 1, MaterialID::Stone);
            place_building_block(world, base_x + dx, base_y - 2, MaterialID::Stone);
        }
    }

    // Gate opening in middle (already empty)

    // Plants inside
    for (int dx = 1; dx < 5; dx++) {
        MaterialID plant = ((seed + dx) & 3) == 0 ? MaterialID::Flower :
                          ((seed + dx) & 3) == 1 ? MaterialID::Moss :
                          ((seed + dx) & 3) == 2 ? MaterialID::Grass : MaterialID::Vine;
        place_building_block(world, base_x + dx, base_y - 1, plant);
    }
    for (int dx = 7; dx < 11; dx++) {
        MaterialID plant = ((seed + dx) & 3) == 0 ? MaterialID::Flower :
                          ((seed + dx) & 3) == 1 ? MaterialID::Moss :
                          ((seed + dx) & 3) == 2 ? MaterialID::Grass : MaterialID::Vine;
        place_building_block(world, base_x + dx, base_y - 1, plant);
    }

    // Tree in center of each half
    place_building_block(world, base_x + 2, base_y - 2, MaterialID::Wood);
    place_building_block(world, base_x + 2, base_y - 3, MaterialID::Leaf);
    place_building_block(world, base_x + 9, base_y - 2, MaterialID::Wood);
    place_building_block(world, base_x + 9, base_y - 3, MaterialID::Leaf);
}

// Build a wooden dock (variable length, 3 tall)
static void build_dock(World& world, int32_t base_x, int32_t base_y, uint32_t seed) {
    int length = 10 + (seed & 7);  // 10-17 long

    // Support posts going down
    for (int dx = 0; dx < length; dx += 3) {
        for (int dy = 0; dy < 4; dy++) {
            place_building_block(world, base_x + dx, base_y + dy, MaterialID::Wood);
        }
    }

    // Deck planks
    for (int dx = 0; dx < length; dx++) {
        place_building_block(world, base_x + dx, base_y, MaterialID::Wood);
    }

    // End mooring post
    place_building_block(world, base_x + length - 1, base_y - 1, MaterialID::Wood);
    place_building_block(world, base_x + length - 1, base_y - 2, MaterialID::Wood);
}

// Build a simple climbable tower with platforms (5 wide, 12 tall)
static void build_tower(World& world, int32_t base_x, int32_t base_y, uint32_t seed) {
    (void)seed;

    // Foundation
    for (int dx = 0; dx < 5; dx++) {
        place_building_block(world, base_x + dx, base_y, MaterialID::Stone);
    }

    // Tower walls with interior platforms for climbing
    for (int dy = 1; dy <= 10; dy++) {
        place_building_block(world, base_x, base_y - dy, MaterialID::Stone);
        place_building_block(world, base_x + 4, base_y - dy, MaterialID::Stone);

        // Interior platforms every 3 levels (people can climb these!)
        if (dy % 3 == 0) {
            for (int dx = 1; dx < 4; dx++) {
                place_building_block(world, base_x + dx, base_y - dy, MaterialID::Wood);
            }
        }
    }

    // Top platform
    for (int dx = 0; dx < 5; dx++) {
        place_building_block(world, base_x + dx, base_y - 11, MaterialID::Stone);
    }
    // Crenellations
    place_building_block(world, base_x, base_y - 12, MaterialID::Stone);
    place_building_block(world, base_x + 2, base_y - 12, MaterialID::Stone);
    place_building_block(world, base_x + 4, base_y - 12, MaterialID::Stone);
}

// ============================================================================
// VERTICAL STRUCTURES - Super climbable multi-level buildings!
// ============================================================================

// Build a spiral tower with wrap-around stairs (6 wide, 20 tall)
static void build_spiral_tower(World& world, int32_t base_x, int32_t base_y, uint32_t seed) {
    (void)seed;
    int height = 18;

    // Foundation
    for (int dx = 0; dx < 6; dx++) {
        place_building_block(world, base_x + dx, base_y, MaterialID::Stone);
    }

    // Outer walls
    for (int dy = 1; dy <= height; dy++) {
        place_building_block(world, base_x, base_y - dy, MaterialID::Brick);
        place_building_block(world, base_x + 5, base_y - dy, MaterialID::Brick);
    }

    // Spiral stairs inside - platforms on alternating sides each level
    for (int level = 0; level < height / 2; level++) {
        int level_y = base_y - 2 - (level * 2);
        if (level % 2 == 0) {
            // Platforms on left
            for (int dx = 1; dx <= 3; dx++) {
                place_building_block(world, base_x + dx, level_y, MaterialID::Wood);
            }
        } else {
            // Platforms on right
            for (int dx = 2; dx <= 4; dx++) {
                place_building_block(world, base_x + dx, level_y, MaterialID::Wood);
            }
        }
    }

    // Top observation deck
    for (int dx = -1; dx <= 6; dx++) {
        place_building_block(world, base_x + dx, base_y - height - 1, MaterialID::Stone);
    }
    // Railings
    place_building_block(world, base_x - 1, base_y - height - 2, MaterialID::Wood);
    place_building_block(world, base_x + 6, base_y - height - 2, MaterialID::Wood);
}

// Build construction scaffolding (8 wide, 15 tall) - super easy to climb!
static void build_scaffolding(World& world, int32_t base_x, int32_t base_y, uint32_t seed) {
    (void)seed;
    int height = 14;

    // Vertical supports
    for (int dy = 0; dy <= height; dy++) {
        place_building_block(world, base_x, base_y - dy, MaterialID::Wood);
        place_building_block(world, base_x + 3, base_y - dy, MaterialID::Wood);
        place_building_block(world, base_x + 6, base_y - dy, MaterialID::Wood);
    }

    // Horizontal platforms every 2 levels (very easy climbing!)
    for (int level = 0; level <= height; level += 2) {
        for (int dx = 0; dx <= 6; dx++) {
            place_building_block(world, base_x + dx, base_y - level, MaterialID::Wood);
        }
    }

    // Cross bracing for visual interest
    for (int level = 1; level < height; level += 4) {
        place_building_block(world, base_x + 1, base_y - level, MaterialID::Wood);
        place_building_block(world, base_x + 2, base_y - level - 1, MaterialID::Wood);
        place_building_block(world, base_x + 4, base_y - level, MaterialID::Wood);
        place_building_block(world, base_x + 5, base_y - level - 1, MaterialID::Wood);
    }
}

// Build a skyscraper (10 wide, 25 tall) with many floors
static void build_skyscraper(World& world, int32_t base_x, int32_t base_y, uint32_t seed) {
    (void)seed;
    int floors = 6;
    int floor_height = 4;
    int total_height = floors * floor_height;

    // Foundation
    for (int dx = 0; dx < 10; dx++) {
        place_building_block(world, base_x + dx, base_y, MaterialID::Stone);
        place_building_block(world, base_x + dx, base_y + 1, MaterialID::Stone);
    }

    // Build each floor
    for (int floor = 0; floor < floors; floor++) {
        int floor_base = base_y - (floor * floor_height);

        // Walls
        for (int dy = 1; dy <= floor_height; dy++) {
            place_building_block(world, base_x, floor_base - dy, MaterialID::Brick);
            place_building_block(world, base_x + 9, floor_base - dy, MaterialID::Brick);

            // Windows on every floor
            if (dy == 2 || dy == 3) {
                place_building_block(world, base_x + 2, floor_base - dy, MaterialID::Glass);
                place_building_block(world, base_x + 3, floor_base - dy, MaterialID::Glass);
                place_building_block(world, base_x + 6, floor_base - dy, MaterialID::Glass);
                place_building_block(world, base_x + 7, floor_base - dy, MaterialID::Glass);
            }
        }

        // Floor/ceiling
        for (int dx = 1; dx < 9; dx++) {
            place_building_block(world, base_x + dx, floor_base - floor_height, MaterialID::Stone);
        }

        // Internal stairs on alternating sides
        if (floor % 2 == 0) {
            for (int step = 0; step < 3; step++) {
                place_building_block(world, base_x + 1 + step, floor_base - 1 - step, MaterialID::Stone);
            }
        } else {
            for (int step = 0; step < 3; step++) {
                place_building_block(world, base_x + 8 - step, floor_base - 1 - step, MaterialID::Stone);
            }
        }
    }

    // Rooftop
    for (int dx = 0; dx < 10; dx++) {
        place_building_block(world, base_x + dx, base_y - total_height - 1, MaterialID::Metal);
    }
    // Antenna
    for (int dy = 0; dy < 4; dy++) {
        place_building_block(world, base_x + 5, base_y - total_height - 2 - dy, MaterialID::Metal);
    }
}

// Build a dense climbing wall (4 wide, 12 tall) - platforms everywhere!
static void build_climbing_wall(World& world, int32_t base_x, int32_t base_y, uint32_t seed) {
    int height = 10 + (seed & 3);

    // Dense grid of handholds
    for (int dy = 0; dy <= height; dy++) {
        for (int dx = 0; dx < 4; dx++) {
            // Checkerboard pattern for easy climbing
            if ((dx + dy) % 2 == 0) {
                place_building_block(world, base_x + dx, base_y - dy, MaterialID::Stone);
            }
        }
    }

    // Platform at top
    for (int dx = -1; dx <= 4; dx++) {
        place_building_block(world, base_x + dx, base_y - height - 1, MaterialID::Wood);
    }
}

// Build a treehouse (8 wide, 12 tall) - elevated platform with trunk ladder
static void build_treehouse(World& world, int32_t base_x, int32_t base_y, uint32_t seed) {
    (void)seed;
    int platform_height = 8;

    // Tree trunk (people climb on it)
    for (int dy = 0; dy <= platform_height + 3; dy++) {
        place_building_block(world, base_x + 3, base_y - dy, MaterialID::Wood);
        place_building_block(world, base_x + 4, base_y - dy, MaterialID::Wood);
    }

    // Main platform
    for (int dx = 0; dx < 8; dx++) {
        place_building_block(world, base_x + dx, base_y - platform_height, MaterialID::Wood);
    }

    // Railings
    for (int dx = 0; dx < 8; dx++) {
        if (dx != 3 && dx != 4) {  // Opening at trunk
            place_building_block(world, base_x + dx, base_y - platform_height - 1, MaterialID::Wood);
        }
    }

    // Small roof
    for (int dx = 1; dx < 7; dx++) {
        place_building_block(world, base_x + dx, base_y - platform_height - 4, MaterialID::Leaf);
    }
    for (int dx = 2; dx < 6; dx++) {
        place_building_block(world, base_x + dx, base_y - platform_height - 5, MaterialID::Leaf);
    }

    // Leaves around top of trunk
    for (int dy = 0; dy < 3; dy++) {
        place_building_block(world, base_x + 2, base_y - platform_height - 2 - dy, MaterialID::Leaf);
        place_building_block(world, base_x + 5, base_y - platform_height - 2 - dy, MaterialID::Leaf);
    }
}

// Build a MEGA tower (8 wide, 35 tall) - extremely tall with many platforms!
static void build_mega_tower(World& world, int32_t base_x, int32_t base_y, uint32_t seed) {
    (void)seed;
    int height = 32;

    // Thick foundation
    for (int dx = 0; dx < 8; dx++) {
        place_building_block(world, base_x + dx, base_y, MaterialID::Stone);
        place_building_block(world, base_x + dx, base_y + 1, MaterialID::Stone);
    }

    // Main tower shaft
    for (int dy = 1; dy <= height; dy++) {
        place_building_block(world, base_x, base_y - dy, MaterialID::Stone);
        place_building_block(world, base_x + 7, base_y - dy, MaterialID::Stone);

        // Platforms every 3 levels for easy climbing
        if (dy % 3 == 0) {
            for (int dx = 1; dx < 7; dx++) {
                place_building_block(world, base_x + dx, base_y - dy, MaterialID::Wood);
            }
        }
    }

    // Wider observation platforms every 10 levels
    for (int section = 1; section <= 3; section++) {
        int level = section * 10;
        if (level <= height) {
            for (int dx = -1; dx <= 8; dx++) {
                place_building_block(world, base_x + dx, base_y - level, MaterialID::Stone);
            }
            // Railings
            place_building_block(world, base_x - 1, base_y - level - 1, MaterialID::Wood);
            place_building_block(world, base_x + 8, base_y - level - 1, MaterialID::Wood);
        }
    }

    // Grand top platform
    for (int dx = -2; dx <= 9; dx++) {
        place_building_block(world, base_x + dx, base_y - height - 1, MaterialID::Stone);
    }
    // Flag pole
    for (int dy = 0; dy < 5; dy++) {
        place_building_block(world, base_x + 4, base_y - height - 2 - dy, MaterialID::Wood);
    }
    place_building_block(world, base_x + 5, base_y - height - 5, MaterialID::Copper);
    place_building_block(world, base_x + 5, base_y - height - 6, MaterialID::Copper);
}

// Build zigzag stairs (12 wide, 16 tall) - switchback stairs
static void build_zigzag_stairs(World& world, int32_t base_x, int32_t base_y, uint32_t seed) {
    (void)seed;
    int levels = 4;
    int level_height = 4;

    for (int level = 0; level < levels; level++) {
        int level_base = base_y - (level * level_height);

        // Landing platform
        for (int dx = 0; dx < 12; dx++) {
            place_building_block(world, base_x + dx, level_base, MaterialID::Stone);
        }

        // Stairs going right on even levels, left on odd
        if (level < levels - 1) {
            if (level % 2 == 0) {
                for (int step = 0; step < 4; step++) {
                    place_building_block(world, base_x + step * 2, level_base - 1 - step, MaterialID::Stone);
                    place_building_block(world, base_x + step * 2 + 1, level_base - 1 - step, MaterialID::Stone);
                }
            } else {
                for (int step = 0; step < 4; step++) {
                    place_building_block(world, base_x + 11 - step * 2, level_base - 1 - step, MaterialID::Stone);
                    place_building_block(world, base_x + 10 - step * 2, level_base - 1 - step, MaterialID::Stone);
                }
            }
        }
    }

    // Top platform with railings
    for (int dx = 0; dx < 12; dx++) {
        place_building_block(world, base_x + dx, base_y - levels * level_height, MaterialID::Stone);
    }
    place_building_block(world, base_x, base_y - levels * level_height - 1, MaterialID::Wood);
    place_building_block(world, base_x + 11, base_y - levels * level_height - 1, MaterialID::Wood);
}

// Build an elevator shaft (4 wide, 20 tall) - vertical platforms
static void build_elevator(World& world, int32_t base_x, int32_t base_y, uint32_t seed) {
    (void)seed;
    int height = 18;

    // Shaft walls
    for (int dy = 0; dy <= height; dy++) {
        place_building_block(world, base_x, base_y - dy, MaterialID::Metal);
        place_building_block(world, base_x + 3, base_y - dy, MaterialID::Metal);
    }

    // "Elevator" platforms at multiple levels (every 4 blocks)
    for (int level = 0; level <= height; level += 4) {
        place_building_block(world, base_x + 1, base_y - level, MaterialID::Wood);
        place_building_block(world, base_x + 2, base_y - level, MaterialID::Wood);
    }

    // Top exit platform
    for (int dx = -2; dx <= 5; dx++) {
        place_building_block(world, base_x + dx, base_y - height - 1, MaterialID::Metal);
    }
}

// Build apartment building (12 wide, 18 tall) - multiple units
static void build_apartment(World& world, int32_t base_x, int32_t base_y, uint32_t seed) {
    (void)seed;
    int floors = 4;
    int floor_height = 4;

    // Foundation
    for (int dx = 0; dx < 12; dx++) {
        place_building_block(world, base_x + dx, base_y, MaterialID::Stone);
    }

    for (int floor = 0; floor < floors; floor++) {
        int fy = base_y - floor * floor_height;

        // Floor divider
        for (int dx = 0; dx < 12; dx++) {
            place_building_block(world, base_x + dx, fy - floor_height, MaterialID::Stone);
        }

        // Walls
        for (int dy = 1; dy < floor_height; dy++) {
            place_building_block(world, base_x, fy - dy, MaterialID::Brick);
            place_building_block(world, base_x + 11, fy - dy, MaterialID::Brick);
            // Center divider
            place_building_block(world, base_x + 5, fy - dy, MaterialID::Brick);
            place_building_block(world, base_x + 6, fy - dy, MaterialID::Brick);
        }

        // Windows
        place_building_block(world, base_x + 2, fy - 2, MaterialID::Glass);
        place_building_block(world, base_x + 3, fy - 2, MaterialID::Glass);
        place_building_block(world, base_x + 8, fy - 2, MaterialID::Glass);
        place_building_block(world, base_x + 9, fy - 2, MaterialID::Glass);
    }

    // External stairs (climbable!)
    for (int floor = 0; floor < floors; floor++) {
        int fy = base_y - floor * floor_height;
        for (int step = 0; step < 3; step++) {
            place_building_block(world, base_x + 12 + step, fy - 1 - step, MaterialID::Metal);
        }
        // Landing
        place_building_block(world, base_x + 12, fy - floor_height, MaterialID::Metal);
        place_building_block(world, base_x + 13, fy - floor_height, MaterialID::Metal);
    }

    // Roof
    for (int dx = 0; dx < 14; dx++) {
        place_building_block(world, base_x + dx, base_y - floors * floor_height - 1, MaterialID::Stone);
    }
}

// Build a step pyramid (16 wide, 12 tall) - climbable stepped sides
static void build_pyramid(World& world, int32_t base_x, int32_t base_y, uint32_t seed) {
    (void)seed;
    int levels = 6;

    for (int level = 0; level < levels; level++) {
        int width = 16 - level * 2;
        int start_x = base_x + level;
        int level_y = base_y - level * 2;

        // Each level is 2 blocks tall
        for (int dy = 0; dy < 2; dy++) {
            for (int dx = 0; dx < width; dx++) {
                place_building_block(world, start_x + dx, level_y - dy, MaterialID::Stone);
            }
        }
    }

    // Treasure at top
    place_building_block(world, base_x + 7, base_y - 12, MaterialID::Gold);
    place_building_block(world, base_x + 8, base_y - 12, MaterialID::Gold);
}

// Build a multi-tier pagoda (8 wide, 20 tall)
static void build_pagoda(World& world, int32_t base_x, int32_t base_y, uint32_t seed) {
    (void)seed;
    int tiers = 5;

    for (int tier = 0; tier < tiers; tier++) {
        int tier_y = base_y - tier * 4;
        int tier_width = 8 - tier;
        int tier_start = base_x + tier / 2;

        // Floor
        for (int dx = 0; dx < tier_width; dx++) {
            place_building_block(world, tier_start + dx, tier_y, MaterialID::Wood);
        }

        // Walls
        for (int dy = 1; dy <= 3; dy++) {
            place_building_block(world, tier_start, tier_y - dy, MaterialID::Wood);
            place_building_block(world, tier_start + tier_width - 1, tier_y - dy, MaterialID::Wood);
        }

        // Roof overhang
        for (int dx = -1; dx <= tier_width; dx++) {
            place_building_block(world, tier_start + dx, tier_y - 4, MaterialID::Copper);
        }
    }

    // Spire at top
    for (int dy = 0; dy < 3; dy++) {
        place_building_block(world, base_x + 4, base_y - tiers * 4 - 1 - dy, MaterialID::Gold);
    }
}

// Build a tall aqueduct (20 wide, 15 tall)
static void build_aqueduct(World& world, int32_t base_x, int32_t base_y, uint32_t seed) {
    (void)seed;
    int height = 12;
    int span = 18;

    // Arched pillars
    for (int pillar = 0; pillar < 3; pillar++) {
        int px = base_x + pillar * 8;

        // Main column
        for (int dy = 0; dy <= height; dy++) {
            place_building_block(world, px, base_y - dy, MaterialID::Stone);
            place_building_block(world, px + 1, base_y - dy, MaterialID::Stone);
        }

        // Arch
        if (pillar < 2) {
            for (int arch = 0; arch < 4; arch++) {
                place_building_block(world, px + 2 + arch, base_y - 7 - arch, MaterialID::Stone);
                place_building_block(world, px + 6 - arch, base_y - 7 - arch, MaterialID::Stone);
            }
        }
    }

    // Water channel on top
    for (int dx = 0; dx < span; dx++) {
        place_building_block(world, base_x + dx, base_y - height - 1, MaterialID::Stone);
    }
    // Water in channel
    for (int dx = 1; dx < span - 1; dx++) {
        place_building_block(world, base_x + dx, base_y - height - 2, MaterialID::Water);
    }

    // Walkway alongside
    for (int dx = 0; dx < span; dx++) {
        place_building_block(world, base_x + dx, base_y - height, MaterialID::Stone);
    }
}

// Build a very tall bell tower (6 wide, 28 tall)
static void build_bell_tower(World& world, int32_t base_x, int32_t base_y, uint32_t seed) {
    (void)seed;
    int height = 25;

    // Thick foundation
    for (int dx = 0; dx < 6; dx++) {
        place_building_block(world, base_x + dx, base_y, MaterialID::Stone);
        place_building_block(world, base_x + dx, base_y + 1, MaterialID::Stone);
    }

    // Main shaft with interior platforms
    for (int dy = 1; dy <= height; dy++) {
        place_building_block(world, base_x, base_y - dy, MaterialID::Brick);
        place_building_block(world, base_x + 5, base_y - dy, MaterialID::Brick);

        // Platforms every 4 levels
        if (dy % 4 == 0 && dy < height - 2) {
            for (int dx = 1; dx < 5; dx++) {
                place_building_block(world, base_x + dx, base_y - dy, MaterialID::Wood);
            }
        }

        // Windows at certain levels
        if (dy % 6 == 3) {
            place_building_block(world, base_x + 2, base_y - dy, MaterialID::Glass);
            place_building_block(world, base_x + 3, base_y - dy, MaterialID::Glass);
        }
    }

    // Bell chamber (open sides)
    for (int dy = 0; dy < 3; dy++) {
        place_building_block(world, base_x, base_y - height - dy, MaterialID::Stone);
        place_building_block(world, base_x + 5, base_y - height - dy, MaterialID::Stone);
    }

    // Bell
    place_building_block(world, base_x + 2, base_y - height - 1, MaterialID::Gold);
    place_building_block(world, base_x + 3, base_y - height - 1, MaterialID::Gold);
    place_building_block(world, base_x + 2, base_y - height - 2, MaterialID::Gold);
    place_building_block(world, base_x + 3, base_y - height - 2, MaterialID::Gold);

    // Pointed roof
    for (int dx = -1; dx <= 6; dx++) {
        place_building_block(world, base_x + dx, base_y - height - 3, MaterialID::Copper);
    }
    for (int dx = 0; dx <= 5; dx++) {
        place_building_block(world, base_x + dx, base_y - height - 4, MaterialID::Copper);
    }
    place_building_block(world, base_x + 2, base_y - height - 5, MaterialID::Copper);
    place_building_block(world, base_x + 3, base_y - height - 5, MaterialID::Copper);
}

// Build a construction crane (10 wide, 30 tall)
static void build_crane(World& world, int32_t base_x, int32_t base_y, uint32_t seed) {
    (void)seed;
    int height = 28;

    // Base
    for (int dx = 0; dx < 4; dx++) {
        place_building_block(world, base_x + dx, base_y, MaterialID::Metal);
        place_building_block(world, base_x + dx, base_y + 1, MaterialID::Metal);
    }

    // Main mast (climbable!)
    for (int dy = 0; dy <= height; dy++) {
        place_building_block(world, base_x + 1, base_y - dy, MaterialID::Metal);
        place_building_block(world, base_x + 2, base_y - dy, MaterialID::Metal);

        // Climbing rungs every 3 levels
        if (dy % 3 == 0 && dy > 0) {
            place_building_block(world, base_x, base_y - dy, MaterialID::Metal);
            place_building_block(world, base_x + 3, base_y - dy, MaterialID::Metal);
        }
    }

    // Horizontal jib at top
    for (int dx = -2; dx < 10; dx++) {
        place_building_block(world, base_x + dx, base_y - height, MaterialID::Metal);
    }

    // Counterweight
    place_building_block(world, base_x - 2, base_y - height + 1, MaterialID::Stone);
    place_building_block(world, base_x - 1, base_y - height + 1, MaterialID::Stone);

    // Hook/cable
    for (int dy = 0; dy < 5; dy++) {
        place_building_block(world, base_x + 8, base_y - height + 1 + dy, MaterialID::Metal);
    }

    // Platform at top of mast
    for (int dx = -1; dx <= 4; dx++) {
        place_building_block(world, base_x + dx, base_y - height - 1, MaterialID::Metal);
    }
}

// Build floating sky platforms with ladders (15 wide, 20 tall)
static void build_sky_platform(World& world, int32_t base_x, int32_t base_y, uint32_t seed) {
    (void)seed;

    // Ground anchor
    for (int dx = 0; dx < 3; dx++) {
        place_building_block(world, base_x + dx, base_y, MaterialID::Stone);
    }

    // Main ladder up to first platform
    for (int dy = 0; dy < 8; dy++) {
        place_building_block(world, base_x + 1, base_y - dy, MaterialID::Wood);
    }

    // First platform (height 8)
    for (int dx = 0; dx < 6; dx++) {
        place_building_block(world, base_x + dx, base_y - 8, MaterialID::Wood);
    }

    // Second ladder
    for (int dy = 9; dy < 14; dy++) {
        place_building_block(world, base_x + 5, base_y - dy, MaterialID::Wood);
    }

    // Second platform (height 14)
    for (int dx = 3; dx < 10; dx++) {
        place_building_block(world, base_x + dx, base_y - 14, MaterialID::Wood);
    }

    // Third ladder
    for (int dy = 15; dy < 18; dy++) {
        place_building_block(world, base_x + 9, base_y - dy, MaterialID::Wood);
    }

    // Top platform (height 18)
    for (int dx = 6; dx < 14; dx++) {
        place_building_block(world, base_x + dx, base_y - 18, MaterialID::Wood);
    }

    // Railings on each platform
    place_building_block(world, base_x, base_y - 9, MaterialID::Wood);
    place_building_block(world, base_x + 3, base_y - 15, MaterialID::Wood);
    place_building_block(world, base_x + 6, base_y - 19, MaterialID::Wood);
    place_building_block(world, base_x + 13, base_y - 19, MaterialID::Wood);
}

// Structure dimensions for space checking - EXPANDED
struct BuildingDimensions {
    int width;
    int height;
};

static const BuildingDimensions BUILDING_SIZES[] = {
    // Basic (0-9)
    {8, 8},   // Cottage (6 + margin)
    {11, 8},  // StoneHouse (9 + margin)
    {7, 14},  // WatchTower (5 + margin)
    {14, 10}, // Barn (12 + margin)
    {7, 8},   // Well (5 + margin)
    {16, 5},  // Bridge (variable, max 15 + margin)
    {14, 4},  // Fence (variable, max 13 + margin)
    {6, 8},   // Shrine (4 + margin)
    {12, 16}, // Windmill (6 + margin, very tall)
    {16, 12}, // Inn (14 + margin)
    // Advanced (10-19)
    {35, 22}, // Castle (multi-tower, very large)
    {10, 18}, // Church (tall steeple)
    {22, 8},  // Market (wide, multiple stalls)
    {8, 25},  // Lighthouse (very tall)
    {12, 8},  // Tavern (ground level)
    {10, 10}, // Stairs (climbable)
    {3, 12},  // Ladder (narrow, tall)
    {18, 6},  // SkywalkBridge (elevated bridge)
    {22, 12}, // GrandHall (large gathering hall)
    {12, 20}, // Observatory (tall dome)
    // Decorative/Infrastructure (20-24)
    {8, 6},   // Fountain
    {5, 10},  // Statue (tall)
    {12, 5},  // Garden (wide, low)
    {15, 5},  // Dock (extends out)
    {6, 16},  // Tower (climbable with platforms)
    // VERTICAL STRUCTURES (25-39)
    {8, 22},  // SpiralTower (6 + margin, very tall)
    {9, 17},  // Scaffolding (7 + margin, easy climb)
    {12, 30}, // Skyscraper (10 + margin, many floors)
    {6, 15},  // ClimbingWall (4 + margin)
    {10, 15}, // TreeHouse (8 + margin)
    {12, 40}, // MegaTower (8 + margin, EXTREMELY tall!)
    {14, 20}, // ZigzagStairs (12 + margin)
    {8, 22},  // Elevator (4 + margin)
    {16, 20}, // Apartment (12 + margin + external stairs)
    {18, 15}, // Pyramid (16 + margin)
    {12, 24}, // Pagoda (8 + margin)
    {22, 18}, // Aqueduct (20 + margin)
    {8, 32},  // BellTower (6 + margin, very tall)
    {15, 35}, // Crane (10 + margin, very tall)
    {16, 22}, // SkyPlatform (15 + margin)
};

// Try to build a structure at the given location
static bool try_build_structure(World& world, int32_t x, int32_t y, BuildingType type, uint32_t seed) {
    int type_idx = static_cast<int>(type);
    const BuildingDimensions& dims = BUILDING_SIZES[type_idx];

    // Check if area is clear for building
    if (!is_area_clear(world, x, y, dims.width, dims.height)) {
        return false;
    }

    // Build the structure
    switch (type) {
        case BuildingType::Cottage:
            build_cottage(world, x, y, seed);
            break;
        case BuildingType::StoneHouse:
            build_stone_house(world, x, y, seed);
            break;
        case BuildingType::WatchTower:
            build_watchtower(world, x, y, seed);
            break;
        case BuildingType::Barn:
            build_barn(world, x, y, seed);
            break;
        case BuildingType::Well:
            build_well(world, x, y, seed);
            break;
        case BuildingType::Bridge:
            build_bridge(world, x, y, seed);
            break;
        case BuildingType::Fence:
            build_fence(world, x, y, seed);
            break;
        case BuildingType::Shrine:
            build_shrine(world, x, y, seed);
            break;
        case BuildingType::Windmill:
            build_windmill(world, x, y, seed);
            break;
        case BuildingType::Inn:
            build_inn(world, x, y, seed);
            break;
        // Advanced (10-19)
        case BuildingType::Castle:
            build_castle(world, x, y, seed);
            break;
        case BuildingType::Church:
            build_church(world, x, y, seed);
            break;
        case BuildingType::Market:
            build_market(world, x, y, seed);
            break;
        case BuildingType::Lighthouse:
            build_lighthouse(world, x, y, seed);
            break;
        case BuildingType::Tavern:
            build_tavern(world, x, y, seed);
            break;
        case BuildingType::Stairs:
            build_stairs(world, x, y, seed);
            break;
        case BuildingType::Ladder:
            build_ladder(world, x, y, seed);
            break;
        case BuildingType::SkywalkBridge:
            build_skywalk_bridge(world, x, y, seed);
            break;
        case BuildingType::GrandHall:
            build_grand_hall(world, x, y, seed);
            break;
        case BuildingType::Observatory:
            build_observatory(world, x, y, seed);
            break;
        // Decorative/Infrastructure (20-24)
        case BuildingType::Fountain:
            build_fountain(world, x, y, seed);
            break;
        case BuildingType::Statue:
            build_statue(world, x, y, seed);
            break;
        case BuildingType::Garden:
            build_garden(world, x, y, seed);
            break;
        case BuildingType::Dock:
            build_dock(world, x, y, seed);
            break;
        case BuildingType::Tower:
            build_tower(world, x, y, seed);
            break;
        // VERTICAL STRUCTURES (25-39)
        case BuildingType::SpiralTower:
            build_spiral_tower(world, x, y, seed);
            break;
        case BuildingType::Scaffolding:
            build_scaffolding(world, x, y, seed);
            break;
        case BuildingType::Skyscraper:
            build_skyscraper(world, x, y, seed);
            break;
        case BuildingType::ClimbingWall:
            build_climbing_wall(world, x, y, seed);
            break;
        case BuildingType::TreeHouse:
            build_treehouse(world, x, y, seed);
            break;
        case BuildingType::MegaTower:
            build_mega_tower(world, x, y, seed);
            break;
        case BuildingType::ZigzagStairs:
            build_zigzag_stairs(world, x, y, seed);
            break;
        case BuildingType::Elevator:
            build_elevator(world, x, y, seed);
            break;
        case BuildingType::Apartment:
            build_apartment(world, x, y, seed);
            break;
        case BuildingType::Pyramid:
            build_pyramid(world, x, y, seed);
            break;
        case BuildingType::Pagoda:
            build_pagoda(world, x, y, seed);
            break;
        case BuildingType::Aqueduct:
            build_aqueduct(world, x, y, seed);
            break;
        case BuildingType::BellTower:
            build_bell_tower(world, x, y, seed);
            break;
        case BuildingType::Crane:
            build_crane(world, x, y, seed);
            break;
        case BuildingType::SkyPlatform:
            build_sky_platform(world, x, y, seed);
            break;
        default:
            return false;
    }
    return true;
}

// Choose a building type based on personality and random seed
static BuildingType choose_building_type(uint32_t seed, uint8_t personality) {
    // Combine seed and personality for variety
    uint32_t choice = (seed ^ (personality * 17)) % 400;

    // Weighted distribution (total 400) - HEAVY emphasis on vertical structures!
    // === VERTICAL CLIMBABLE (Very Common - 40% of buildings!) ===
    if (choice < 25) return BuildingType::Scaffolding;     // 6.25% - easy climb
    if (choice < 48) return BuildingType::Ladder;          // 5.75% - basic vertical
    if (choice < 70) return BuildingType::Stairs;          // 5.5% - diagonal climb
    if (choice < 90) return BuildingType::ClimbingWall;    // 5% - dense handholds
    if (choice < 108) return BuildingType::ZigzagStairs;   // 4.5% - switchback
    if (choice < 124) return BuildingType::Tower;          // 4% - multi-platform
    if (choice < 140) return BuildingType::SpiralTower;    // 4% - spiral climb
    if (choice < 154) return BuildingType::Elevator;       // 3.5% - vertical shaft
    if (choice < 166) return BuildingType::SkyPlatform;    // 3% - floating platforms

    // === Basic Village (25% of buildings) ===
    if (choice < 182) return BuildingType::Cottage;        // 4%
    if (choice < 196) return BuildingType::Fence;          // 3.5%
    if (choice < 210) return BuildingType::Bridge;         // 3.5%
    if (choice < 222) return BuildingType::StoneHouse;     // 3%
    if (choice < 232) return BuildingType::Barn;           // 2.5%
    if (choice < 240) return BuildingType::Well;           // 2%
    if (choice < 248) return BuildingType::Garden;         // 2%

    // === Multi-story Buildings (15% - vertical living!) ===
    if (choice < 260) return BuildingType::TreeHouse;      // 3%
    if (choice < 272) return BuildingType::Apartment;      // 3%
    if (choice < 282) return BuildingType::Skyscraper;     // 2.5%
    if (choice < 292) return BuildingType::Pagoda;         // 2.5%
    if (choice < 300) return BuildingType::Inn;            // 2%
    if (choice < 306) return BuildingType::BellTower;      // 1.5%

    // === Infrastructure (10%) ===
    if (choice < 316) return BuildingType::SkywalkBridge;  // 2.5% - connects buildings!
    if (choice < 324) return BuildingType::Aqueduct;       // 2%
    if (choice < 332) return BuildingType::Market;         // 2%
    if (choice < 338) return BuildingType::Shrine;         // 1.5%
    if (choice < 344) return BuildingType::Fountain;       // 1.5%
    if (choice < 348) return BuildingType::Dock;           // 1%

    // === Impressive Structures (8%) ===
    if (choice < 358) return BuildingType::WatchTower;     // 2.5%
    if (choice < 368) return BuildingType::Windmill;       // 2.5%
    if (choice < 374) return BuildingType::GrandHall;      // 1.5%
    if (choice < 380) return BuildingType::Pyramid;        // 1.5%

    // === Rare/Epic (2%) ===
    if (choice < 386) return BuildingType::MegaTower;      // 1.5% - extremely tall!
    if (choice < 390) return BuildingType::Crane;          // 1% - construction crane
    if (choice < 394) return BuildingType::Church;         // 1%
    if (choice < 396) return BuildingType::Lighthouse;     // 0.5%
    if (choice < 398) return BuildingType::Observatory;    // 0.5%
    if (choice < 399) return BuildingType::Castle;         // 0.25%
    return BuildingType::Statue;                           // 0.25%
}

// Main person update function - with village building behavior
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
    (void)personality;  // Used for building seeds

    // Get facing direction
    bool facing_right = cell.get_person_facing_right();

    // ========================================
    // SIMPLIFIED GRAVITY & CLIMBING - Build when stuck!
    // ========================================
    bool grounded = false;
    if (world.in_bounds(x, y + 1)) {
        MaterialID below = world.get_material(x, y + 1);
        grounded = is_person_ground(below);
    }

    // Check for walls on either side
    bool wall_left = world.in_bounds(x - 1, y) && is_person_ground(world.get_material(x - 1, y));
    bool wall_right = world.in_bounds(x + 1, y) && is_person_ground(world.get_material(x + 1, y));
    bool touching_wall = wall_left || wall_right;

    // SIMPLE CLIMBING: If not grounded and touching a wall, climb up or build
    if (!grounded && touching_wall) {
        int wall_dir = wall_left ? -1 : 1;

        // Can we move up?
        bool can_move_up = world.in_bounds(x, y - 1) && is_passable(world.get_material(x, y - 1));

        if (can_move_up) {
            // Check if wall continues above - if so, climb
            bool wall_above = world.in_bounds(x + wall_dir, y - 1) &&
                              is_person_ground(world.get_material(x + wall_dir, y - 1));

            if (wall_above) {
                // Climb up
                world.try_move_cell(x, y, x, y - 1);
                return;
            }

            // Wall ended - step onto top
            if (world.in_bounds(x + wall_dir, y - 1) && is_passable(world.get_material(x + wall_dir, y - 1))) {
                world.try_move_cell(x, y, x + wall_dir, y - 1);
                return;
            }
        }

        // STUCK! Can't climb - build a structure here and drop down
        uint32_t stuck_seed = static_cast<uint32_t>(x * 31337 + y * 7919 + personality + frame);
        if ((frame & 7) == 0) {
            BuildingType building;
            uint32_t choice = stuck_seed % 6;
            if (choice < 2) building = BuildingType::Ladder;
            else if (choice < 3) building = BuildingType::Tower;
            else if (choice < 4) building = BuildingType::Scaffolding;
            else building = BuildingType::ZigzagStairs;

            try_build_structure(world, x, y, building, stuck_seed);
        }

        // Fall down slowly
        if ((frame & 3) == 0 && world.in_bounds(x, y + 1) && is_passable(world.get_material(x, y + 1))) {
            world.try_move_cell(x, y, x, y + 1);
        }
        return;
    }

    // Normal gravity if not touching walls
    if (!grounded && !touching_wall) {
        if (world.try_move_cell(x, y, x, y + 1)) {
            return;
        }
        // Try diagonal fall
        int side = facing_right ? 1 : -1;
        if (world.in_bounds(x + side, y + 1) && is_passable(world.get_material(x + side, y + 1))) {
            world.try_move_cell(x, y, x + side, y + 1);
        }
        return;
    }

    // ========================================
    // BUILDING BEHAVIOR - Everyone builds constantly!
    // ========================================
    // Build every 16 frames - very frequent building!
    if ((frame & 15) == 0) {
        uint32_t build_seed = static_cast<uint32_t>(x * 31337 + y * 7919 + personality + frame);

        // 25% chance to build each check (1 in 4)
        if ((build_seed & 3) == 0) {
            int search_dir = facing_right ? 1 : -1;
            int build_x = x + search_dir * (2 + (build_seed & 7));  // 2-9 blocks away

            int build_y = find_ground_level(world, build_x, y - 30);

            if (build_y > 0 && build_y < WORLD_HEIGHT - 30) {
                BuildingType building = choose_building_type(build_seed >> 3, personality);

                if (try_build_structure(world, build_x, build_y, building, build_seed)) {
                    cell.set_person_facing_right(!facing_right);
                    return;
                }
            }
        }
    }

    // ========================================
    // BRIDGE DETECTION - Check if there's a bridge above us we should climb to
    // ========================================
    // Every 8 frames, check if we're stuck under a bridge/platform
    if ((frame & 7) == 0 && grounded) {
        // Scan upward for a bridge/platform above us
        int bridge_height = -1;
        for (int scan_y = y - 2; scan_y >= y - 12; scan_y--) {
            if (!world.in_bounds(x, scan_y)) break;

            MaterialID above = world.get_material(x, scan_y);

            // Found a platform above! (brick, stone, wood, etc.)
            if (is_person_ground(above)) {
                // Check if there's empty space to stand on top of it
                if (world.in_bounds(x, scan_y - 1) && is_passable(world.get_material(x, scan_y - 1))) {
                    bridge_height = scan_y;
                }
                break;
            }
        }

        // If we found a bridge above us, build a pillar up to it!
        if (bridge_height > 0 && bridge_height < y - 2) {
            // Check that the space between us and the bridge is mostly empty
            bool can_build_pillar = true;
            for (int check_y = y - 1; check_y > bridge_height; check_y--) {
                if (!world.in_bounds(x, check_y)) {
                    can_build_pillar = false;
                    break;
                }
                MaterialID mat = world.get_material(x, check_y);
                if (mat != MaterialID::Empty && mat != MaterialID::Steam && mat != MaterialID::Smoke) {
                    can_build_pillar = false;
                    break;
                }
            }

            if (can_build_pillar) {
                // Build pillar from our position up to the bridge
                // Build one block at a time (more natural looking)
                for (int build_y = y - 1; build_y > bridge_height; build_y--) {
                    if (world.in_bounds(x, build_y) && world.get_material(x, build_y) == MaterialID::Empty) {
                        world.set_material(x, build_y, MaterialID::Wood);
                        // Move up onto the pillar
                        world.try_move_cell(x, y, x, build_y);
                        return;
                    }
                }

                // Pillar complete - move up to bridge level
                int stand_y = bridge_height - 1;
                if (world.in_bounds(x, stand_y) && is_passable(world.get_material(x, stand_y))) {
                    world.try_move_cell(x, y, x, stand_y);
                    return;
                }
            }
        }
    }

    // ========================================
    // MOVEMENT - Only move every 4 frames (works when grounded OR clinging)
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

    // ===== CASE 1: Path is clear - check for edges first! =====
    if (is_passable(ahead)) {
        // Check if there's ground below where we'd step
        bool has_ground_ahead = false;
        for (int drop = 1; drop <= 3; drop++) {
            if (world.in_bounds(next_x, y + drop)) {
                MaterialID below_ahead = world.get_material(next_x, y + drop);
                if (is_person_ground(below_ahead)) {
                    has_ground_ahead = true;
                    break;
                }
                if (!is_passable(below_ahead)) {
                    break;  // Hit something non-passable that's not ground
                }
            }
        }

        if (has_ground_ahead) {
            // Safe to walk forward
            world.try_move_cell(x, y, next_x, y);
            return;
        }

        // ===== ON AN EDGE! =====
        // ALWAYS try to build a structure here! No complex logic.
        uint32_t edge_seed = static_cast<uint32_t>(x * 31337 + y * 7919 + personality + frame);

        // Try to build a structure at the edge
        BuildingType building;
        uint32_t choice = edge_seed % 10;
        if (choice < 2) building = BuildingType::Tower;
        else if (choice < 3) building = BuildingType::Ladder;
        else if (choice < 4) building = BuildingType::Scaffolding;
        else if (choice < 5) building = BuildingType::ZigzagStairs;
        else if (choice < 6) building = BuildingType::SpiralTower;
        else if (choice < 7) building = BuildingType::ClimbingWall;
        else if (choice < 8) building = BuildingType::Apartment;
        else if (choice < 9) building = BuildingType::WatchTower;
        else building = BuildingType::BellTower;

        try_build_structure(world, x, y, building, edge_seed);

        // Always turn around at edge - don't try to bridge
        cell.set_person_facing_right(!facing_right);
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

    // ===== CASE 3: Blocked by solid - Step up or build! =====

    // Try to step up 1-2 blocks onto the wall
    for (int step_up = 1; step_up <= 2; step_up++) {
        int target_y = y - step_up;
        if (!world.in_bounds(next_x, target_y)) break;
        if (!world.in_bounds(x, target_y)) break;

        // Check if we can stand at this height on the wall
        if (is_passable(world.get_material(next_x, target_y)) &&
            is_passable(world.get_material(x, target_y)) &&
            world.in_bounds(next_x, target_y + 1) &&
            is_person_ground(world.get_material(next_x, target_y + 1))) {
            if (world.try_move_cell(x, y, next_x, target_y)) {
                return;  // Stepped up!
            }
        }
    }

    // Can't step up - build a structure and turn around!
    uint32_t wall_seed = static_cast<uint32_t>(x * 31337 + y * 7919 + personality + frame);
    BuildingType building;
    uint32_t choice = wall_seed % 8;
    if (choice < 2) building = BuildingType::Tower;
    else if (choice < 3) building = BuildingType::Ladder;
    else if (choice < 4) building = BuildingType::Scaffolding;
    else if (choice < 5) building = BuildingType::ZigzagStairs;
    else if (choice < 6) building = BuildingType::SpiralTower;
    else if (choice < 7) building = BuildingType::Apartment;
    else building = BuildingType::ClimbingWall;

    try_build_structure(world, x, y, building, wall_seed);

    // Turn around
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
    // PERFORMANCE FIX: Use static cache with periodic refresh
    static int portal_out_x = -1, portal_out_y = -1;
    static uint32_t last_scan_frame = 0;
    static uint32_t scan_frame_counter = 0;

    scan_frame_counter++;

    // Check if cached portal is still valid
    bool cache_valid = (portal_out_x >= 0 && portal_out_y >= 0 &&
                        world.in_bounds(portal_out_x, portal_out_y) &&
                        world.get_material(portal_out_x, portal_out_y) == MaterialID::Portal_Out);

    // Re-scan if cache invalid and enough time has passed (every 30 frames = 0.5 sec)
    if (!cache_valid && (scan_frame_counter - last_scan_frame > 30)) {
        portal_out_x = -1;
        portal_out_y = -1;
        last_scan_frame = scan_frame_counter;

        // Scan every cell but exit immediately when found
        for (int sy = 0; sy < WORLD_HEIGHT; sy++) {
            for (int sx = 0; sx < WORLD_WIDTH; sx++) {
                if (world.get_material(sx, sy) == MaterialID::Portal_Out) {
                    portal_out_x = sx;
                    portal_out_y = sy;
                    cache_valid = true;
                    goto found_portal;  // Exit nested loops
                }
            }
        }
        found_portal:;
    }

    // No valid portal out - nothing to do
    if (portal_out_x < 0 || !cache_valid) return;

    // Only teleport every few frames to reduce load when many portals exist
    if ((world.random_int() & 1) != 0) return;  // 50% chance

    // Teleport materials touching this portal
    for (int dy = -1; dy <= 1; dy++) {
        for (int dx = -1; dx <= 1; dx++) {
            if (dx == 0 && dy == 0) continue;
            int nx = x + dx, ny = y + dy;
            if (!world.in_bounds(nx, ny)) continue;

            MaterialID m = world.get_material(nx, ny);
            if (m == MaterialID::Empty || m == MaterialID::Portal_In ||
                m == MaterialID::Portal_Out || m == MaterialID::Stone) continue;

            // Find empty spot near portal out
            for (int oy = -2; oy <= 2; oy++) {
                for (int ox = -2; ox <= 2; ox++) {
                    int px = portal_out_x + ox, py = portal_out_y + oy;
                    if (!world.in_bounds(px, py)) continue;
                    if (world.get_material(px, py) != MaterialID::Empty) continue;

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

    // Check for nearby people - don't spawn if too crowded!
    // This prevents overlapping and gives people space
    for (int dy = -4; dy <= 4; dy++) {
        for (int dx = -4; dx <= 4; dx++) {
            if (dx == 0 && dy == 0) continue;
            int nx = x + dx, ny = y + dy;
            if (world.in_bounds(nx, ny)) {
                MaterialID m = world.get_material(nx, ny);
                if (m == MaterialID::Person) {
                    return false;  // Too close to another person!
                }
            }
        }
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

    // Lifetime countdown - used both for sparkle effect and spawn delay
    uint8_t life = cell.get_lifetime();

    // Life particle needs to "settle" before spawning (wait ~30 frames minimum)
    // When first placed, lifetime is 0, so we initialize it
    if (life == 0) {
        cell.set_lifetime(50 + (world.random_int() & 31));  // 50-81 frame delay before can spawn
        return;  // Don't try to spawn on first frame
    }
    cell.decrement_lifetime();

    // Only try to spawn once lifetime gets low (particle has settled)
    // And only with a random chance to spread out spawns
    if (life < 20 && (world.random_int() & 3) == 0) {
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
    }

    // If lifetime runs out and we couldn't spawn, disappear with a puff
    if (life == 1) {
        world.set_material(x, y, MaterialID::Smoke);
        world.get_cell(x, y).set_lifetime(8);
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

// ============================================================================
// NEW MATERIALS (81-102) UPDATE FUNCTIONS
// ============================================================================

// ============================================================================
// NEW POWDERS (81-85)
// ============================================================================

// Thermite Powder - burns extremely hot when ignited by fire/spark/lava
void update_thermite_powder(World& world, int32_t x, int32_t y) {
    // Check for ignition sources nearby
    for (int dy = -1; dy <= 1; dy++) {
        for (int dx = -1; dx <= 1; dx++) {
            if (dx == 0 && dy == 0) continue;
            if (!world.in_bounds(x + dx, y + dy)) continue;

            MaterialID neighbor = world.get_material(x + dx, y + dy);
            if (neighbor == MaterialID::Fire || neighbor == MaterialID::Spark ||
                neighbor == MaterialID::Lava || neighbor == MaterialID::Dragon_Fire ||
                neighbor == MaterialID::Thermite || neighbor == MaterialID::Plasma) {
                // IGNITE! Become burning thermite
                world.set_material(x, y, MaterialID::Thermite);
                Cell& cell = world.get_cell(x, y);
                cell.set_lifetime(40);  // Burns for a while
                return;
            }
        }
    }

    // Otherwise fall like normal powder
    generic_powder_update(world, x, y, 3, 16);  // Heavy powder
}

// Sugar - dissolves in water, highly flammable
void update_sugar(World& world, int32_t x, int32_t y) {
    // Check for water (dissolves) or fire (burns bright)
    for (int dy = -1; dy <= 1; dy++) {
        for (int dx = -1; dx <= 1; dx++) {
            if (dx == 0 && dy == 0) continue;
            if (!world.in_bounds(x + dx, y + dy)) continue;

            MaterialID neighbor = world.get_material(x + dx, y + dy);

            // Dissolves in water
            if (neighbor == MaterialID::Water || neighbor == MaterialID::Juice) {
                if ((world.random_int() & 7) == 0) {
                    world.set_material(x, y, MaterialID::Empty);
                    return;
                }
            }

            // Burns easily
            if (neighbor == MaterialID::Fire || neighbor == MaterialID::Spark ||
                neighbor == MaterialID::Lava || neighbor == MaterialID::Ember) {
                world.set_material(x, y, MaterialID::Fire);
                Cell& cell = world.get_cell(x, y);
                cell.set_lifetime(15);  // Short bright burn
                return;
            }
        }
    }

    generic_powder_update(world, x, y, 2, 12);
}

// Iron Filings - rusts when wet, attracted to magnets (visual effect)
void update_iron_filings(World& world, int32_t x, int32_t y) {
    // Check for water - rust over time
    for (int dy = -1; dy <= 1; dy++) {
        for (int dx = -1; dx <= 1; dx++) {
            if (dx == 0 && dy == 0) continue;
            if (!world.in_bounds(x + dx, y + dy)) continue;

            MaterialID neighbor = world.get_material(x + dx, y + dy);
            if (neighbor == MaterialID::Water || neighbor == MaterialID::Blood ||
                neighbor == MaterialID::Acid) {
                if ((world.random_int() & 31) == 0) {  // Slow rusting
                    world.set_material(x, y, MaterialID::Rust);
                    return;
                }
            }
        }
    }

    generic_powder_update(world, x, y, 4, 20);  // Heavy iron falls fast
}

// Chalk - simple powder, dissolves slowly in water
void update_chalk(World& world, int32_t x, int32_t y) {
    // Dissolves slowly in water
    for (int dy = -1; dy <= 1; dy++) {
        for (int dx = -1; dx <= 1; dx++) {
            if (dx == 0 && dy == 0) continue;
            if (!world.in_bounds(x + dx, y + dy)) continue;

            MaterialID neighbor = world.get_material(x + dx, y + dy);
            if (neighbor == MaterialID::Water || neighbor == MaterialID::Acid) {
                if ((world.random_int() & 63) == 0) {
                    world.set_material(x, y, MaterialID::Empty);
                    return;
                }
            }
        }
    }

    generic_powder_update(world, x, y, 2, 12);
}

// Calcium - reacts violently with water (fizzes, produces hydrogen)
void update_calcium(World& world, int32_t x, int32_t y) {
    // React with water - fizz and produce hydrogen!
    for (int dy = -1; dy <= 1; dy++) {
        for (int dx = -1; dx <= 1; dx++) {
            if (dx == 0 && dy == 0) continue;
            if (!world.in_bounds(x + dx, y + dy)) continue;

            MaterialID neighbor = world.get_material(x + dx, y + dy);
            if (neighbor == MaterialID::Water) {
                if ((world.random_int() & 3) == 0) {
                    // Calcium + Water = Hydrogen gas + heat
                    world.set_material(x, y, MaterialID::Hydrogen);
                    // Sometimes also spawn a bit of steam from the heat
                    if ((world.random_int() & 1) == 0) {
                        world.set_material(x + dx, y + dy, MaterialID::Steam);
                    }
                    return;
                }
            }
        }
    }

    generic_powder_update(world, x, y, 2, 12);
}

// ============================================================================
// NEW LIQUIDS (86-90)
// ============================================================================

// Tar - extremely slow, sticky, flammable
void update_tar(World& world, int32_t x, int32_t y) {
    // Check for fire - tar burns slowly
    for (int dy = -1; dy <= 1; dy++) {
        for (int dx = -1; dx <= 1; dx++) {
            if (dx == 0 && dy == 0) continue;
            if (!world.in_bounds(x + dx, y + dy)) continue;

            MaterialID neighbor = world.get_material(x + dx, y + dy);
            if (neighbor == MaterialID::Fire || neighbor == MaterialID::Lava ||
                neighbor == MaterialID::Spark || neighbor == MaterialID::Thermite) {
                world.set_material(x, y, MaterialID::Fire);
                Cell& cell = world.get_cell(x, y);
                cell.set_lifetime(35);  // Long slow burn
                // Spread fire to nearby tar
                for (int sy = -1; sy <= 1; sy++) {
                    for (int sx = -1; sx <= 1; sx++) {
                        if (world.in_bounds(x + sx, y + sy) &&
                            world.get_material(x + sx, y + sy) == MaterialID::Tar &&
                            (world.random_int() & 7) == 0) {
                            world.set_material(x + sx, y + sy, MaterialID::Fire);
                            world.get_cell(x + sx, y + sy).set_lifetime(30);
                        }
                    }
                }
                return;
            }
        }
    }

    generic_slow_liquid_update(world, x, y, 7);  // Extremely slow like glue
}

// Juice - evaporates slowly, attracts organic life
void update_juice(World& world, int32_t x, int32_t y) {
    // Slow evaporation
    if ((world.random_int() & 511) == 0) {
        world.set_material(x, y, MaterialID::Steam);
        return;
    }

    // Standard liquid flow
    if (try_material_combination(world, x, y)) return;
    generic_slow_liquid_update(world, x, y, 1);
}

// Sap - amber liquid, can solidify over time
void update_sap(World& world, int32_t x, int32_t y) {
    // Very slowly solidifies into amber/wax
    if ((world.random_int() & 2047) == 0) {
        world.set_material(x, y, MaterialID::Wax);
        return;
    }

    // Burns when ignited
    for (int dy = -1; dy <= 1; dy++) {
        for (int dx = -1; dx <= 1; dx++) {
            if (dx == 0 && dy == 0) continue;
            if (!world.in_bounds(x + dx, y + dy)) continue;

            MaterialID neighbor = world.get_material(x + dx, y + dy);
            if (neighbor == MaterialID::Fire || neighbor == MaterialID::Lava) {
                world.set_material(x, y, MaterialID::Fire);
                world.get_cell(x, y).set_lifetime(20);
                return;
            }
        }
    }

    generic_slow_liquid_update(world, x, y, 3);  // Thick and slow
}

// Bleach - destroys organic materials, toxic
void update_bleach(World& world, int32_t x, int32_t y) {
    // Destroy organic materials on contact
    for (int dy = -1; dy <= 1; dy++) {
        for (int dx = -1; dx <= 1; dx++) {
            if (dx == 0 && dy == 0) continue;
            if (!world.in_bounds(x + dx, y + dy)) continue;

            MaterialID neighbor = world.get_material(x + dx, y + dy);

            // Destroys organics
            if (neighbor == MaterialID::Leaf || neighbor == MaterialID::Moss ||
                neighbor == MaterialID::Vine || neighbor == MaterialID::Fungus ||
                neighbor == MaterialID::Flower || neighbor == MaterialID::Algae ||
                neighbor == MaterialID::Flesh || neighbor == MaterialID::Bamboo ||
                neighbor == MaterialID::Wood || neighbor == MaterialID::Seed) {
                if ((world.random_int() & 7) == 0) {
                    world.set_material(x + dx, y + dy, MaterialID::Empty);
                    // Bleach gets consumed too
                    if ((world.random_int() & 3) == 0) {
                        world.set_material(x, y, MaterialID::Toxic_Gas);
                        return;
                    }
                }
            }

            // Mixing with acid creates toxic gas
            if (neighbor == MaterialID::Acid) {
                world.set_material(x, y, MaterialID::Toxic_Gas);
                world.set_material(x + dx, y + dy, MaterialID::Toxic_Gas);
                return;
            }
        }
    }

    generic_slow_liquid_update(world, x, y, 1);
}

// Ink - stains surfaces, flows like water
void update_ink(World& world, int32_t x, int32_t y) {
    // Just flows, nothing special
    if (try_material_combination(world, x, y)) return;
    generic_slow_liquid_update(world, x, y, 0);  // Flows smoothly
}

// ============================================================================
// NEW GASES (91-93)
// ============================================================================

// Chlorine - toxic green gas that SINKS (heavier than air)
void update_chlorine(World& world, int32_t x, int32_t y) {
    Cell& cell = world.get_cell(x, y);

    // Has lifetime - slowly dissipates
    if (cell.get_lifetime() > 0) {
        cell.decrement_lifetime();
        if (cell.get_lifetime() == 0) {
            world.set_material(x, y, MaterialID::Empty);
            return;
        }
    } else {
        // Initialize lifetime
        cell.set_lifetime(50 + (world.random_int() & 31));
    }

    // Damages organic things it touches
    for (int dy = -1; dy <= 1; dy++) {
        for (int dx = -1; dx <= 1; dx++) {
            if (dx == 0 && dy == 0) continue;
            if (!world.in_bounds(x + dx, y + dy)) continue;

            MaterialID neighbor = world.get_material(x + dx, y + dy);
            if (neighbor == MaterialID::Person || neighbor == MaterialID::Flesh ||
                neighbor == MaterialID::Leaf || neighbor == MaterialID::Flower) {
                if ((world.random_int() & 15) == 0) {
                    world.set_material(x + dx, y + dy, MaterialID::Empty);
                }
            }
        }
    }

    // SINKS instead of rises (heavy gas)
    if (world.in_bounds(x, y + 1)) {
        MaterialID below = world.get_material(x, y + 1);
        if (below == MaterialID::Empty) {
            world.try_move_cell(x, y, x, y + 1);
            return;
        }
    }

    // Spread sideways
    int dir = (world.random_int() & 1) ? 1 : -1;
    if (world.in_bounds(x + dir, y) && world.get_material(x + dir, y) == MaterialID::Empty) {
        world.try_move_cell(x, y, x + dir, y);
    }
}

// Liquid Nitrogen - freezes things on contact
void update_liquid_nitrogen(World& world, int32_t x, int32_t y) {
    Cell& cell = world.get_cell(x, y);

    // Short lifetime
    if (cell.get_lifetime() > 0) {
        cell.decrement_lifetime();
        if (cell.get_lifetime() == 0) {
            world.set_material(x, y, MaterialID::Empty);
            return;
        }
    } else {
        cell.set_lifetime(20 + (world.random_int() & 15));
    }

    // Freeze things on contact!
    for (int dy = -1; dy <= 1; dy++) {
        for (int dx = -1; dx <= 1; dx++) {
            if (dx == 0 && dy == 0) continue;
            if (!world.in_bounds(x + dx, y + dy)) continue;

            MaterialID neighbor = world.get_material(x + dx, y + dy);

            // Water -> Ice
            if (neighbor == MaterialID::Water) {
                world.set_material(x + dx, y + dy, MaterialID::Ice);
                continue;
            }

            // Lava -> Obsidian
            if (neighbor == MaterialID::Lava) {
                world.set_material(x + dx, y + dy, MaterialID::Obsidian);
                continue;
            }

            // Fire -> Empty (extinguishes)
            if (neighbor == MaterialID::Fire || neighbor == MaterialID::Ember) {
                world.set_material(x + dx, y + dy, MaterialID::Empty);
                continue;
            }

            // Organic -> Frost
            if (neighbor == MaterialID::Leaf || neighbor == MaterialID::Flower ||
                neighbor == MaterialID::Flesh) {
                if ((world.random_int() & 3) == 0) {
                    world.set_material(x + dx, y + dy, MaterialID::Frost);
                }
            }
        }
    }

    // Rises quickly (very cold gas)
    generic_gas_update(world, x, y, -3, -20, false);
}

// Oxygen - makes fires burn brighter and hotter
void update_oxygen(World& world, int32_t x, int32_t y) {
    Cell& cell = world.get_cell(x, y);

    // Lifetime
    if (cell.get_lifetime() > 0) {
        cell.decrement_lifetime();
        if (cell.get_lifetime() == 0) {
            world.set_material(x, y, MaterialID::Empty);
            return;
        }
    } else {
        cell.set_lifetime(40 + (world.random_int() & 31));
    }

    // Intensify nearby fires
    for (int dy = -1; dy <= 1; dy++) {
        for (int dx = -1; dx <= 1; dx++) {
            if (dx == 0 && dy == 0) continue;
            if (!world.in_bounds(x + dx, y + dy)) continue;

            MaterialID neighbor = world.get_material(x + dx, y + dy);

            // Makes fire last longer and spread
            if (neighbor == MaterialID::Fire) {
                Cell& fire_cell = world.get_cell(x + dx, y + dy);
                fire_cell.set_lifetime(std::min(63, (int)fire_cell.get_lifetime() + 10));

                // Spread fire more aggressively
                if ((world.random_int() & 3) == 0) {
                    int spread_dir = (world.random_int() & 3);
                    int sx = (spread_dir == 0) ? 1 : (spread_dir == 1) ? -1 : 0;
                    int sy = (spread_dir == 2) ? 1 : (spread_dir == 3) ? -1 : 0;
                    if (world.in_bounds(x + dx + sx, y + dy + sy) &&
                        world.get_material(x + dx + sx, y + dy + sy) == MaterialID::Empty) {
                        world.set_material(x + dx + sx, y + dy + sy, MaterialID::Fire);
                        world.get_cell(x + dx + sx, y + dy + sy).set_lifetime(15);
                    }
                }

                // Oxygen consumed
                world.set_material(x, y, MaterialID::Empty);
                return;
            }

            // Ember becomes fire
            if (neighbor == MaterialID::Ember) {
                world.set_material(x + dx, y + dy, MaterialID::Fire);
                world.get_cell(x + dx, y + dy).set_lifetime(25);
                world.set_material(x, y, MaterialID::Empty);
                return;
            }
        }
    }

    generic_gas_update(world, x, y, -1, -10, false);
}

// ============================================================================
// NEW SOLIDS (94-97)
// ============================================================================

// Concrete - indestructible building material
void update_concrete(World& world, int32_t x, int32_t y) {
    // Static solid - does nothing
    (void)world;
    (void)x;
    (void)y;
}

// Titanium - very strong metal, resists acid
void update_titanium(World& world, int32_t x, int32_t y) {
    // Static solid - nearly indestructible
    (void)world;
    (void)x;
    (void)y;
}

// Clay - can be fired into brick by heat
void update_clay(World& world, int32_t x, int32_t y) {
    // Check for heat sources - become brick
    for (int dy = -1; dy <= 1; dy++) {
        for (int dx = -1; dx <= 1; dx++) {
            if (dx == 0 && dy == 0) continue;
            if (!world.in_bounds(x + dx, y + dy)) continue;

            MaterialID neighbor = world.get_material(x + dx, y + dy);
            if (neighbor == MaterialID::Fire || neighbor == MaterialID::Lava ||
                neighbor == MaterialID::Thermite || neighbor == MaterialID::Dragon_Fire) {
                if ((world.random_int() & 15) == 0) {
                    world.set_material(x, y, MaterialID::Brick);
                    return;
                }
            }
        }
    }
}

// Charcoal - slow burning fuel
void update_charcoal(World& world, int32_t x, int32_t y) {
    Cell& cell = world.get_cell(x, y);

    // If burning (lifetime > 0), continue burning
    if (cell.get_lifetime() > 0) {
        cell.decrement_lifetime();

        // Emit ember particles occasionally
        if ((world.random_int() & 15) == 0) {
            for (int dy = -1; dy <= 0; dy++) {
                for (int dx = -1; dx <= 1; dx++) {
                    if (world.in_bounds(x + dx, y + dy) &&
                        world.get_material(x + dx, y + dy) == MaterialID::Empty) {
                        world.set_material(x + dx, y + dy, MaterialID::Ember);
                        world.get_cell(x + dx, y + dy).set_lifetime(10);
                        break;
                    }
                }
            }
        }

        // Spread fire to neighbors
        for (int dy = -1; dy <= 1; dy++) {
            for (int dx = -1; dx <= 1; dx++) {
                if (dx == 0 && dy == 0) continue;
                if (!world.in_bounds(x + dx, y + dy)) continue;

                MaterialID neighbor = world.get_material(x + dx, y + dy);
                if ((neighbor == MaterialID::Charcoal || neighbor == MaterialID::Wood ||
                     neighbor == MaterialID::Coal) && (world.random_int() & 31) == 0) {
                    Cell& n_cell = world.get_cell(x + dx, y + dy);
                    if (n_cell.get_lifetime() == 0) {
                        n_cell.set_lifetime(50);  // Start burning
                    }
                }
            }
        }

        if (cell.get_lifetime() == 0) {
            world.set_material(x, y, MaterialID::Ash);
            return;
        }
    } else {
        // Check for ignition
        for (int dy = -1; dy <= 1; dy++) {
            for (int dx = -1; dx <= 1; dx++) {
                if (dx == 0 && dy == 0) continue;
                if (!world.in_bounds(x + dx, y + dy)) continue;

                MaterialID neighbor = world.get_material(x + dx, y + dy);
                if (neighbor == MaterialID::Fire || neighbor == MaterialID::Lava ||
                    neighbor == MaterialID::Spark || neighbor == MaterialID::Ember) {
                    cell.set_lifetime(60);  // Long burn time
                    return;
                }
            }
        }
    }
}

// ============================================================================
// NEW ORGANIC (98-100)
// ============================================================================

// Bamboo - can grow upward when near water
void update_bamboo(World& world, int32_t x, int32_t y) {
    // Check for fire - burns
    for (int dy = -1; dy <= 1; dy++) {
        for (int dx = -1; dx <= 1; dx++) {
            if (dx == 0 && dy == 0) continue;
            if (!world.in_bounds(x + dx, y + dy)) continue;

            MaterialID neighbor = world.get_material(x + dx, y + dy);
            if (neighbor == MaterialID::Fire || neighbor == MaterialID::Lava) {
                world.set_material(x, y, MaterialID::Fire);
                world.get_cell(x, y).set_lifetime(12);
                return;
            }
        }
    }

    // Grow upward if near water
    if ((world.random_int() & 255) == 0) {
        bool has_water = false;
        for (int dy = -2; dy <= 2; dy++) {
            for (int dx = -2; dx <= 2; dx++) {
                if (world.in_bounds(x + dx, y + dy) &&
                    world.get_material(x + dx, y + dy) == MaterialID::Water) {
                    has_water = true;
                    break;
                }
            }
            if (has_water) break;
        }

        if (has_water && world.in_bounds(x, y - 1) &&
            world.get_material(x, y - 1) == MaterialID::Empty) {
            world.set_material(x, y - 1, MaterialID::Bamboo);
        }
    }
}

// Honeycomb - melts into honey when heated
void update_honeycomb(World& world, int32_t x, int32_t y) {
    // Check for heat - melts into honey
    for (int dy = -1; dy <= 1; dy++) {
        for (int dx = -1; dx <= 1; dx++) {
            if (dx == 0 && dy == 0) continue;
            if (!world.in_bounds(x + dx, y + dy)) continue;

            MaterialID neighbor = world.get_material(x + dx, y + dy);
            if (neighbor == MaterialID::Fire || neighbor == MaterialID::Lava ||
                neighbor == MaterialID::Thermite) {
                if ((world.random_int() & 7) == 0) {
                    world.set_material(x, y, MaterialID::Honey);
                    return;
                }
            }
        }
    }
}

// Bone - static, can be dissolved by acid
void update_bone(World& world, int32_t x, int32_t y) {
    // Dissolves in acid
    for (int dy = -1; dy <= 1; dy++) {
        for (int dx = -1; dx <= 1; dx++) {
            if (dx == 0 && dy == 0) continue;
            if (!world.in_bounds(x + dx, y + dy)) continue;

            MaterialID neighbor = world.get_material(x + dx, y + dy);
            if (neighbor == MaterialID::Acid) {
                if ((world.random_int() & 15) == 0) {
                    world.set_material(x, y, MaterialID::Empty);
                    // Acid consumed
                    world.set_material(x + dx, y + dy, MaterialID::Toxic_Gas);
                    return;
                }
            }
        }
    }
}

// ============================================================================
// NEW SPECIAL (101-102)
// ============================================================================

// Napalm - sticky fire that spreads and clings to surfaces
void update_napalm(World& world, int32_t x, int32_t y) {
    Cell& cell = world.get_cell(x, y);

    // Initialize or continue burning
    if (cell.get_lifetime() == 0) {
        cell.set_lifetime(50 + (world.random_int() & 31));
    }

    cell.decrement_lifetime();

    // Emit fire/smoke
    if ((world.random_int() & 3) == 0) {
        // Try to emit fire upward
        if (world.in_bounds(x, y - 1) && world.get_material(x, y - 1) == MaterialID::Empty) {
            world.set_material(x, y - 1, MaterialID::Fire);
            world.get_cell(x, y - 1).set_lifetime(10);
        }
    }

    // Spread to flammable neighbors aggressively
    for (int dy = -1; dy <= 1; dy++) {
        for (int dx = -1; dx <= 1; dx++) {
            if (dx == 0 && dy == 0) continue;
            if (!world.in_bounds(x + dx, y + dy)) continue;

            MaterialID neighbor = world.get_material(x + dx, y + dy);

            // Spread napalm to flammable materials
            if ((neighbor == MaterialID::Wood || neighbor == MaterialID::Oil ||
                 neighbor == MaterialID::Petrol || neighbor == MaterialID::Tar ||
                 neighbor == MaterialID::Leaf || neighbor == MaterialID::Grass) &&
                (world.random_int() & 7) == 0) {
                world.set_material(x + dx, y + dy, MaterialID::Napalm);
            }

            // Set other flammables on fire
            if ((neighbor == MaterialID::Coal || neighbor == MaterialID::Charcoal ||
                 neighbor == MaterialID::Sawdust || neighbor == MaterialID::Sugar) &&
                (world.random_int() & 3) == 0) {
                world.set_material(x + dx, y + dy, MaterialID::Fire);
                world.get_cell(x + dx, y + dy).set_lifetime(20);
            }
        }
    }

    // Die and become ash/smoke
    if (cell.get_lifetime() == 0) {
        if ((world.random_int() & 1) == 0) {
            world.set_material(x, y, MaterialID::Smoke);
        } else {
            world.set_material(x, y, MaterialID::Ash);
        }
        return;
    }

    // Flow slowly downward like burning liquid
    if ((world.random_int() & 3) == 0) {
        if (world.try_move_cell(x, y, x, y + 1)) return;

        int dir = (world.random_int() & 1) ? 1 : -1;
        if (world.try_move_cell(x, y, x + dir, y + 1)) return;
        if (world.try_move_cell(x, y, x - dir, y + 1)) return;
    }
}

// Thermite - extremely hot burning compound, melts through metal
void update_thermite(World& world, int32_t x, int32_t y) {
    Cell& cell = world.get_cell(x, y);

    // Initialize lifetime if needed
    if (cell.get_lifetime() == 0) {
        cell.set_lifetime(40);
    }

    cell.decrement_lifetime();

    // EXTREMELY hot - melts through almost anything
    for (int dy = -1; dy <= 1; dy++) {
        for (int dx = -1; dx <= 1; dx++) {
            if (dx == 0 && dy == 0) continue;
            if (!world.in_bounds(x + dx, y + dy)) continue;

            MaterialID neighbor = world.get_material(x + dx, y + dy);

            // Melt metals!
            if (neighbor == MaterialID::Metal || neighbor == MaterialID::Iron_Filings ||
                neighbor == MaterialID::Copper) {
                if ((world.random_int() & 3) == 0) {
                    world.set_material(x + dx, y + dy, MaterialID::Lava);
                }
            }

            // Even melts stone and brick
            if (neighbor == MaterialID::Stone || neighbor == MaterialID::Brick ||
                neighbor == MaterialID::Concrete) {
                if ((world.random_int() & 7) == 0) {
                    world.set_material(x + dx, y + dy, MaterialID::Lava);
                }
            }

            // Ignite thermite powder
            if (neighbor == MaterialID::Thermite_Powder) {
                world.set_material(x + dx, y + dy, MaterialID::Thermite);
                world.get_cell(x + dx, y + dy).set_lifetime(40);
            }

            // Instantly ignite flammables
            if (neighbor == MaterialID::Wood || neighbor == MaterialID::Oil ||
                neighbor == MaterialID::Coal || neighbor == MaterialID::Gunpowder) {
                world.set_material(x + dx, y + dy, MaterialID::Fire);
                world.get_cell(x + dx, y + dy).set_lifetime(25);
            }

            // Water creates steam explosion
            if (neighbor == MaterialID::Water) {
                world.set_material(x + dx, y + dy, MaterialID::Steam);
                // Violent steam
                for (int ey = -2; ey <= 2; ey++) {
                    for (int ex = -2; ex <= 2; ex++) {
                        if (world.in_bounds(x + ex, y + ey) &&
                            world.get_material(x + ex, y + ey) == MaterialID::Empty) {
                            world.set_material(x + ex, y + ey, MaterialID::Steam);
                        }
                    }
                }
            }
        }
    }

    // Die - become molten iron (lava-like)
    if (cell.get_lifetime() == 0) {
        world.set_material(x, y, MaterialID::Lava);
        return;
    }

    // Fall slowly (molten)
    if ((world.random_int() & 1) == 0) {
        if (world.try_move_cell(x, y, x, y + 1)) return;

        int dir = (world.random_int() & 1) ? 1 : -1;
        world.try_move_cell(x, y, x + dir, y + 1);
    }
}

// ============================================================================
// EXPANSION MATERIALS (103-161)
// ============================================================================

// === EXPANSION: BASIC (103-112) ===

void update_bedrock(World& world, int32_t x, int32_t y) {
    // Indestructible - does nothing, cannot be damaged
    (void)world; (void)x; (void)y;
}

void update_ceramic(World& world, int32_t x, int32_t y) {
    // Fired clay - static solid, shatters with enough force
    (void)world; (void)x; (void)y;
}

void update_granite(World& world, int32_t x, int32_t y) {
    // Hard igneous rock - static
    (void)world; (void)x; (void)y;
}

void update_marble(World& world, int32_t x, int32_t y) {
    // Polished stone - dissolves slowly in acid
    if ((world.random_int() % 200) == 0) {
        for (int dy = -1; dy <= 1; dy++) {
            for (int dx = -1; dx <= 1; dx++) {
                if (world.in_bounds(x + dx, y + dy) &&
                    world.get_material(x + dx, y + dy) == MaterialID::Acid) {
                    world.set_material(x, y, MaterialID::Empty);
                    return;
                }
            }
        }
    }
}

void update_sandstone(World& world, int32_t x, int32_t y) {
    // Compressed sand - erodes with water contact
    if ((world.random_int() % 500) == 0) {
        for (int dy = -1; dy <= 1; dy++) {
            for (int dx = -1; dx <= 1; dx++) {
                if (world.in_bounds(x + dx, y + dy) &&
                    world.get_material(x + dx, y + dy) == MaterialID::Water) {
                    world.set_material(x, y, MaterialID::Sand);
                    return;
                }
            }
        }
    }
}

void update_limestone(World& world, int32_t x, int32_t y) {
    // Calcium rock - dissolves in acid
    if ((world.random_int() % 100) == 0) {
        for (int dy = -1; dy <= 1; dy++) {
            for (int dx = -1; dx <= 1; dx++) {
                if (world.in_bounds(x + dx, y + dy) &&
                    world.get_material(x + dx, y + dy) == MaterialID::Acid) {
                    world.set_material(x, y, MaterialID::Empty);
                    world.set_material(x + dx, y + dy, MaterialID::Empty);
                    return;
                }
            }
        }
    }
}

void update_slate(World& world, int32_t x, int32_t y) {
    // Layered rock - static
    (void)world; (void)x; (void)y;
}

void update_basalt(World& world, int32_t x, int32_t y) {
    // Dark volcanic rock - static
    (void)world; (void)x; (void)y;
}

void update_quartz_block(World& world, int32_t x, int32_t y) {
    // Crystalline silica - static, glows near magic
    (void)world; (void)x; (void)y;
}

void update_soil(World& world, int32_t x, int32_t y) {
    // Rich earth - can grow plants, falls like powder
    // Check for support
    if (!world.in_bounds(x, y + 1) ||
        world.get_material(x, y + 1) == MaterialID::Empty ||
        world.get_material(x, y + 1) == MaterialID::Water) {
        generic_powder_update(world, x, y, 1, 8);
        return;
    }

    // Grow grass on top if exposed to air
    if ((world.random_int() % 1000) == 0 &&
        world.in_bounds(x, y - 1) &&
        world.get_material(x, y - 1) == MaterialID::Empty) {
        // Check if there's water nearby to grow
        bool has_water = false;
        for (int dy = -2; dy <= 2; dy++) {
            for (int dx = -2; dx <= 2; dx++) {
                if (world.in_bounds(x + dx, y + dy) &&
                    world.get_material(x + dx, y + dy) == MaterialID::Water) {
                    has_water = true;
                    break;
                }
            }
            if (has_water) break;
        }
        if (has_water) {
            world.set_material(x, y, MaterialID::Grass);
        }
    }
}

// === EXPANSION: POWDERS (113-117) ===

void update_flour(World& world, int32_t x, int32_t y) {
    // Explosive when dispersed near fire
    for (int dy = -1; dy <= 1; dy++) {
        for (int dx = -1; dx <= 1; dx++) {
            if (world.in_bounds(x + dx, y + dy)) {
                MaterialID neighbor = world.get_material(x + dx, y + dy);
                if (neighbor == MaterialID::Fire || neighbor == MaterialID::Lava ||
                    neighbor == MaterialID::Spark || neighbor == MaterialID::Ember) {
                    // Explode!
                    world.set_material(x, y, MaterialID::Fire);
                    Cell& cell = world.get_cell(x, y);
                    cell.set_lifetime(15);
                    // Small explosion
                    for (int ey = -2; ey <= 2; ey++) {
                        for (int ex = -2; ex <= 2; ex++) {
                            if (world.in_bounds(x + ex, y + ey)) {
                                MaterialID m = world.get_material(x + ex, y + ey);
                                if (m == MaterialID::Flour) {
                                    world.set_material(x + ex, y + ey, MaterialID::Fire);
                                    world.get_cell(x + ex, y + ey).set_lifetime(12);
                                } else if (m == MaterialID::Empty) {
                                    world.set_material(x + ex, y + ey, MaterialID::Smoke);
                                    world.get_cell(x + ex, y + ey).set_lifetime(20);
                                }
                            }
                        }
                    }
                    return;
                }
            }
        }
    }
    generic_powder_update(world, x, y, 1, 8);
}

void update_sulfur(World& world, int32_t x, int32_t y) {
    // Yellow powder - burns slowly with blue flame
    for (int dy = -1; dy <= 1; dy++) {
        for (int dx = -1; dx <= 1; dx++) {
            if (world.in_bounds(x + dx, y + dy)) {
                MaterialID neighbor = world.get_material(x + dx, y + dy);
                if (neighbor == MaterialID::Fire || neighbor == MaterialID::Lava) {
                    world.set_material(x, y, MaterialID::Fire);
                    world.get_cell(x, y).set_lifetime(40);
                    // Release toxic gas
                    if (world.in_bounds(x, y - 1) && world.get_material(x, y - 1) == MaterialID::Empty) {
                        world.set_material(x, y - 1, MaterialID::Toxic_Gas);
                        world.get_cell(x, y - 1).set_lifetime(60);
                    }
                    return;
                }
            }
        }
    }
    generic_powder_update(world, x, y, 2, 12);
}

void update_cement(World& world, int32_t x, int32_t y) {
    // Hardens when wet
    for (int dy = -1; dy <= 1; dy++) {
        for (int dx = -1; dx <= 1; dx++) {
            if (world.in_bounds(x + dx, y + dy) &&
                world.get_material(x + dx, y + dy) == MaterialID::Water) {
                // Absorb water and harden
                world.set_material(x + dx, y + dy, MaterialID::Empty);
                world.set_material(x, y, MaterialID::Concrete);
                return;
            }
        }
    }
    generic_powder_update(world, x, y, 2, 14);
}

void update_fertilizer(World& world, int32_t x, int32_t y) {
    // Helps plants grow - accelerates nearby organic growth
    if ((world.random_int() % 30) == 0) {
        for (int dy = -2; dy <= 2; dy++) {
            for (int dx = -2; dx <= 2; dx++) {
                if (world.in_bounds(x + dx, y + dy)) {
                    MaterialID neighbor = world.get_material(x + dx, y + dy);
                    // Boost plant growth
                    if (neighbor == MaterialID::Seed) {
                        world.set_material(x + dx, y + dy, MaterialID::Vine);
                        world.set_material(x, y, MaterialID::Empty);
                        return;
                    }
                    if (neighbor == MaterialID::Grass && (world.random_int() % 20) == 0) {
                        if (world.in_bounds(x + dx, y + dy - 1) &&
                            world.get_material(x + dx, y + dy - 1) == MaterialID::Empty) {
                            world.set_material(x + dx, y + dy - 1, MaterialID::Flower);
                        }
                    }
                }
            }
        }
    }
    generic_powder_update(world, x, y, 2, 10);
}

void update_volcanic_ash(World& world, int32_t x, int32_t y) {
    // Hot ash - can ignite and floats briefly
    Cell& cell = world.get_cell(x, y);

    // Slowly cool down
    if (cell.get_lifetime() > 0) {
        cell.decrement_lifetime();
        // Hot ash can ignite things
        if (cell.get_lifetime() > 10) {
            for (int dy = -1; dy <= 1; dy++) {
                for (int dx = -1; dx <= 1; dx++) {
                    if (world.in_bounds(x + dx, y + dy)) {
                        MaterialID neighbor = world.get_material(x + dx, y + dy);
                        if (neighbor == MaterialID::Wood || neighbor == MaterialID::Leaf) {
                            world.set_material(x + dx, y + dy, MaterialID::Fire);
                            world.get_cell(x + dx, y + dy).set_lifetime(20);
                        }
                    }
                }
            }
        }
    }

    generic_powder_update(world, x, y, 1, 6);
}

// === EXPANSION: LIQUIDS (118-122) ===

void update_brine(World& world, int32_t x, int32_t y) {
    // Salt water - evaporates to leave salt
    if ((world.random_int() % 2000) == 0) {
        world.set_material(x, y, MaterialID::Salt);
        return;
    }
    // Flow like water
    if (world.try_move_cell(x, y, x, y + 1)) return;

    int dir = world.get_cell(x, y).get_flow_direction() ? 1 : -1;
    if (!world.try_move_cell(x, y, x + dir, y + 1)) {
        if (!world.try_move_cell(x, y, x - dir, y + 1)) {
            // Spread horizontally
            int spread = 3 + (world.random_int() % 3);
            bool moved = false;
            for (int i = 1; i <= spread && !moved; i++) {
                if (world.try_move_cell(x, y, x + dir * i, y)) {
                    moved = true;
                }
            }
            if (!moved) {
                world.get_cell(x, y).set_flow_direction(!world.get_cell(x, y).get_flow_direction());
            }
        }
    }
}

void update_coffee(World& world, int32_t x, int32_t y) {
    // Brown liquid - stains things, evaporates slowly
    if ((world.random_int() % 3000) == 0) {
        world.set_material(x, y, MaterialID::Steam);
        return;
    }

    // Flow like water
    if (world.try_move_cell(x, y, x, y + 1)) return;

    int dir = world.get_cell(x, y).get_flow_direction() ? 1 : -1;
    if (!world.try_move_cell(x, y, x + dir, y + 1)) {
        if (!world.try_move_cell(x, y, x - dir, y + 1)) {
            int spread = 4;
            bool moved = false;
            for (int i = 1; i <= spread && !moved; i++) {
                if (world.try_move_cell(x, y, x + dir * i, y)) {
                    moved = true;
                }
            }
            if (!moved) {
                world.get_cell(x, y).set_flow_direction(!world.get_cell(x, y).get_flow_direction());
            }
        }
    }
}

void update_soap(World& world, int32_t x, int32_t y) {
    // Bubbly cleaner - creates bubbles, floats on water
    // Chance to create bubble
    if ((world.random_int() % 100) == 0 &&
        world.in_bounds(x, y - 1) && world.get_material(x, y - 1) == MaterialID::Empty) {
        world.set_material(x, y - 1, MaterialID::Steam);  // Bubble effect
        world.get_cell(x, y - 1).set_lifetime(30);
    }

    // Float on water
    if (world.in_bounds(x, y + 1) && world.get_material(x, y + 1) == MaterialID::Water) {
        world.swap_cells(x, y, x, y + 1);
        return;
    }

    generic_slow_liquid_update(world, x, y, 1);
}

void update_paint(World& world, int32_t x, int32_t y) {
    // Colorful liquid - sticks to surfaces
    generic_slow_liquid_update(world, x, y, 2);
}

void update_sewage(World& world, int32_t x, int32_t y) {
    // Gross waste - spawns miasma, kills plants
    if ((world.random_int() % 200) == 0 &&
        world.in_bounds(x, y - 1) && world.get_material(x, y - 1) == MaterialID::Empty) {
        world.set_material(x, y - 1, MaterialID::Miasma);
        world.get_cell(x, y - 1).set_lifetime(40);
    }

    // Kill nearby plants
    if ((world.random_int() % 50) == 0) {
        for (int dy = -1; dy <= 1; dy++) {
            for (int dx = -1; dx <= 1; dx++) {
                if (world.in_bounds(x + dx, y + dy)) {
                    MaterialID neighbor = world.get_material(x + dx, y + dy);
                    if (neighbor == MaterialID::Grass || neighbor == MaterialID::Flower ||
                        neighbor == MaterialID::Leaf || neighbor == MaterialID::Vine) {
                        world.set_material(x + dx, y + dy, MaterialID::Empty);
                    }
                }
            }
        }
    }

    generic_slow_liquid_update(world, x, y, 1);
}

// === EXPANSION: GASES (123-129) ===

void update_ammonia(World& world, int32_t x, int32_t y) {
    // Pungent gas - rises, reacts with acid
    Cell& cell = world.get_cell(x, y);

    // React with acid to neutralize
    for (int dy = -1; dy <= 1; dy++) {
        for (int dx = -1; dx <= 1; dx++) {
            if (world.in_bounds(x + dx, y + dy) &&
                world.get_material(x + dx, y + dy) == MaterialID::Acid) {
                world.set_material(x, y, MaterialID::Empty);
                world.set_material(x + dx, y + dy, MaterialID::Salt);  // Neutralization
                return;
            }
        }
    }

    generic_gas_update(world, x, y, -2, -15, true);
}

void update_carbon_dioxide(World& world, int32_t x, int32_t y) {
    // Heavy gas - sinks and suffocates fire
    Cell& cell = world.get_cell(x, y);

    // Extinguish nearby fire
    for (int dy = -1; dy <= 1; dy++) {
        for (int dx = -1; dx <= 1; dx++) {
            if (world.in_bounds(x + dx, y + dy)) {
                MaterialID neighbor = world.get_material(x + dx, y + dy);
                if (neighbor == MaterialID::Fire || neighbor == MaterialID::Ember) {
                    world.set_material(x + dx, y + dy, MaterialID::Smoke);
                    world.get_cell(x + dx, y + dy).set_lifetime(15);
                }
            }
        }
    }

    // Sink slowly (positive rise = sinking)
    if ((world.random_int() & 1) == 0) {
        if (world.try_move_cell(x, y, x, y + 1)) return;
    }

    // Spread horizontally
    int dir = (world.random_int() & 1) ? 1 : -1;
    if (!world.try_move_cell(x, y, x + dir, y)) {
        world.try_move_cell(x, y, x - dir, y);
    }

    // Dissipate
    if (cell.get_lifetime() > 0) {
        cell.decrement_lifetime();
        if (cell.get_lifetime() == 0) {
            world.set_material(x, y, MaterialID::Empty);
        }
    } else if ((world.random_int() % 500) == 0) {
        world.set_material(x, y, MaterialID::Empty);
    }
}

void update_nitrous(World& world, int32_t x, int32_t y) {
    // Laughing gas - rises fast, harmless
    generic_gas_update(world, x, y, -3, -20, true);
}

void update_steam_hot(World& world, int32_t x, int32_t y) {
    // Scalding steam - damages organic matter
    Cell& cell = world.get_cell(x, y);

    // Damage organic things
    for (int dy = -1; dy <= 1; dy++) {
        for (int dx = -1; dx <= 1; dx++) {
            if (world.in_bounds(x + dx, y + dy)) {
                MaterialID neighbor = world.get_material(x + dx, y + dy);
                if (neighbor == MaterialID::Person || neighbor == MaterialID::Flesh) {
                    // Damage
                    Cell& target = world.get_cell(x + dx, y + dy);
                    target.damage_health(5);
                }
                // Cook food
                if (neighbor == MaterialID::Egg && (world.random_int() % 30) == 0) {
                    world.set_material(x + dx, y + dy, MaterialID::Flesh);  // Cooked egg
                }
            }
        }
    }

    // Cool down to regular steam
    if (cell.get_lifetime() > 0) {
        cell.decrement_lifetime();
        if (cell.get_lifetime() == 0) {
            world.set_material(x, y, MaterialID::Steam);
            return;
        }
    }

    generic_gas_update(world, x, y, -2, -15, false);
}

void update_miasma(World& world, int32_t x, int32_t y) {
    // Disease gas - kills living things
    Cell& cell = world.get_cell(x, y);

    // Damage living things
    for (int dy = -1; dy <= 1; dy++) {
        for (int dx = -1; dx <= 1; dx++) {
            if (world.in_bounds(x + dx, y + dy)) {
                MaterialID neighbor = world.get_material(x + dx, y + dy);
                if (neighbor == MaterialID::Person) {
                    Cell& target = world.get_cell(x + dx, y + dy);
                    target.damage_health(2);
                }
                // Wilt plants
                if ((neighbor == MaterialID::Flower || neighbor == MaterialID::Leaf) &&
                    (world.random_int() % 20) == 0) {
                    world.set_material(x + dx, y + dy, MaterialID::Empty);
                }
            }
        }
    }

    generic_gas_update(world, x, y, -1, -8, true);
}

void update_pheromone(World& world, int32_t x, int32_t y) {
    // Attracts creatures - people move toward it
    Cell& cell = world.get_cell(x, y);

    // Dissipate over time
    if (cell.get_lifetime() > 0) {
        cell.decrement_lifetime();
        if (cell.get_lifetime() == 0) {
            world.set_material(x, y, MaterialID::Empty);
            return;
        }
    } else if ((world.random_int() % 300) == 0) {
        world.set_material(x, y, MaterialID::Empty);
        return;
    }

    generic_gas_update(world, x, y, -1, -5, false);
}

void update_nerve_gas(World& world, int32_t x, int32_t y) {
    // Deadly to life - instant kill on contact
    Cell& cell = world.get_cell(x, y);

    // Kill living things instantly
    for (int dy = -1; dy <= 1; dy++) {
        for (int dx = -1; dx <= 1; dx++) {
            if (world.in_bounds(x + dx, y + dy)) {
                MaterialID neighbor = world.get_material(x + dx, y + dy);
                if (neighbor == MaterialID::Person) {
                    world.set_material(x + dx, y + dy, MaterialID::Bone);
                }
            }
        }
    }

    generic_gas_update(world, x, y, -1, -10, true);
}

// === EXPANSION: SOLIDS (130-136) ===

void update_silver(World& world, int32_t x, int32_t y) {
    // Precious metal - tarnishes slowly
    if ((world.random_int() % 10000) == 0) {
        // Could add tarnish effect
    }
    (void)world; (void)x; (void)y;
}

void update_platinum(World& world, int32_t x, int32_t y) {
    // Rare metal - catalyzes reactions
    (void)world; (void)x; (void)y;
}

void update_lead(World& world, int32_t x, int32_t y) {
    // Heavy soft metal - blocks radiation
    (void)world; (void)x; (void)y;
}

void update_tin(World& world, int32_t x, int32_t y) {
    // Light metal
    (void)world; (void)x; (void)y;
}

void update_zinc(World& world, int32_t x, int32_t y) {
    // Reactive metal - reacts with acid to produce hydrogen
    for (int dy = -1; dy <= 1; dy++) {
        for (int dx = -1; dx <= 1; dx++) {
            if (world.in_bounds(x + dx, y + dy) &&
                world.get_material(x + dx, y + dy) == MaterialID::Acid) {
                world.set_material(x + dx, y + dy, MaterialID::Hydrogen);
                if ((world.random_int() % 10) == 0) {
                    world.set_material(x, y, MaterialID::Empty);
                    return;
                }
            }
        }
    }
}

void update_bronze(World& world, int32_t x, int32_t y) {
    // Copper-tin alloy - static
    (void)world; (void)x; (void)y;
}

void update_steel(World& world, int32_t x, int32_t y) {
    // Iron-carbon alloy - rusts very slowly
    if ((world.random_int() % 20000) == 0) {
        for (int dy = -1; dy <= 1; dy++) {
            for (int dx = -1; dx <= 1; dx++) {
                if (world.in_bounds(x + dx, y + dy) &&
                    world.get_material(x + dx, y + dy) == MaterialID::Water) {
                    world.set_material(x, y, MaterialID::Rust);
                    return;
                }
            }
        }
    }
}

// === EXPANSION: ORGANIC (137-143) ===

void update_pollen(World& world, int32_t x, int32_t y) {
    // Plant reproduction - floats, can grow flowers
    Cell& cell = world.get_cell(x, y);

    // Land on soil/grass to grow flower
    if (world.in_bounds(x, y + 1)) {
        MaterialID below = world.get_material(x, y + 1);
        if (below == MaterialID::Soil || below == MaterialID::Grass || below == MaterialID::Dirt) {
            if ((world.random_int() % 10) == 0) {
                world.set_material(x, y, MaterialID::Flower);
                return;
            }
        }
    }

    // Float like dust
    generic_gas_update(world, x, y, -1, -5, true);
}

void update_root(World& world, int32_t x, int32_t y) {
    // Underground plant - grows through soil, absorbs water
    if ((world.random_int() % 100) == 0) {
        // Grow downward through soil
        int grow_dir = (world.random_int() % 3) - 1;  // -1, 0, or 1
        int gx = x + grow_dir;
        int gy = y + 1;

        if (world.in_bounds(gx, gy)) {
            MaterialID target = world.get_material(gx, gy);
            if (target == MaterialID::Soil || target == MaterialID::Dirt) {
                world.set_material(gx, gy, MaterialID::Root);
            }
        }

        // Absorb nearby water
        for (int dy = -1; dy <= 1; dy++) {
            for (int dx = -1; dx <= 1; dx++) {
                if (world.in_bounds(x + dx, y + dy) &&
                    world.get_material(x + dx, y + dy) == MaterialID::Water) {
                    world.set_material(x + dx, y + dy, MaterialID::Empty);
                    // Could spawn something above ground
                    break;
                }
            }
        }
    }
}

void update_bark(World& world, int32_t x, int32_t y) {
    // Tree skin - flammable, static
    for (int dy = -1; dy <= 1; dy++) {
        for (int dx = -1; dx <= 1; dx++) {
            if (world.in_bounds(x + dx, y + dy)) {
                MaterialID neighbor = world.get_material(x + dx, y + dy);
                if (neighbor == MaterialID::Fire || neighbor == MaterialID::Lava) {
                    world.set_material(x, y, MaterialID::Fire);
                    world.get_cell(x, y).set_lifetime(30);
                    return;
                }
            }
        }
    }
}

void update_fruit(World& world, int32_t x, int32_t y) {
    // Edible plant part - falls, rots over time
    Cell& cell = world.get_cell(x, y);

    // Check if supported
    if (!world.in_bounds(x, y + 1) ||
        world.get_material(x, y + 1) == MaterialID::Empty ||
        world.get_material(x, y + 1) == MaterialID::Water) {
        // Fall
        if (world.try_move_cell(x, y, x, y + 1)) return;
        int dir = (world.random_int() & 1) ? 1 : -1;
        if (world.try_move_cell(x, y, x + dir, y + 1)) return;
    }

    // Rot over time
    if ((world.random_int() % 5000) == 0) {
        world.set_material(x, y, MaterialID::Mud);
    }
}

void update_egg(World& world, int32_t x, int32_t y) {
    // Hatches creatures - falls, breaks on impact
    Cell& cell = world.get_cell(x, y);

    // Check support
    if (!world.in_bounds(x, y + 1) ||
        world.get_material(x, y + 1) == MaterialID::Empty) {
        // Falling
        cell.add_velocity(1);
        cell.clamp_velocity(-8, 8);

        if (world.try_move_cell(x, y, x, y + 1)) {
            // Check for hard landing
            if (cell.velocity_y > 3 && world.in_bounds(x, y + 2) &&
                world.get_material(x, y + 2) != MaterialID::Empty) {
                // Crack! Spawn something
                world.set_material(x, y, MaterialID::Slime);  // Egg goo
                return;
            }
            return;
        }
    }

    // Incubate over time
    if ((world.random_int() % 3000) == 0) {
        // Check if warm
        bool warm = false;
        for (int dy = -1; dy <= 1; dy++) {
            for (int dx = -1; dx <= 1; dx++) {
                if (world.in_bounds(x + dx, y + dy)) {
                    MaterialID neighbor = world.get_material(x + dx, y + dy);
                    if (neighbor == MaterialID::Fire || neighbor == MaterialID::Lava ||
                        neighbor == MaterialID::Steam_Hot) {
                        warm = true;
                        break;
                    }
                }
            }
            if (warm) break;
        }
        if (warm) {
            // Hatch!
            world.set_material(x, y, MaterialID::Person);
            world.get_cell(x, y).set_health(50);
        }
    }
}

void update_web(World& world, int32_t x, int32_t y) {
    // Sticky spider silk - traps things, burns easily
    // Check for fire
    for (int dy = -1; dy <= 1; dy++) {
        for (int dx = -1; dx <= 1; dx++) {
            if (world.in_bounds(x + dx, y + dy)) {
                MaterialID neighbor = world.get_material(x + dx, y + dy);
                if (neighbor == MaterialID::Fire || neighbor == MaterialID::Lava) {
                    world.set_material(x, y, MaterialID::Fire);
                    world.get_cell(x, y).set_lifetime(5);
                    return;
                }
            }
        }
    }
    // Static - trapping handled elsewhere
}

void update_mucus(World& world, int32_t x, int32_t y) {
    // Biological slime - slow liquid
    generic_slow_liquid_update(world, x, y, 4);
}

// === EXPANSION: SPECIAL (144-151) ===

void update_bomb(World& world, int32_t x, int32_t y) {
    // Explodes on impact - check velocity
    Cell& cell = world.get_cell(x, y);

    // Fall with gravity
    if (!world.in_bounds(x, y + 1) ||
        world.get_material(x, y + 1) == MaterialID::Empty) {
        cell.add_velocity(2);
        cell.clamp_velocity(-10, 10);
        if (world.try_move_cell(x, y, x, y + 1)) return;
    }

    // Check for fire nearby to detonate
    bool detonate = false;
    for (int dy = -1; dy <= 1; dy++) {
        for (int dx = -1; dx <= 1; dx++) {
            if (world.in_bounds(x + dx, y + dy)) {
                MaterialID neighbor = world.get_material(x + dx, y + dy);
                if (neighbor == MaterialID::Fire || neighbor == MaterialID::Lava ||
                    neighbor == MaterialID::Spark || neighbor == MaterialID::Lightning) {
                    detonate = true;
                    break;
                }
            }
        }
        if (detonate) break;
    }

    // Impact detection
    if (cell.velocity_y > 5) {
        if (world.in_bounds(x, y + 1) && world.get_material(x, y + 1) != MaterialID::Empty) {
            detonate = true;
        }
    }

    if (detonate) {
        // Medium explosion
        int radius = 8;
        for (int ey = -radius; ey <= radius; ey++) {
            for (int ex = -radius; ex <= radius; ex++) {
                if (ex * ex + ey * ey <= radius * radius) {
                    if (world.in_bounds(x + ex, y + ey)) {
                        MaterialID m = world.get_material(x + ex, y + ey);
                        if (m != MaterialID::Bedrock && m != MaterialID::Obsidian) {
                            if (ex * ex + ey * ey <= (radius/2) * (radius/2)) {
                                world.set_material(x + ex, y + ey, MaterialID::Fire);
                                world.get_cell(x + ex, y + ey).set_lifetime(20);
                            } else if ((world.random_int() % 2) == 0) {
                                world.set_material(x + ex, y + ey, MaterialID::Smoke);
                                world.get_cell(x + ex, y + ey).set_lifetime(30);
                            } else {
                                world.set_material(x + ex, y + ey, MaterialID::Empty);
                            }
                        }
                    }
                }
            }
        }
    }
}

void update_nuke(World& world, int32_t x, int32_t y) {
    // Massive explosion - check for detonation
    Cell& cell = world.get_cell(x, y);

    // Fall
    if (world.try_move_cell(x, y, x, y + 1)) return;

    // Check for fire/detonation
    bool detonate = false;
    for (int dy = -1; dy <= 1; dy++) {
        for (int dx = -1; dx <= 1; dx++) {
            if (world.in_bounds(x + dx, y + dy)) {
                MaterialID neighbor = world.get_material(x + dx, y + dy);
                if (neighbor == MaterialID::Fire || neighbor == MaterialID::Lava ||
                    neighbor == MaterialID::Lightning || neighbor == MaterialID::Plasma) {
                    detonate = true;
                    break;
                }
            }
        }
        if (detonate) break;
    }

    if (detonate) {
        // Massive explosion
        int radius = 40;
        for (int ey = -radius; ey <= radius; ey++) {
            for (int ex = -radius; ex <= radius; ex++) {
                if (ex * ex + ey * ey <= radius * radius) {
                    if (world.in_bounds(x + ex, y + ey)) {
                        MaterialID m = world.get_material(x + ex, y + ey);
                        if (m != MaterialID::Bedrock) {
                            int dist_sq = ex * ex + ey * ey;
                            if (dist_sq <= (radius/3) * (radius/3)) {
                                world.set_material(x + ex, y + ey, MaterialID::Plasma);
                                world.get_cell(x + ex, y + ey).set_lifetime(40);
                            } else if (dist_sq <= (radius/2) * (radius/2)) {
                                world.set_material(x + ex, y + ey, MaterialID::Fire);
                                world.get_cell(x + ex, y + ey).set_lifetime(30);
                            } else if ((world.random_int() % 3) == 0) {
                                world.set_material(x + ex, y + ey, MaterialID::Smoke);
                                world.get_cell(x + ex, y + ey).set_lifetime(50);
                            } else {
                                world.set_material(x + ex, y + ey, MaterialID::Empty);
                            }
                        }
                    }
                }
            }
        }
    }
}

void update_laser(World& world, int32_t x, int32_t y) {
    // Light beam - travels in a direction, melts things
    Cell& cell = world.get_cell(x, y);

    // Laser travels downward, destroying things
    int dir_y = 1;  // Default: down

    // Move in direction
    for (int i = 0; i < 10; i++) {
        int ny = y + dir_y * (i + 1);
        if (!world.in_bounds(x, ny)) break;

        MaterialID target = world.get_material(x, ny);
        if (target == MaterialID::Empty) {
            continue;
        } else if (target == MaterialID::Glass || target == MaterialID::Ice) {
            // Pass through glass/ice
            continue;
        } else if (target == MaterialID::Water) {
            world.set_material(x, ny, MaterialID::Steam);
        } else if (target != MaterialID::Bedrock && target != MaterialID::Obsidian) {
            // Melt/destroy
            if (target == MaterialID::Stone || target == MaterialID::Metal) {
                world.set_material(x, ny, MaterialID::Lava);
            } else {
                world.set_material(x, ny, MaterialID::Fire);
                world.get_cell(x, ny).set_lifetime(15);
            }
            break;
        } else {
            break;
        }
    }

    // Laser dissipates
    cell.decrement_lifetime();
    if (cell.get_lifetime() == 0) {
        world.set_material(x, y, MaterialID::Empty);
    }
}

void update_black_hole(World& world, int32_t x, int32_t y) {
    // Optimized black hole with realistic physics
    // Uses sparse sampling for distant regions to reduce computation

    const int event_horizon = 3;      // Instant destruction radius
    const int accretion_disk = 10;    // Strong spiral effect radius
    const int gravity_well = 35;      // Reduced for performance (was 50)

    // Hawking radiation - very rare emission of particles
    if ((world.random_int() % 2000) == 0) {
        int emit_dir = world.random_int() % 4;
        int ex = x + ((emit_dir == 0) ? -1 : (emit_dir == 1) ? 1 : 0);
        int ey = y + ((emit_dir == 2) ? -1 : (emit_dir == 3) ? 1 : 0);
        if (world.in_bounds(ex, ey) && world.get_material(ex, ey) == MaterialID::Empty) {
            world.set_material(ex, ey, MaterialID::Plasma);
            world.get_cell(ex, ey).set_lifetime(10);
        }
    }

    // Pre-compute squared radii for faster comparison
    const int event_horizon_sq = event_horizon * event_horizon;
    const int accretion_disk_sq = accretion_disk * accretion_disk;
    const int gravity_well_sq = gravity_well * gravity_well;

    // Process in rings from center outward, with decreasing sample density
    for (int dy = -gravity_well; dy <= gravity_well; dy++) {
        for (int dx = -gravity_well; dx <= gravity_well; dx++) {
            if (dx == 0 && dy == 0) continue;

            int dist_sq = dx * dx + dy * dy;

            // Skip outside gravity well (use squared comparison - no sqrt)
            if (dist_sq > gravity_well_sq) continue;

            // Sparse sampling for distant regions - skip most cells far away
            // This dramatically reduces computation for large radius
            if (dist_sq > accretion_disk_sq) {
                // Only process ~1/4 of cells in outer region
                if (((dx + dy) & 1) != 0) continue;  // Checkerboard skip
                if ((world.random_int() & 3) != 0) continue;  // 25% chance
            }

            int px = x + dx;
            int py = y + dy;

            if (!world.in_bounds(px, py)) continue;
            MaterialID m = world.get_material(px, py);

            // Early exit for common empty case
            if (m == MaterialID::Empty) continue;
            if (m == MaterialID::Black_Hole || m == MaterialID::White_Hole ||
                m == MaterialID::Bedrock) continue;

            // EVENT HORIZON - instant destruction (use squared comparison)
            if (dist_sq <= event_horizon_sq) {
                world.set_material(px, py, MaterialID::Empty);
                // Simplified energy burst - less frequent
                if ((world.random_int() % 8) == 0) {
                    int burst_x = x + (world.random_int() % 5) - 2;
                    int burst_y = y + (world.random_int() % 5) - 2;
                    if (world.in_bounds(burst_x, burst_y) &&
                        world.get_material(burst_x, burst_y) == MaterialID::Empty) {
                        world.set_material(burst_x, burst_y, MaterialID::Plasma);
                        world.get_cell(burst_x, burst_y).set_lifetime(8);
                    }
                }
                continue;
            }

            // Compute actual distance only when needed (after all early exits)
            float dist = sqrtf((float)dist_sq);

            // Gravitational pull - simplified inverse square
            float gravity_strength = 400.0f / (float)dist_sq;
            int pull_chance = (int)(gravity_strength);
            if (pull_chance < 1) pull_chance = 1;
            if (pull_chance > 100) pull_chance = 100;

            if ((int)(world.random_int() % 100) >= pull_chance) continue;

            // Movement calculation
            int move_x, move_y;

            if (dist_sq <= accretion_disk_sq) {
                // Accretion disk - spiral motion
                float inv_dist = 1.0f / dist;
                float norm_x = -dx * inv_dist;
                float norm_y = -dy * inv_dist;
                float tang_x = -norm_y;
                float tang_y = norm_x;

                float spiral_factor = (dist / accretion_disk) * 0.6f;
                float radial_factor = 1.0f - spiral_factor;

                float move_x_f = norm_x * radial_factor + tang_x * spiral_factor;
                float move_y_f = norm_y * radial_factor + tang_y * spiral_factor;

                move_x = (move_x_f > 0.3f) ? 1 : (move_x_f < -0.3f) ? -1 : 0;
                move_y = (move_y_f > 0.3f) ? 1 : (move_y_f < -0.3f) ? -1 : 0;

                // Spaghettification - less frequent
                if (dist < 5 && (world.random_int() % 5) == 0) {
                    int stretch_y = (dy > 0) ? py + 1 : py - 1;
                    if (world.in_bounds(px, stretch_y) &&
                        world.get_material(px, stretch_y) == MaterialID::Empty) {
                        world.set_material(px, stretch_y, m);
                    }
                }
            } else {
                // Simple pull toward center
                move_x = (dx > 0) ? -1 : (dx < 0) ? 1 : 0;
                move_y = (dy > 0) ? -1 : (dy < 0) ? 1 : 0;
            }

            // Single move attempt (removed multi-step for performance)
            int new_x = px + move_x;
            int new_y = py + move_y;

            if (world.in_bounds(new_x, new_y)) {
                MaterialID target = world.get_material(new_x, new_y);
                if (target == MaterialID::Empty) {
                    world.swap_cells(px, py, new_x, new_y);
                }
            }
        }
    }
}

void update_white_hole(World& world, int32_t x, int32_t y) {
    // Repels matter
    int push_radius = 15;

    // Push nearby materials away from center
    for (int dy = -push_radius; dy <= push_radius; dy++) {
        for (int dx = -push_radius; dx <= push_radius; dx++) {
            if (dx == 0 && dy == 0) continue;

            int dist_sq = dx * dx + dy * dy;
            if (dist_sq > push_radius * push_radius) continue;
            if ((world.random_int() % 5) != 0) continue;

            int px = x + dx;
            int py = y + dy;

            if (!world.in_bounds(px, py)) continue;
            MaterialID m = world.get_material(px, py);
            if (m == MaterialID::Empty || m == MaterialID::Black_Hole ||
                m == MaterialID::White_Hole || m == MaterialID::Bedrock) continue;

            // Move away from white hole
            int move_x = (dx > 0) ? 1 : (dx < 0) ? -1 : 0;
            int move_y = (dy > 0) ? 1 : (dy < 0) ? -1 : 0;

            int new_x = px + move_x;
            int new_y = py + move_y;

            if (world.in_bounds(new_x, new_y) &&
                world.get_material(new_x, new_y) == MaterialID::Empty) {
                world.swap_cells(px, py, new_x, new_y);
            }
        }
    }
}

void update_acid_gas(World& world, int32_t x, int32_t y) {
    // Corrosive vapor - damages materials
    Cell& cell = world.get_cell(x, y);

    // Corrode nearby materials
    if ((world.random_int() % 10) == 0) {
        for (int dy = -1; dy <= 1; dy++) {
            for (int dx = -1; dx <= 1; dx++) {
                if (world.in_bounds(x + dx, y + dy)) {
                    MaterialID neighbor = world.get_material(x + dx, y + dy);
                    // Corrode metals and organics
                    if (neighbor == MaterialID::Metal || neighbor == MaterialID::Copper ||
                        neighbor == MaterialID::Iron_Filings || neighbor == MaterialID::Flesh ||
                        neighbor == MaterialID::Wood || neighbor == MaterialID::Leaf) {
                        if ((world.random_int() % 5) == 0) {
                            world.set_material(x + dx, y + dy, MaterialID::Empty);
                        }
                    }
                }
            }
        }
    }

    generic_gas_update(world, x, y, -1, -10, true);
}

void update_ice_bomb(World& world, int32_t x, int32_t y) {
    // Freezing explosion
    Cell& cell = world.get_cell(x, y);

    // Fall
    if (world.try_move_cell(x, y, x, y + 1)) return;

    // Check for impact or trigger
    bool detonate = false;
    if (cell.velocity_y > 3) detonate = true;

    for (int dy = -1; dy <= 1; dy++) {
        for (int dx = -1; dx <= 1; dx++) {
            if (world.in_bounds(x + dx, y + dy)) {
                MaterialID neighbor = world.get_material(x + dx, y + dy);
                if (neighbor == MaterialID::Fire || neighbor == MaterialID::Spark) {
                    detonate = true;  // Ironic trigger
                }
            }
        }
    }

    if (detonate) {
        int radius = 12;
        for (int ey = -radius; ey <= radius; ey++) {
            for (int ex = -radius; ex <= radius; ex++) {
                if (ex * ex + ey * ey <= radius * radius) {
                    if (world.in_bounds(x + ex, y + ey)) {
                        MaterialID m = world.get_material(x + ex, y + ey);
                        if (m == MaterialID::Water) {
                            world.set_material(x + ex, y + ey, MaterialID::Ice);
                        } else if (m == MaterialID::Lava) {
                            world.set_material(x + ex, y + ey, MaterialID::Obsidian);
                        } else if (m == MaterialID::Steam || m == MaterialID::Steam_Hot) {
                            world.set_material(x + ex, y + ey, MaterialID::Snow);
                        } else if (m == MaterialID::Fire || m == MaterialID::Ember) {
                            world.set_material(x + ex, y + ey, MaterialID::Empty);
                        } else if (m == MaterialID::Empty && (world.random_int() % 3) == 0) {
                            world.set_material(x + ex, y + ey, MaterialID::Frost);
                            world.get_cell(x + ex, y + ey).set_lifetime(40);
                        }
                    }
                }
            }
        }
        world.set_material(x, y, MaterialID::Ice);
    }
}

void update_fire_bomb(World& world, int32_t x, int32_t y) {
    // Incendiary explosion
    Cell& cell = world.get_cell(x, y);

    // Fall
    if (world.try_move_cell(x, y, x, y + 1)) return;

    // Check for impact
    bool detonate = cell.velocity_y > 3;

    for (int dy = -1; dy <= 1; dy++) {
        for (int dx = -1; dx <= 1; dx++) {
            if (world.in_bounds(x + dx, y + dy)) {
                MaterialID neighbor = world.get_material(x + dx, y + dy);
                if (neighbor == MaterialID::Fire || neighbor == MaterialID::Lava) {
                    detonate = true;
                }
            }
        }
    }

    if (detonate) {
        int radius = 10;
        for (int ey = -radius; ey <= radius; ey++) {
            for (int ex = -radius; ex <= radius; ex++) {
                if (ex * ex + ey * ey <= radius * radius) {
                    if (world.in_bounds(x + ex, y + ey)) {
                        MaterialID m = world.get_material(x + ex, y + ey);
                        if (m != MaterialID::Bedrock && m != MaterialID::Water) {
                            if (ex * ex + ey * ey <= (radius/2) * (radius/2)) {
                                world.set_material(x + ex, y + ey, MaterialID::Napalm);
                                world.get_cell(x + ex, y + ey).set_lifetime(50);
                            } else if (m == MaterialID::Empty || (world.random_int() % 2) == 0) {
                                world.set_material(x + ex, y + ey, MaterialID::Fire);
                                world.get_cell(x + ex, y + ey).set_lifetime(25);
                            }
                        }
                    }
                }
            }
        }
    }
}

// === EXPANSION: FANTASY (152-161) ===

void update_mana(World& world, int32_t x, int32_t y) {
    // Magic energy liquid - glows, powers magical things
    // Flow like water but slower
    if (world.try_move_cell(x, y, x, y + 1)) return;

    int dir = world.get_cell(x, y).get_flow_direction() ? 1 : -1;
    if (!world.try_move_cell(x, y, x + dir, y + 1)) {
        if (!world.try_move_cell(x, y, x - dir, y + 1)) {
            if (!world.try_move_cell(x, y, x + dir, y)) {
                world.get_cell(x, y).set_flow_direction(!world.get_cell(x, y).get_flow_direction());
            }
        }
    }

    // Energize nearby magic things
    if ((world.random_int() % 50) == 0) {
        for (int dy = -1; dy <= 1; dy++) {
            for (int dx = -1; dx <= 1; dx++) {
                if (world.in_bounds(x + dx, y + dy)) {
                    MaterialID neighbor = world.get_material(x + dx, y + dy);
                    if (neighbor == MaterialID::Crystal) {
                        // Power up crystal - create magic effect
                        if (world.in_bounds(x, y - 1) && world.get_material(x, y - 1) == MaterialID::Empty) {
                            world.set_material(x, y - 1, MaterialID::Magic);
                            world.get_cell(x, y - 1).set_lifetime(30);
                        }
                    }
                }
            }
        }
    }
}

void update_mirage(World& world, int32_t x, int32_t y) {
    // Illusory shimmer - fades in and out
    Cell& cell = world.get_cell(x, y);

    if (cell.get_lifetime() > 0) {
        cell.decrement_lifetime();
        if (cell.get_lifetime() == 0) {
            world.set_material(x, y, MaterialID::Empty);
            return;
        }
    } else if ((world.random_int() % 200) == 0) {
        world.set_material(x, y, MaterialID::Empty);
        return;
    }

    // Shimmer around
    if ((world.random_int() % 5) == 0) {
        int dx = (world.random_int() % 3) - 1;
        int dy = (world.random_int() % 3) - 1;
        if (world.in_bounds(x + dx, y + dy) && world.get_material(x + dx, y + dy) == MaterialID::Empty) {
            world.swap_cells(x, y, x + dx, y + dy);
        }
    }
}

void update_holy_water(World& world, int32_t x, int32_t y) {
    // Blessed liquid - purifies cursed things, damages undead
    // Flow like water
    if (world.try_move_cell(x, y, x, y + 1)) return;

    int dir = world.get_cell(x, y).get_flow_direction() ? 1 : -1;
    if (!world.try_move_cell(x, y, x + dir, y + 1)) {
        if (!world.try_move_cell(x, y, x - dir, y + 1)) {
            int spread = 4;
            bool moved = false;
            for (int i = 1; i <= spread && !moved; i++) {
                if (world.try_move_cell(x, y, x + dir * i, y)) {
                    moved = true;
                }
            }
            if (!moved) {
                world.get_cell(x, y).set_flow_direction(!world.get_cell(x, y).get_flow_direction());
            }
        }
    }

    // Purify nearby cursed things
    for (int dy = -1; dy <= 1; dy++) {
        for (int dx = -1; dx <= 1; dx++) {
            if (world.in_bounds(x + dx, y + dy)) {
                MaterialID neighbor = world.get_material(x + dx, y + dy);
                if (neighbor == MaterialID::Cursed) {
                    world.set_material(x + dx, y + dy, MaterialID::Blessed);
                    world.set_material(x, y, MaterialID::Empty);
                    return;
                }
                if (neighbor == MaterialID::Ectoplasm || neighbor == MaterialID::Spirit) {
                    world.set_material(x + dx, y + dy, MaterialID::Empty);
                }
            }
        }
    }
}

void update_cursed(World& world, int32_t x, int32_t y) {
    // Dark corruption - spreads slowly, damages life
    Cell& cell = world.get_cell(x, y);

    // Spread to nearby materials
    if ((world.random_int() % 200) == 0) {
        int dx = (world.random_int() % 3) - 1;
        int dy = (world.random_int() % 3) - 1;
        if (world.in_bounds(x + dx, y + dy)) {
            MaterialID neighbor = world.get_material(x + dx, y + dy);
            // Corrupt organic materials
            if (neighbor == MaterialID::Grass || neighbor == MaterialID::Flower ||
                neighbor == MaterialID::Leaf || neighbor == MaterialID::Wood) {
                world.set_material(x + dx, y + dy, MaterialID::Cursed);
            }
        }
    }

    // Damage nearby people
    for (int dy = -1; dy <= 1; dy++) {
        for (int dx = -1; dx <= 1; dx++) {
            if (world.in_bounds(x + dx, y + dy) &&
                world.get_material(x + dx, y + dy) == MaterialID::Person) {
                world.get_cell(x + dx, y + dy).damage_health(1);
            }
        }
    }
}

void update_blessed(World& world, int32_t x, int32_t y) {
    // Light purification - heals and protects
    // Heal nearby people
    if ((world.random_int() % 30) == 0) {
        for (int dy = -1; dy <= 1; dy++) {
            for (int dx = -1; dx <= 1; dx++) {
                if (world.in_bounds(x + dx, y + dy) &&
                    world.get_material(x + dx, y + dy) == MaterialID::Person) {
                    Cell& target = world.get_cell(x + dx, y + dy);
                    uint8_t health = target.get_health();
                    if (health < 100) {
                        target.set_health(health + 1);
                    }
                }
            }
        }
    }

    // Remove nearby cursed
    for (int dy = -1; dy <= 1; dy++) {
        for (int dx = -1; dx <= 1; dx++) {
            if (world.in_bounds(x + dx, y + dy) &&
                world.get_material(x + dx, y + dy) == MaterialID::Cursed) {
                world.set_material(x + dx, y + dy, MaterialID::Empty);
            }
        }
    }
}

void update_soul(World& world, int32_t x, int32_t y) {
    // Spirit essence - rises slowly, fades
    Cell& cell = world.get_cell(x, y);

    // Fade over time
    if (cell.get_lifetime() > 0) {
        cell.decrement_lifetime();
        if (cell.get_lifetime() == 0) {
            world.set_material(x, y, MaterialID::Empty);
            return;
        }
    } else if ((world.random_int() % 500) == 0) {
        world.set_material(x, y, MaterialID::Empty);
        return;
    }

    // Rise slowly with wandering
    if ((world.random_int() % 3) == 0) {
        int dx = (world.random_int() % 3) - 1;
        if (world.in_bounds(x + dx, y - 1) && world.get_material(x + dx, y - 1) == MaterialID::Empty) {
            world.swap_cells(x, y, x + dx, y - 1);
        }
    }
}

void update_spirit(World& world, int32_t x, int32_t y) {
    // Ghost matter - passes through solids, wanders
    Cell& cell = world.get_cell(x, y);

    // Fade over time
    if (cell.get_lifetime() > 0) {
        cell.decrement_lifetime();
        if (cell.get_lifetime() == 0) {
            world.set_material(x, y, MaterialID::Empty);
            return;
        }
    }

    // Wander randomly, can pass through things
    if ((world.random_int() % 2) == 0) {
        int dx = (world.random_int() % 3) - 1;
        int dy = (world.random_int() % 3) - 1;
        if (world.in_bounds(x + dx, y + dy)) {
            MaterialID target = world.get_material(x + dx, y + dy);
            if (target == MaterialID::Empty) {
                world.swap_cells(x, y, x + dx, y + dy);
            } else if (target != MaterialID::Bedrock && target != MaterialID::Holy_Water) {
                // Phase through - swap positions
                world.swap_cells(x, y, x + dx, y + dy);
            }
        }
    }
}

void update_aether(World& world, int32_t x, int32_t y) {
    // Heavenly gas - rises fast, heals
    Cell& cell = world.get_cell(x, y);

    // Heal nearby people
    if ((world.random_int() % 50) == 0) {
        for (int dy = -1; dy <= 1; dy++) {
            for (int dx = -1; dx <= 1; dx++) {
                if (world.in_bounds(x + dx, y + dy) &&
                    world.get_material(x + dx, y + dy) == MaterialID::Person) {
                    Cell& target = world.get_cell(x + dx, y + dy);
                    uint8_t health = target.get_health();
                    if (health < 100) {
                        target.set_health(health + 2);
                    }
                }
            }
        }
    }

    generic_gas_update(world, x, y, -3, -20, true);
}

void update_nether(World& world, int32_t x, int32_t y) {
    // Hellish gas - sinks, damages
    Cell& cell = world.get_cell(x, y);

    // Damage nearby life
    for (int dy = -1; dy <= 1; dy++) {
        for (int dx = -1; dx <= 1; dx++) {
            if (world.in_bounds(x + dx, y + dy)) {
                MaterialID neighbor = world.get_material(x + dx, y + dy);
                if (neighbor == MaterialID::Person) {
                    world.get_cell(x + dx, y + dy).damage_health(3);
                }
                // Wilt plants
                if ((neighbor == MaterialID::Grass || neighbor == MaterialID::Flower) &&
                    (world.random_int() % 10) == 0) {
                    world.set_material(x + dx, y + dy, MaterialID::Ash);
                }
            }
        }
    }

    // Sink (positive rise speed = sinking)
    if ((world.random_int() & 1) == 0) {
        if (world.try_move_cell(x, y, x, y + 1)) return;
    }

    int dir = (world.random_int() & 1) ? 1 : -1;
    if (!world.try_move_cell(x, y, x + dir, y)) {
        world.try_move_cell(x, y, x - dir, y);
    }

    // Dissipate
    if (cell.get_lifetime() > 0) {
        cell.decrement_lifetime();
        if (cell.get_lifetime() == 0) {
            world.set_material(x, y, MaterialID::Empty);
        }
    }
}

void update_phoenix_ash(World& world, int32_t x, int32_t y) {
    // Rebirth powder - can revive dead things, burns bright
    Cell& cell = world.get_cell(x, y);

    // Check for bones nearby - revive to person
    if ((world.random_int() % 100) == 0) {
        for (int dy = -1; dy <= 1; dy++) {
            for (int dx = -1; dx <= 1; dx++) {
                if (world.in_bounds(x + dx, y + dy) &&
                    world.get_material(x + dx, y + dy) == MaterialID::Bone) {
                    world.set_material(x + dx, y + dy, MaterialID::Person);
                    world.get_cell(x + dx, y + dy).set_health(30);
                    world.set_material(x, y, MaterialID::Fire);
                    world.get_cell(x, y).set_lifetime(20);
                    return;
                }
            }
        }
    }

    // Near fire - burst into flames and respawn
    for (int dy = -1; dy <= 1; dy++) {
        for (int dx = -1; dx <= 1; dx++) {
            if (world.in_bounds(x + dx, y + dy)) {
                MaterialID neighbor = world.get_material(x + dx, y + dy);
                if (neighbor == MaterialID::Fire || neighbor == MaterialID::Lava) {
                    // Burst - create dragon fire
                    world.set_material(x, y, MaterialID::Dragon_Fire);
                    world.get_cell(x, y).set_lifetime(30);
                    return;
                }
            }
        }
    }

    generic_powder_update(world, x, y, 1, 6);
}

} // namespace Materials

} // namespace PixelEngine
