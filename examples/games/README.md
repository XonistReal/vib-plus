# Vib+ game examples

These programs illustrate **real-time input**, **state machines**, and **simple physics** in Vib+. They require the hypothetical **`std/game`** module described in [docs/GAME_API.md](../../docs/GAME_API.md).

| File | Idea |
|------|------|
| `pong.vp` | Two paddles, ball, bounce, score. |
| `chess.vp` | 8×8 board, click to move, simplified rules (no castling / en passant). |
| `flappy.vp` | Gravity, flap, scrolling pipes, score. |

Run (future toolchain):

```bash
vibc run pong.vp
vibc run chess.vp
vibc run flappy.vp
```
