# Vib+ Getting Started

This guide is for new users who want to understand Vib+ quickly and run something real.

## 1) What Vib+ is today

- A language design with `.vp` source files.
- A bootstrap compiler CLI (`vibc`) written in Python.
- An SDL2 runtime backend that can build and run the game examples as Windows `.exe`.

## 2) Prerequisites (Windows)

- Python 3.12+
- Visual Studio Build Tools (MSVC v143 or newer)
- SDL2 VC dev package (example path used below):  
  `C:\Users\sould\SDL2\SDL2-2.32.10`

## 3) First commands to try

From project root (`C:\Users\sould\vib-plus`):

```powershell
powershell -ExecutionPolicy Bypass -File ".\compiler\vibc.ps1" check ".\examples\games\pong.vp"
powershell -ExecutionPolicy Bypass -File ".\compiler\vibc.ps1" ir ".\examples\games\pong.vp"
```

## 4) Build a `.vp` game into `.exe`

```powershell
powershell -ExecutionPolicy Bypass -File ".\compiler\vibc.ps1" build ".\examples\games\pong.vp" --sdl-root "C:\Users\sould\SDL2\SDL2-2.32.10" --compiler cl
```

Output:

`C:\Users\sould\vib-plus\runtime\platform\sdl\build-win\vib_pong.exe`

## 5) Build + run directly

```powershell
powershell -ExecutionPolicy Bypass -File ".\compiler\vibc.ps1" run ".\examples\games\flappy.vp" --sdl-root "C:\Users\sould\SDL2\SDL2-2.32.10" --compiler cl
```

## 6) Supported game entry files

- `examples/games/pong.vp`
- `examples/games/flappy.vp`
- `examples/games/chess.vp`

`vibc build` currently maps these entry files to SDL native runners.

## 7) Learning path

1. Read `docs/LANGUAGE.md` for syntax and semantics.
2. Read `examples/games/*.vp` to see larger code patterns.
3. Use `vibc tokens` and `vibc ast` to inspect parser behavior.
4. Use `vibc ir` to inspect lowered textual IR.
5. Build and run games to connect source with runtime behavior.

