# Megabonk Copy — Unreal Engine 5.8 Phase 1

This repository is a native Unreal Engine 5.8 C++ movement-and-camera greybox. Phase 1 deliberately contains no combat, enemies, progression, UI, or production art.

## Open and run

1. Install Unreal Engine 5.8 with a C++ toolchain.
2. Right-click `megabonkcopy.uproject` and generate project files (or let the editor build the module).
3. Open the project. The game mode builds the greybox arena from engine primitives at runtime, so no hand-authored binary assets are required.
4. Create and save an **Empty Level** as `Content/Maps/PrototypeArena.umap`, then set it as the Editor Startup Map and Game Default Map if desired. This source-only migration intentionally does not fabricate a `.umap` without the Unreal Editor.
5. Press Play. The runtime arena provides open ground, gentle and steep walkable slopes, a raised platform, a ramp, and obstacle blocks.

Until the map is saved, `/Engine/Maps/Entry` is used as a safe source-controlled fallback and produces the same runtime test arena.

## Controls

- **W / A / S / D:** camera-relative movement
- **Mouse:** camera yaw and pitch
- **Left Shift:** sprint
- **Space:** jump

Movement and camera feel values are exposed as `EditDefaultsOnly` properties on `APhase1Character`. Input mappings are created with Enhanced Input in C++, keeping this clean migration free of generated binary assets.

## Prototype scope

`APrototypeArena` constructs the disposable greybox from Unreal's built-in cube mesh. `APhase1Character` uses `UCharacterMovementComponent`, a spring arm, and a camera. `APrototypeGameMode` selects the character, gives it a reliable elevated spawn, and creates the arena.

Unreal Engine 5.8 and Unreal MCP were not present in the migration environment. Compilation and Play In Editor validation must therefore be completed locally in Unreal Engine 5.8 after saving `PrototypeArena`.
