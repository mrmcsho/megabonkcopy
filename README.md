# Megabonk Copy — Phase 1 Movement Prototype

Unity 6 URP greybox focused exclusively on responsive third-person movement and camera feel.

Open `Assets/_Game/Scenes/PrototypeArena.unity` and press Play. The URP project asset is generated and assigned automatically on first editor import.

## Controls

- **WASD / arrow keys:** camera-relative movement
- **Left Shift:** sprint
- **Space:** jump (includes coyote time and jump buffering)
- **Mouse:** orbit camera
- **Escape:** release cursor
- **Left click:** capture cursor again

All movement and camera tuning values are exposed on the scene's `Prototype Arena Bootstrap` component. Runtime player and arena objects are generated when Play Mode begins to keep the committed scene small and deterministic.
