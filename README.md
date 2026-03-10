# KiKi Game Foundation (C + raylib)

This is a starter foundation for a 2D story game based on your class notes:
- Young man lost in the jungle while trying to get back to his aunt
- Temple and dungeon progression across five levels
- Wizard-guided challenge arc
- Heart bar (lives), map unlock progression, reward item, good/bad ending

## Recommended platform setup

Use this stack for class teams (simple and stable):
- Language: C99
- Framework: raylib (2D/game loop/input/audio)
- Build system: CMake
- IDE: VS Code + C/C++ extension

Why this platform:
- Very lightweight for beginner/intermediate C projects
- Fast iteration for gameplay prototypes
- Easy for teammates to split work by level/state/module

## Project structure

- `src/main.c` : complete playable foundation with scene state machine
- `CMakeLists.txt` : build config using `find_package(raylib REQUIRED)`

## Current gameplay foundations

- Title scene
- Story intro scene
- Map scene with unlocked/completed level boxes
- Level 1: river escape
- Level 2: spider nest combat
- Level 3: rune riddle
- Level 4: snake maze chase
- Level 5: final temple choice
- Heart/life UI and restart flow

## Build and run

### macOS

1. Install dependencies:
   - `brew install raylib cmake`
2. Build:
   - `cmake -S . -B build`
   - `cmake --build build`
3. Run:
   - `./build/kikigame`

### Windows (MSYS2 example)

1. Install `mingw-w64-x86_64-raylib` and `mingw-w64-x86_64-cmake`
2. Build with the same CMake commands

### Linux (Debian/Ubuntu)

1. Install dev tools + raylib package (name varies by distro)
2. Build with same CMake commands

## Team split suggestion

- Teammate A: narrative/dialogue system + cutscenes
- Teammate B: player movement/combat polish
- Teammate C: level scripts (LV1-LV5)
- Teammate D: assets/audio/UI

## Next upgrades

- Replace placeholder shapes with sprite sheets
- Add tilemap loader (Tiled JSON)
- Add save/load file for progression
- Add audio SFX + BGM + transitions
