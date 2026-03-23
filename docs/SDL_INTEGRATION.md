# Vib+ SDL Integration Plan

This document defines how `std/game` maps to SDL2.

## Scope

- Graphics, input, window, and timing use SDL2.
- Vib+ compiler/runtime remain in-house.
- Console applications do not depend on SDL2.

## Module mapping

- `std/game.window(title, w, h)` -> `SDL_CreateWindow` + `SDL_CreateRenderer`
- `game.run(win, each_frame)` -> event pump + frame timer + callback invocation
- `Frame.key_held` / `key_pressed` -> derived from SDL keyboard state + transition table
- `Frame.mouse` -> `SDL_GetMouseState` + button transitions

## Drawing mapping

- `Canvas.clear` -> `SDL_SetRenderDrawColor` + `SDL_RenderClear`
- `fill_rect` / `stroke_rect` / `line` -> SDL render primitives
- `fill_circle` -> software circle raster loop (in our code) on top of SDL renderer
- `text` -> initial bitmap font atlas in repository (our asset + renderer code)

## Event loop contract

Per frame:

1. Poll all SDL events.
2. Update input transitions.
3. Compute `dt` from high-resolution counter.
4. Call Vib+ callback with `Frame`.
5. Present renderer.

## Build expectations

- Windows first: ship SDL2 development DLL/import libs.
- `vibc run` links runtime SDL platform layer.
- Keep SDL isolated under `runtime/platform/sdl/*` so alternate backends remain possible.

## Current scaffold

- `runtime/platform/sdl/sdl_backend.h`
- `runtime/platform/sdl/sdl_backend.c`
- `runtime/platform/sdl/CMakeLists.txt`

This scaffold already includes:

- `vib_sdl_init` / `vib_sdl_shutdown`
- `vib_window_create` / `vib_window_destroy`
- `vib_game_run` with `dt` and input transitions
- `vib_canvas_clear` / `vib_canvas_present`
- `vib_canvas_fill_rect`, `vib_canvas_stroke_rect`, `vib_canvas_line`
- `vib_canvas_fill_circle`, `vib_canvas_text` (built-in 5x7 bitmap glyph set)
- `vib_frame_request_quit` for controlled loop shutdown from callbacks

## Why users can build games now

`vibc build` / `vibc run` currently map recognized game entry files (`pong.vp`, `flappy.vp`, `chess.vp`) to this SDL backend.
This is a bootstrap bridge while full Vib+ native code generation is still being developed.

