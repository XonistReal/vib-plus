# Vib+ Compiler (`vibc`)

This folder contains the bootstrap compiler implementation.

## Planned modules

- `lexer/` tokenization and spans
- `parser/` AST construction
- `sema/` name + type analysis
- `ir/` Vib IR
- `codegen/` backend(s)
- `cli/` `vibc` entry commands

## v0 command targets

- `vibc check file.vp`
- `vibc build file.vp`
- `vibc run file.vp`

## Commands

The bootstrap CLI currently provides:

- `vibc check <file.vp>`: lex + parse validation
- `vibc tokens <file.vp>`: token stream dump
- `vibc ast <file.vp>`: AST JSON dump
- `vibc ir <file.vp>`: bootstrap textual IR dump
- `vibc build <file.vp>`: build supported Vib+ game source to `.exe` via SDL backend
- `vibc run <file.vp>`: build and run supported Vib+ game source

Run with wrapper script:

```powershell
powershell -ExecutionPolicy Bypass -File .\vibc.ps1 check ..\examples\games\pong.vp
```

or directly:

```powershell
python -m vibc check ..\examples\games\pong.vp
```

Build game executable from `.vp` form:

```powershell
python -m vibc build ..\examples\games\pong.vp --sdl-root C:\Users\sould\SDL2\SDL2-2.32.10
python -m vibc run ..\examples\games\flappy.vp --sdl-root C:\Users\sould\SDL2\SDL2-2.32.10
```

## Current limits

- `build`/`run` are currently wired for the three game entry files:
  - `pong.vp`
  - `flappy.vp`
  - `chess.vp`
- Parser and semantic coverage are still expanding toward full language support.

