# Vib+ example games

This page summarizes the three reference games under `examples/games/`. They share the same hypothetical **`std/game`** surface ([GAME_API.md](GAME_API.md)).

## Pong (`pong.vp`)

- **Controls**: W/S (left paddle), Up/Down (right paddle), Space resets the ball after a point.
- **Techniques**: Fixed timestep via `frame.dt`, axis-aligned paddle collision, score state on out-of-bounds.

## Chess (`chess.vp`)

- **Controls**: Click a piece, then a destination square.
- **Rules**: Standard piece movement on an 8×8 board with blocking pieces; **no** castling, en passant, or check detection. Pawns that reach the back rank **auto-promote to queen**.
- **Techniques**: Flat `64`‑cell board index, ray casting for sliding pieces (`is_path_clear`), tagged `Cell` union for occupancy.

## Flappy (`flappy.vp`)

- **Controls**: Space or W to flap; Space/Enter after **Game Over** to restart.
- **Techniques**: Gravity integration, circle-vs-rectangle collision, recycling pipe columns with a fixed pool (`[Pipe; 4]`).

## Running now

From `C:\Users\sould\vib-plus`:

```powershell
powershell -ExecutionPolicy Bypass -File ".\compiler\vibc.ps1" run ".\examples\games\pong.vp" --sdl-root "C:\Users\sould\SDL2\SDL2-2.32.10" --compiler cl
powershell -ExecutionPolicy Bypass -File ".\compiler\vibc.ps1" run ".\examples\games\chess.vp" --sdl-root "C:\Users\sould\SDL2\SDL2-2.32.10" --compiler cl
powershell -ExecutionPolicy Bypass -File ".\compiler\vibc.ps1" run ".\examples\games\flappy.vp" --sdl-root "C:\Users\sould\SDL2\SDL2-2.32.10" --compiler cl
```

Build-only:

```powershell
powershell -ExecutionPolicy Bypass -File ".\compiler\vibc.ps1" build ".\examples\games\pong.vp" --sdl-root "C:\Users\sould\SDL2\SDL2-2.32.10" --compiler cl
```
