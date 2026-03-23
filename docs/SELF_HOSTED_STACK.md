# Vib+ Core Stack (Compiler + Runtime In-House)

This document defines what we build ourselves, while allowing SDL for graphics.

## Constraint

- Compiler, runtime, stdlib, and tooling logic are authored in this repository.
- SDL2 is allowed for graphics/input/windowing in `std/game`.
- No third-party parser generators or language frameworks.

## Practical meaning

- Language pipeline remains fully ours: lexer, parser, type checker, IR, codegen, runtime.
- Graphics backend can be SDL2-backed for faster delivery and portability.
- Console/runtime features can still use direct OS APIs where useful.

## Target architecture

1. Frontend
   - Handwritten lexer.
   - Handwritten recursive-descent parser.
   - Typed AST/HIR.
2. Backend
   - Vib IR (our own SSA-like IR).
   - x86-64 codegen (Windows first).
   - PE object emission or direct assembly emission.
3. Runtime
   - Startup, allocator, panic/error paths.
   - Green-task scheduler for async.
   - Optional GC (phase 2) implemented in-house.
4. Stdlib
   - `std/io`, `std/fs`, `std/net`, `std/console`, `std/game`.
5. Graphics
   - `std/game` backed by SDL2 first.
   - Optional native backend later (Win32/software rasterizer) behind same API.

## Why this is feasible

- Console apps only need stdlib + runtime wrappers.
- SDL2 gives us stable cross-platform window/input/audio quickly.
- We keep control of language architecture while avoiding graphics reinvention.

## Non-goals for v0

- 3D graphics.
- GPU API abstraction.
- Full optimizing compiler parity with mature ecosystems.

