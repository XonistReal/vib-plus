# Vib+

**Vib+** is a language project focused on readability, safety defaults, and high performance.  
This repository includes:

- language docs,
- a bootstrap compiler (`vibc`) frontend,
- SDL-backed game runtime,
- and `.vp` game sources that can be built to `.exe`.

## AI Attribution

This project is **AI-assisted and AI-authored in significant parts**.

- Core architecture, compiler bootstrap, runtime scaffolding, docs, and game runtime code were generated and iterated with AI tooling.
- Human direction, review, and product decisions still define the project goals.

See `docs/AI_DISCLOSURE.md` for details.

## Contents

| Path | Description |
|------|-------------|
| [docs/GETTING_STARTED.md](docs/GETTING_STARTED.md) | Best starting point for new users |
| [docs/AI_DISCLOSURE.md](docs/AI_DISCLOSURE.md) | Transparent AI authorship and contribution disclosure |
| [docs/LANGUAGE.md](docs/LANGUAGE.md) | Vib+ syntax and semantics (spec + bootstrap status) |
| [docs/GAME_API.md](docs/GAME_API.md) | `std/game` target API and SDL runtime mapping |
| [docs/GAMES.md](docs/GAMES.md) | Game examples and controls |
| [docs/SELF_HOSTED_STACK.md](docs/SELF_HOSTED_STACK.md) | In-house compiler/runtime + SDL graphics policy |
| [docs/SDL_INTEGRATION.md](docs/SDL_INTEGRATION.md) | `std/game` to SDL2 implementation mapping |
| [docs/BUILD_ROADMAP.md](docs/BUILD_ROADMAP.md) | Concrete implementation timeline |
| [examples/games/](examples/games/) | Pong, chess, and Flappy-style source (`.vp`) |
| [compiler/README.md](compiler/README.md) | `vibc` commands, status, and limits |

## Quick Start

From PowerShell:

```powershell
powershell -ExecutionPolicy Bypass -File "C:\Users\sould\vib-plus\compiler\vibc.ps1" build "C:\Users\sould\vib-plus\examples\games\pong.vp" --sdl-root "C:\Users\sould\SDL2\SDL2-2.32.10" --compiler cl
powershell -ExecutionPolicy Bypass -File "C:\Users\sould\vib-plus\compiler\vibc.ps1" run "C:\Users\sould\vib-plus\examples\games\flappy.vp" --sdl-root "C:\Users\sould\SDL2\SDL2-2.32.10" --compiler cl
```

Built executables are written to:

`C:\Users\sould\vib-plus\runtime\platform\sdl\build-win`

## Current project status

- `vibc check/tokens/ast/ir` works on the implemented parser subset.
- `vibc build/run` supports game entry files: `pong.vp`, `flappy.vp`, `chess.vp`.
- Runtime is SDL2-backed and builds GUI executables (no console window).

## Repo update helper

To commit and push updates quickly:

```powershell
powershell -ExecutionPolicy Bypass -File "C:\Users\sould\vib-plus\update-repo.ps1" -Message "your commit message"
```

Optional:

```powershell
powershell -ExecutionPolicy Bypass -File "C:\Users\sould\vib-plus\update-repo.ps1" -Message "update docs" -GitName "xonist" -GitEmail "xonistrblx@gmail.com"
powershell -ExecutionPolicy Bypass -File "C:\Users\sould\vib-plus\update-repo.ps1" -Message "update docs" -PullFirst
powershell -ExecutionPolicy Bypass -File "C:\Users\sould\vib-plus\update-repo.ps1" -Message "wip local" -NoPush
```
