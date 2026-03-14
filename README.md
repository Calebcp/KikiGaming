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

## Team Contribution Notes

- Nouha primarily handled level setup, state reset, and progression flow.
- Cerine primarily handled the main menu layout and menu interaction.
- Abdullah primarily handled the character/scene visual helpers and simple graphics staging.
- Nihad primarily handled the opening story cutscene and early narrative pacing.
- Jasey primarily handled the first wizard dialogue sequence and dialogue flow.
- Caleb served as the main programmer and primarily handled gameplay logic, scene flow, integration, and the main loop.

## Current gameplay foundations

- Title menu scene
- Story intro scene
- First dialogue scene
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

## Next upgrades

- Replace placeholder shapes with sprite sheets
- Add tilemap loader (Tiled JSON)
- Add save/load file for progression
- Add audio SFX + BGM + transitions
