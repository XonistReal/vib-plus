# `std/game` Target API

The `.vp` game sources are written against this `std/game` surface.

Today:

- SDL runtime implementations exist in `runtime/platform/sdl/` as native C runners.
- Vib+ `std/game` is still a target API surface for the future full compiler/runtime.

## Types

| Type | Role |
|------|------|
| `game.Window` | OS window with title and pixel size. |
| `game.Canvas` | 2D drawing target (clear, rects, circles, text). |
| `game.Frame` | One tick: `dt` (seconds), `canvas`, `width`, `height`, `mouse`. |
| `game.Key` | Virtual key codes: `W`, `A`, `S`, `D`, `Up`, `Down`, `Left`, `Right`, `Space`, `Escape`, `Enter`, … |
| `game.Mouse` | `x`, `y`, `left_pressed`, `left_released`, `right_pressed`. |
| `game.Color` | RGBA: `Color::rgb(r, g, b)`, `Color::rgba(r, g, b, a)` with `u8` components. |

## Functions

```vib
fn game.window(title str, width u32, height u32) -> game.Window

// Drive the loop until the window closes. Calls `each_frame` every tick.
fn game.run(win game.Window, each_frame fn(game.Frame) -> ()) -> ()

// Input helpers
fn game.Frame::key_held(self, k game.Key) -> bool
fn game.Frame::key_pressed(self, k game.Key) -> bool
fn game.Mouse::in_rect(self, x f64, y f64, w f64, h f64) -> bool
```

## Canvas (subset used in examples)

```vib
fn game.Canvas::clear(self, c game.Color) -> ()
fn game.Canvas::fill_rect(self, x f64, y f64, w f64, h f64, c game.Color) -> ()
fn game.Canvas::stroke_rect(self, x f64, y f64, w f64, h f64, c game.Color, thickness f64) -> ()
fn game.Canvas::fill_circle(self, cx f64, cy f64, r f64, c game.Color) -> ()
fn game.Canvas::line(self, x1 f64, y1 f64, x2 f64, y2 f64, c game.Color, thickness f64) -> ()
fn game.Canvas::text(self, x f64, y f64, s str, size f64, c game.Color) -> ()
```

## Math helpers (from `std/math`)

```vib
fn math.clamp(x f64, lo f64, hi f64) -> f64
fn math.abs(x f64) -> f64
```

These names are the target vocabulary for Vib+ source code and runtime integration.
