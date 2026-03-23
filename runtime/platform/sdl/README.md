# SDL Platform Backend (Scaffold)

This folder contains the SDL2-backed implementation for `std/game`.

## Purpose

- Provide window creation, input polling, frame timing, and 2D drawing primitives.
- Keep SDL details isolated from the rest of runtime/compiler layers.

## Current status

- `sdl_backend.h`: public runtime-facing interface.
- `sdl_backend.c`: minimal loop and input transition tracking skeleton.
- `smoke_main.c`: quick visual/input test executable.

## Design rules

- No SDL symbols outside this folder.
- Expose plain C ABI to simplify future `vibc` codegen integration.
- `vib_game_run` owns the frame loop and calls a function pointer callback.

## Next tasks

1. Add richer text rendering and font atlas loading.
2. Connect this ABI to `std/game` lowering in compiler/runtime bridge.
3. Add automated smoke test in CI.

## Smoke build (CMake)

From this folder:

```bash
cmake -S . -B build
cmake --build build
./build/vib_sdl_smoke
```

## Smoke build (PowerShell fallback, no CMake)

Use `build-smoke.ps1` to compile directly with `cl` or `clang`:

```powershell
powershell -ExecutionPolicy Bypass -File .\build-smoke.ps1 -SdlRoot "C:\SDL2"
```

Optional flags:

```powershell
powershell -ExecutionPolicy Bypass -File .\build-smoke.ps1 -SdlRoot "C:\SDL2" -Compiler cl -Target all
powershell -ExecutionPolicy Bypass -File .\build-smoke.ps1 -SdlRoot "C:\SDL2" -Compiler cl -Target pong
powershell -ExecutionPolicy Bypass -File .\build-smoke.ps1 -SdlRoot "C:\SDL2" -Compiler clang -Target smoke
```

Targets: `smoke`, `pong`, `flappy`, `chess`, `all`.

The script emits executables into `build-win\` and copies the SDL runtime DLL when found.

