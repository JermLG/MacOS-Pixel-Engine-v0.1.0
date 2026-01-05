# PixelEngine Material Reference

A comprehensive list of all materials, their properties, behaviors, and possible combinations/interactions.

---

## BASIC MATERIALS

### Empty (Air)
- **State:** Empty
- **Density:** 0.0
- **Color:** Transparent
- **Behavior:** Does nothing, fills space.
- **Combinations:** N/A

### Stone
- **State:** Solid, immovable
- **Density:** 1000.0
- **Color:** Gray
- **Behavior:** Indestructible except by acid (slowly) and Dragon Fire (rarely turns to Lava).
- **Combinations:** 
  - With Magic: Becomes Crystal
  - With Dragon Fire: Rarely turns to Lava

### Sand
- **State:** Powder
- **Density:** 1.5
- **Color:** Sandy yellow
- **Behavior:** Falls, piles, slides diagonally if blocked.
- **Combinations:** 
  - With Water: Mud
  - With Lava: Glass
  - With Magic: Gold

### Water
- **State:** Liquid
- **Density:** 1.0
- **Color:** Blue
- **Behavior:** Flows, spreads, extinguishes fire, turns Lava to Stone and itself to Steam.
- **Combinations:** 
  - With Sand: Mud
  - With Dirt: Mud
  - With Salt: Dissolves salt
  - With Lava: Steam + Obsidian
  - With Acid: Poison
  - With Blood: Water
  - With Honey: Slime
  - With Seed: Flower
  - With Leaf: Algae
  - With Moss: Algae
  - With Stardust: Magic
  - With Dragon Fire: Steam (lots)
  - With Frost: Ice

### Steam
- **State:** Gas
- **Density:** 0.1
- **Color:** Light gray, semi-transparent
- **Behavior:** Rises, permanent gas, does not condense.
- **Combinations:** N/A

### Oil
- **State:** Liquid
- **Density:** 0.8
- **Color:** Dark brown/black
- **Behavior:** Flows slowly, floats on water, flammable.
- **Combinations:** 
  - With Fire/Lava: Ignites

### Fire
- **State:** Gas
- **Density:** 0.05
- **Color:** Bright orange
- **Behavior:** Rises, spreads to flammable materials, has lifetime, turns to Smoke when expired.
- **Combinations:** 
  - With Water: Steam
  - With Alcohol/Petrol: More Fire
  - With Fairy Dust: Magic
  - With Frost: Steam
  - With Void Dust: Consumed
  - With Gunpowder/Hydrogen/Methane: Ignites/explodes

### Wood
- **State:** Solid
- **Density:** 0.6
- **Color:** Brown
- **Behavior:** Burns, floats on water, spreads fire to adjacent wood.
- **Combinations:** 
  - With Lava: Ignites
  - With Acid: Dissolves
  - With Moss/Fungus: Spreads onto wood

### Acid
- **State:** Liquid
- **Density:** 1.2
- **Color:** Bright green
- **Behavior:** Corrodes solids (especially wood, grass, stone slowly), reacts with Lava to create Smoke.
- **Combinations:** 
  - With Water: Poison
  - With Milk: Slime
  - With Mercury: Toxic Gas
  - With Copper/Metal: Rust (+ Toxic Gas for Copper)

### Lava
- **State:** Liquid
- **Density:** 2.5
- **Color:** Bright red-orange
- **Behavior:** Burns, solidifies on contact with water, ignites wood, grass, oil.
- **Combinations:** 
  - With Water: Steam + Obsidian
  - With Sand/Glass Powder: Glass + Stone

---

## POWDERS

### Ash
- **State:** Powder
- **Density:** 0.3
- **Color:** Dark gray
- **Behavior:** Rises slowly, then settles.
- **Combinations:** 
  - With Water: Mud

### Dirt
- **State:** Powder
- **Density:** 1.4
- **Color:** Brown
- **Behavior:** Falls, can combine with water to become mud.
- **Combinations:** 
  - With Water: Mud
  - With Seed: Grass

### Gravel
- **State:** Powder
- **Density:** 2.0
- **Color:** Gray
- **Behavior:** Heavier, falls faster.
- **Combinations:** N/A

### Snow
- **State:** Powder
- **Density:** 0.3
- **Color:** White
- **Behavior:** Melts near fire/lava/Dragon Fire, can combine with itself to become ice.
- **Combinations:** 
  - With Snow: Ice

