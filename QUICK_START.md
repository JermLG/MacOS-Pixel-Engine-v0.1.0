# Quick Start Guide

Get the Pixel Engine running in under 5 minutes.

---

## Prerequisites Check

```bash
# Verify you have Xcode tools
xcode-select -p
# Should print: /Library/Developer/CommandLineTools or similar

# Verify you're on Apple Silicon
uname -m
# Should print: arm64
```

If either fails, install Xcode Command Line Tools:
```bash
xcode-select --install
```

---

## Build & Run (3 Commands)

```bash
# 1. Navigate to project
cd PixelEngine

# 2. Build
make

# 3. Run
make run
```

That's it! A window should appear with the falling sand simulator.

---

## Controls

| Key | Action |
|-----|--------|
| **Left Mouse** | Place material |
| **Right Mouse** | Erase |
| **1** | Sand |
| **2** | Water |
| **3** | Stone |
| **4** | Steam |
| **Q** | Quit |

---

## What To Try

### 1. Basic Sand
- Press `1` to select sand
- Left-click and drag in the window
- Watch sand fall and pile up

### 2. Water Flow
- Press `2` to select water
- Draw water at the top
- Watch it flow down and spread

### 3. Mixing Materials
- Draw sand walls with `1`
- Pour water with `2`
- Watch water flow around sand

### 4. Steam Rising
- Press `4` to select steam
- Draw steam at the bottom
- Watch it rise to the top

### 5. Build Structures
- Press `3` for stone (immovable)
- Build platforms and walls
- Pour sand and water over them

---

## Troubleshooting

### "make: command not found"
Install Xcode Command Line Tools:
```bash
xcode-select --install
```

### "Build failed" or compilation errors
Clean and rebuild:
```bash
make clean
make
```

### Black screen when running
Check the console output. Look for:
```
Initializing Pixel Engine...
Engine initialized successfully!
```

If you see errors, they'll be printed here.

### Low FPS or laggy
- Reduce the amount of active materials on screen
- The engine is optimized for sparse scenarios
- See OPTIMIZATION_GUIDE.md for performance tips

---

## File Structure

```
PixelEngine/
├── Makefile           ← Build configuration
├── README.md          ← Full documentation
├── QUICK_START.md     ← This file
├── OPTIMIZATION_GUIDE.md  ← Performance guide
│
├── src/               ← Source code
├── include/           ← Headers
├── shaders/           ← Metal shaders
└── build/             ← Build output
    └── PixelEngine    ← Executable
```

---

## Next Steps

1. **Read README.md** - Full documentation and architecture
2. **Experiment** - Try different material combinations
3. **Extend** - Add new materials (see README.md "Extending the Engine")
4. **Optimize** - See OPTIMIZATION_GUIDE.md for performance tips

---

## Quick Modifications

### Change World Size

Edit `include/Types.h`:
```cpp
constexpr int32_t WORLD_WIDTH = 1920;   // Change this
constexpr int32_t WORLD_HEIGHT = 1080;  // Change this
```

Then rebuild: `make clean && make`

### Change Brush Size

Edit `src/main.cpp` in `handle_input()`:
```cpp
int32_t brush_radius = 10;  // Change this (default: 5)
```

### Add Keyboard Shortcuts

Edit `src/Platform.mm` in `keyDown:`:
```objc
case '5':
    _inputState->selected_material = MaterialID::YourNewMaterial;
    break;
```

---

## Getting Help

**Console output** - Run from terminal to see debug info:
```bash
cd build
./PixelEngine
```

**Performance stats** - Printed every second:
```
FPS: 60 | Active chunks: 45 | Updated cells: 12845
```

**Check logs** - macOS Console.app → search for "PixelEngine"

---

## Clean Up

Remove all build files:
```bash
make clean
```

Remove entire build directory:
```bash
rm -rf build
```

---

**Enjoy building your pixel sandbox!**
