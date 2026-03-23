# Vib+ Language

This document is both:

- the language **spec direction**, and
- a guide to the **currently implemented bootstrap subset** in `vibc`.

## Language goals

- Readable syntax with minimal ceremony.
- Safety-focused defaults.
- Performance-focused compilation model.
- Progressive complexity: easy to start, powerful at scale.

## Source format

- File extension: `.vp`
- Comments: `// line` and `/* block */`
- Identifiers: ASCII letters/digits/`_`, not starting with a digit.

## Core syntax

### Variables

```vib
let name = "vib"
mut score = 0
score = score + 1
```

### Functions

```vib
fn add(a i32, b i32) -> i32 {
    a + b
}
```

Closure-style callback form is also used:

```vib
game.run(win, fn frame {
    update(mut state, frame)
})
```

### Types

Record style:

```vib
type Point {
    x f64,
    y f64,
}
```

Enum style:

```vib
enum Side { White, Black }
```

### Control flow

```vib
if x > 0 {
    say("positive")
} else {
    say("non-positive")
}

while running {
    tick()
}
```

## Operators and expressions

- Arithmetic: `+ - * / %`
- Comparison: `== != < <= > >=`
- Boolean: `&& || !`
- Member access: `obj.field`
- Static/member namespace access: `Type::method`
- Function calls: `fn_name(a, b, c)`

## Error model (design)

Vib+ uses value-based errors (`Result`) instead of hidden exceptions.
The full typed error system is in progress; parser/bootstrap behavior is currently syntax-first.

## Current bootstrap compiler coverage

Implemented now in `vibc`:

- `import`, `type`, `enum`, `fn`
- `let` / `mut`, assignment
- `if` / `else`, `while`, `return`
- calls, member access, struct literals
- token dump, AST dump, textual IR dump

Still incomplete vs final language spec:

- full `match` semantics
- complete generic/type parsing coverage
- semantic analysis and type checking
- native codegen from Vib+ AST/IR (currently game targets use backend mapping)

For practical build/run instructions, see `docs/GETTING_STARTED.md`.