### Gunpowder
- **State:** Powder
- **Density:** 1.2
- **Color:** Dark gray
- **Behavior:** Explodes on contact with fire/spark/lava/lightning.
- **Combinations:** 
  - With Coal: More Gunpowder
  - With Spark/Fire/Hydrogen/Methane: Ignites/explodes

### Salt
- **State:** Powder
- **Density:** 1.3
- **Color:** White
- **Behavior:** Dissolves in water.
- **Combinations:** 
  - With Water: Dissolves

### Coal
- **State:** Powder
- **Density:** 1.5
- **Color:** Black
- **Behavior:** Burns when ignited, spreads fire to adjacent coal.
- **Combinations:** 
  - With Gunpowder: More Gunpowder
  - With Magic: Diamond

### Rust
- **State:** Powder
- **Density:** 1.8
- **Color:** Rust orange
- **Behavior:** Created from copper/metal + acid/water.
- **Combinations:** N/A

### Sawdust
- **State:** Powder
- **Density:** 0.4
- **Color:** Tan
- **Behavior:** Very flammable.
- **Combinations:** 
  - With Coal: Coal

### Glass Powder
- **State:** Powder
- **Density:** 1.6
- **Color:** Light blue tint
- **Behavior:** Melts into glass near lava.
- **Combinations:** 
  - With Lava: Glass

---

## LIQUIDS

### Honey
- **State:** Liquid
- **Density:** 1.4
- **Color:** Golden
- **Behavior:** Very slow, viscous.
- **Combinations:** 
  - With Water: Slime

### Mud
- **State:** Liquid
- **Density:** 1.5
- **Color:** Dark brown
- **Behavior:** Dries out slowly if not touching water.
- **Combinations:** N/A

### Blood
- **State:** Liquid
- **Density:** 1.05
- **Color:** Dark red
- **Behavior:** Behaves like water but slower.
- **Combinations:** 
  - With Water: Water

### Poison
- **State:** Liquid
- **Density:** 1.1
- **Color:** Purple
- **Behavior:** Kills organic materials.
- **Combinations:** N/A

### Slime
- **State:** Liquid
- **Density:** 1.3
- **Color:** Lime green
- **Behavior:** Thick and slow.
- **Combinations:** N/A

### Milk
- **State:** Liquid
- **Density:** 1.03
- **Color:** White
- **Behavior:** Flows like water.
- **Combinations:** 
  - With Acid: Slime

### Alcohol
- **State:** Liquid
- **Density:** 0.79
- **Color:** Light blue
- **Behavior:** Flammable, flows like water.
- **Combinations:** 
  - With Fire: More Fire

### Mercury
- **State:** Liquid
- **Density:** 13.5
- **Color:** Silver
- **Behavior:** Very dense, sinks through most things.
- **Combinations:** 
  - With Acid: Toxic Gas

### Petrol
- **State:** Liquid
- **Density:** 0.75
- **Color:** Light yellow
- **Behavior:** Very flammable, flows like oil.
- **Combinations:** 
  - With Fire: Explodes/becomes Fire

### Glue
- **State:** Liquid
- **Density:** 1.2
- **Color:** Off-white
- **Behavior:** Very slow, sticky, eventually solidifies.
- **Combinations:** N/A

---

## GASES

### Smoke
- **State:** Gas
- **Density:** 0.08
- **Color:** Gray, semi-transparent
- **Behavior:** Rises slowly, dissipates over time.
- **Combinations:** N/A

### Toxic Gas
- **State:** Gas
- **Density:** 0.07
- **Color:** Green, semi-transparent
- **Behavior:** Rises, dissipates over time, kills people.
- **Combinations:** N/A

### Hydrogen
- **State:** Gas
- **Density:** 0.02
- **Color:** Very faint blue
- **Behavior:** Rises very fast, explosive.
- **Combinations:** 
  - With Spark/Fire/Lava/Lightning: Explodes

### Helium
- **State:** Gas
- **Density:** 0.03
- **Color:** Very faint pink
- **Behavior:** Rises even faster than hydrogen.
- **Combinations:** N/A

### Methane
- **State:** Gas
- **Density:** 0.04
- **Color:** Very faint
- **Behavior:** Flammable, rises.
- **Combinations:** 
  - With Spark/Fire/Lava: Ignites

### Spark
- **State:** Gas
- **Density:** 0.01
- **Color:** Bright yellow
- **Behavior:** Short-lived, moves erratically, ignites flammable materials.
- **Combinations:** 
  - With Gunpowder/Hydrogen/Methane: Ignites/explodes

