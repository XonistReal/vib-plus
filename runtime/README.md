# Vib+ Runtime Scaffold

Runtime is authored in-house and only uses platform APIs directly.

## Layers

- `core`: startup, allocator hooks, panic handling
- `platform/windows`: Win32 wrappers for time, file, window, input
- `platform/sdl`: SDL2 backend for `std/game`
- `task`: async scheduler
- `game2d`: software rasterizer and frame presentation

## v0 priorities

1. Console support (`say`, stdin read line).
2. Process exit/error wiring.
3. Frame timer and event pump for `std/game` (SDL first).