### Plasma
- **State:** Gas
- **Density:** 0.01
- **Color:** Hot magenta
- **Behavior:** Destroys most things, moves erratically.
- **Combinations:** N/A

### Dust
- **State:** Gas
- **Density:** 0.15
- **Color:** Dusty brown
- **Behavior:** Rises, dissipates over time.
- **Combinations:** N/A

### Spore
- **State:** Gas
- **Density:** 0.12
- **Color:** Green
- **Behavior:** Rises, can land and grow into fungus.
- **Combinations:** N/A

### Confetti
- **State:** Gas
- **Density:** 0.2
- **Color:** Pink (varies)
- **Behavior:** Flutters down slowly, colorful.
- **Combinations:** N/A

---

## SOLIDS

### Grass
- **State:** Solid
- **Density:** 0.8
- **Color:** Green
- **Behavior:** Burns quickly, spreads fire to adjacent grass.
- **Combinations:** 
  - With Seed: Grass
  - With Poison: Killed

### Metal
- **State:** Solid
- **Density:** 7.8
- **Color:** Steel gray
- **Behavior:** Conducts electricity, oxidizes to rust when touching water/acid.
- **Combinations:** 
  - With Acid: Rust
  - With Water: Rust

### Gold
- **State:** Solid
- **Density:** 19.3
- **Color:** Gold
- **Behavior:** Static solid.
- **Combinations:** N/A

### Ice
- **State:** Solid
- **Density:** 0.92
- **Color:** Light blue
- **Behavior:** Melts near heat.
- **Combinations:** N/A

### Glass
- **State:** Solid
- **Density:** 2.5
- **Color:** Very light blue
- **Behavior:** Static solid.
- **Combinations:** N/A

### Brick
- **State:** Solid
- **Density:** 1.9
- **Color:** Brick red
- **Behavior:** Static solid.
- **Combinations:** N/A

### Obsidian
- **State:** Solid
- **Density:** 2.4
- **Color:** Very dark
- **Behavior:** Indestructible solid.
- **Combinations:** N/A

### Diamond
- **State:** Solid
- **Density:** 3.5
- **Color:** Light cyan sparkle
- **Behavior:** Indestructible solid.
- **Combinations:** N/A

### Copper
- **State:** Solid
- **Density:** 8.9
- **Color:** Copper brown
- **Behavior:** Oxidizes to rust when touching water/acid.
- **Combinations:** 
  - With Acid: Rust + Toxic Gas

### Rubber
- **State:** Solid
- **Density:** 1.1
- **Color:** Dark
- **Behavior:** Burns when touching fire/lava.
- **Combinations:** N/A

---

## ORGANIC

### Leaf
- **State:** Powder
- **Density:** 0.2
- **Color:** Green
- **Behavior:** Falls slowly, burns easily.
- **Combinations:** 
  - With Water: Algae

### Moss
- **State:** Solid
- **Density:** 0.5
- **Color:** Dark green
- **Behavior:** Spreads on stone/brick/wood, burns.
- **Combinations:** 
  - With Water: Algae

### Vine
- **State:** Solid
- **Density:** 0.4
- **Color:** Green
- **Behavior:** Grows downward, burns.
- **Combinations:** N/A

### Fungus
- **State:** Solid
- **Density:** 0.6
- **Color:** Purple-tan
- **Behavior:** Spreads, releases spores, infects wood/dirt/grass/flesh.
- **Combinations:** 
  - With Flesh: More Fungus

### Seed
- **State:** Powder
- **Density:** 0.8
- **Color:** Brown
- **Behavior:** Falls, can grow into plants.
- **Combinations:** 
  - With Water: Flower
  - With Dirt: Grass

### Flower
- **State:** Solid
- **Density:** 0.3
- **Color:** Pink
- **Behavior:** Static, burns, can release seeds.
- **Combinations:** N/A

### Algae
- **State:** Liquid
- **Density:** 0.95
- **Color:** Dark green
- **Behavior:** Floats in water, spreads.
- **Combinations:** N/A

### Coral
- **State:** Solid
- **Density:** 1.5
- **Color:** Coral color
- **Behavior:** Grows underwater.
- **Combinations:** N/A

### Wax
- **State:** Solid
- **Density:** 0.9
- **Color:** Off-white/cream
- **Behavior:** Melts near fire, becomes oil.
- **Combinations:** N/A

### Flesh
- **State:** Solid
- **Density:** 1.05
- **Color:** Pink
- **Behavior:** Burns, can be infected by fungus, dissolved by acid.
- **Combinations:** 
  - With Ectoplasm: More Ectoplasm

---

## SPECIAL

### Person
- **State:** Solid (for update order)
- **Density:** 1.0
- **Color:** Bright magenta
- **Behavior:** Autonomous agent, can be damaged by fire, lava, acid, and water (slowly).
- **Combinations:** Healed by Fairy Dust.

### Clone
- **State:** Solid
- **Density:** 1.0
- **Color:** Gray
- **Behavior:** Copies adjacent materials.
- **Combinations:** N/A

### Void
- **State:** Solid
- **Density:** 1000.0
- **Color:** Pure black
- **Behavior:** Destroys everything it touches.
- **Combinations:** N/A

### Fuse
- **State:** Solid
- **Density:** 0.5
- **Color:** Brown rope
- **Behavior:** Burns along a line when ignited.
- **Combinations:** N/A

### TNT
- **State:** Solid
- **Density:** 1.0
- **Color:** Red
- **Behavior:** Explodes when touched by fire/fuse/spark/lava/lightning.
- **Combinations:** N/A

### C4
- **State:** Solid
- **Density:** 1.3
- **Color:** Khaki/tan
- **Behavior:** More powerful explosion than TNT.
- **Combinations:** N/A

### Firework
- **State:** Solid
- **Density:** 0.8
- **Color:** Red
- **Behavior:** Shoots up and explodes into confetti when ignited.
- **Combinations:** N/A

### Lightning
- **State:** Gas
- **Density:** 0.01
- **Color:** Bright yellow
- **Behavior:** Moves down erratically, destroys things, creates fire.
- **Combinations:** N/A

### Portal In/Out
- **State:** Solid
- **Density:** 1.0
- **Color:** Blue/Orange
- **Behavior:** Teleports materials from Portal In to Portal Out.
- **Combinations:** N/A

---

## FANTASY

### Magic
- **State:** Gas
- **Density:** 0.05
- **Color:** Purple
- **Behavior:** Transforms random materials, floats, has lifetime.
- **Combinations:** 
  - With Sand: Gold
  - With Coal: Diamond
  - With Stone: Crystal
  - With Crystal: Diamond

### Crystal
- **State:** Solid
- **Density:** 2.8
- **Color:** Purple crystal
- **Behavior:** Grows slowly, can become diamond with magic.
- **Combinations:** 
  - With Magic: Diamond

### Ectoplasm
- **State:** Liquid
- **Density:** 0.5
- **Color:** Ghostly green
- **Behavior:** Floats up, can phase through some materials, infects flesh.
- **Combinations:** 
  - With Flesh: More Ectoplasm

### Antimatter
- **State:** Liquid
- **Density:** -1.0
- **Color:** Dark purple
- **Behavior:** Destroys everything on contact, rises.
- **Combinations:** N/A

### Fairy Dust
- **State:** Powder
- **Density:** 0.1
- **Color:** Pink
- **Behavior:** Heals people, floats, can become magic with fire.
- **Combinations:** 
  - With Fire: Magic

### Dragon Fire
- **State:** Gas
- **Density:** 0.02
- **Color:** Deep orange-red
- **Behavior:** Super hot, burns even stone (turns to Lava), turns water to steam, spreads to flammable materials.
- **Combinations:** 
  - With Water: Steam (lots)
  - With Stone: Rarely turns to Lava

### Frost
- **State:** Gas
- **Density:** 0.08
- **Color:** Icy blue
- **Behavior:** Freezes water, extinguishes fire/lava, turns lava to obsidian.
- **Combinations:** 
  - With Water: Ice
  - With Fire: Steam
  - With Lava: Obsidian

### Ember
- **State:** Powder
- **Density:** 0.4
- **Color:** Orange glow
- **Behavior:** Falling hot particle, ignites flammable materials.
- **Combinations:** 
  - With Water: Steam + Ash

### Stardust
- **State:** Powder
- **Density:** 0.05
- **Color:** Pale gold
- **Behavior:** Floats gently, sparkles.
- **Combinations:** 
  - With Water: Magic

### Void Dust
- **State:** Powder
- **Density:** 0.15
- **Color:** Very dark purple
- **Behavior:** Slowly erases things it touches, consumes fire/spark.
- **Combinations:** 
  - With Fire/Spark: Consumed

---

**Note:** Many materials have additional behaviors in their update logic (e.g., burning, spreading, floating, rising, falling, transforming, etc.), and some combinations are handled by special update functions rather than the combination table.
