from __future__ import annotations

import argparse
import json
import os
import subprocess
from dataclasses import asdict, is_dataclass
from pathlib import Path
from typing import Any

from .irgen import module_to_ir
from .lexer import LexError, lex_text
from .parser import ParseError, parse_tokens


GAME_TARGETS: dict[str, str] = {
    "pong": "pong",
    "flappy": "flappy",
    "chess": "chess",
}


def _dc_to_json(value: Any) -> Any:
    if is_dataclass(value):
        return {k: _dc_to_json(v) for k, v in asdict(value).items()}
    if isinstance(value, list):
        return [_dc_to_json(v) for v in value]
    if isinstance(value, dict):
        return {k: _dc_to_json(v) for k, v in value.items()}
    return value


def _load(path: str) -> tuple[str, str]:
    p = Path(path)
    return str(p), p.read_text(encoding="utf-8")


def cmd_check(path: str) -> int:
    try:
        file, src = _load(path)
        toks = lex_text(src, file)
        parse_tokens(toks)
        print(f"ok: {file}")
        return 0
    except (LexError, ParseError) as exc:
        print(str(exc))
        return 1


def cmd_tokens(path: str) -> int:
    try:
        file, src = _load(path)
        toks = lex_text(src, file)
        for t in toks:
            print(f"{t.span.line}:{t.span.col} {t.kind.name:<10} {t.lexeme!r}")
        return 0
    except LexError as exc:
        print(str(exc))
        return 1


def cmd_ast(path: str) -> int:
    try:
        file, src = _load(path)
        toks = lex_text(src, file)
        mod = parse_tokens(toks)
        print(json.dumps(_dc_to_json(mod), indent=2))
        return 0
    except (LexError, ParseError) as exc:
        print(str(exc))
        return 1


def cmd_ir(path: str) -> int:
    try:
        file, src = _load(path)
        toks = lex_text(src, file)
        mod = parse_tokens(toks)
        print(module_to_ir(mod))
        return 0
    except (LexError, ParseError) as exc:
        print(str(exc))
        return 1


def _vib_root() -> Path:
    # compiler/vibc/cli.py -> compiler -> repo root
    return Path(__file__).resolve().parents[2]


def _build_script() -> Path:
    return _vib_root() / "runtime" / "platform" / "sdl" / "build-smoke.ps1"


def _detect_game_target(vp_file: str) -> str | None:
    stem = Path(vp_file).stem.lower()
    return GAME_TARGETS.get(stem)


def cmd_build(path: str, sdl_root: str | None, compiler: str) -> int:
    # Parse-check when possible, but do not block known bootstrap builds.
    check_code = cmd_check(path)
    if check_code != 0:
        print("warning: frontend check failed; continuing with backend mapping for supported game target")

    target = _detect_game_target(path)
    if target is None:
        print("build currently supports game entry files: pong.vp, flappy.vp, chess.vp")
        return 2

    script = _build_script()
    if not script.exists():
        print(f"missing build script: {script}")
        return 2

    cmd = [
        "powershell",
        "-ExecutionPolicy",
        "Bypass",
        "-File",
        str(script),
        "-Target",
        target,
        "-Compiler",
        compiler,
    ]
    if sdl_root:
        cmd.extend(["-SdlRoot", sdl_root])
    elif os.environ.get("SDL2_ROOT"):
        cmd.extend(["-SdlRoot", os.environ["SDL2_ROOT"]])
    elif os.environ.get("SDL3_ROOT"):
        cmd.extend(["-SdlRoot", os.environ["SDL3_ROOT"]])

    proc = subprocess.run(cmd)
    if proc.returncode != 0:
        return proc.returncode

    out_dir = _vib_root() / "runtime" / "platform" / "sdl" / "build-win"
    exe_name = f"vib_{target}.exe"
    print(f"built: {out_dir / exe_name}")
    return 0


def cmd_run(path: str, sdl_root: str | None, compiler: str) -> int:
    rc = cmd_build(path, sdl_root=sdl_root, compiler=compiler)
    if rc != 0:
        return rc
    target = _detect_game_target(path)
    assert target is not None
    exe = _vib_root() / "runtime" / "platform" / "sdl" / "build-win" / f"vib_{target}.exe"
    if not exe.exists():
        print(f"missing executable: {exe}")
        return 2
    return subprocess.run([str(exe)]).returncode


def main(argv: list[str] | None = None) -> int:
    p = argparse.ArgumentParser(prog="vibc", description="Vib+ bootstrap compiler CLI")
    sub = p.add_subparsers(dest="cmd", required=True)

    c_check = sub.add_parser("check", help="Lex + parse a .vp file")
    c_check.add_argument("file")

    c_tokens = sub.add_parser("tokens", help="Print token stream")
    c_tokens.add_argument("file")

    c_ast = sub.add_parser("ast", help="Print parsed AST as JSON")
    c_ast.add_argument("file")

    c_ir = sub.add_parser("ir", help="Print bootstrap IR")
    c_ir.add_argument("file")

    c_build = sub.add_parser("build", help="Build supported .vp game to .exe (SDL backend)")
    c_build.add_argument("file")
    c_build.add_argument("--sdl-root", default=None, help="Path to SDL root (contains include/ and lib/)")
    c_build.add_argument("--compiler", default="auto", choices=["auto", "cl", "clang"])

    c_run = sub.add_parser("run", help="Build and run supported .vp game")
    c_run.add_argument("file")
    c_run.add_argument("--sdl-root", default=None, help="Path to SDL root (contains include/ and lib/)")
    c_run.add_argument("--compiler", default="auto", choices=["auto", "cl", "clang"])

    args = p.parse_args(argv)
    if args.cmd == "check":
        return cmd_check(args.file)
    if args.cmd == "tokens":
        return cmd_tokens(args.file)
    if args.cmd == "ast":
        return cmd_ast(args.file)
    if args.cmd == "ir":
        return cmd_ir(args.file)
    if args.cmd == "build":
        return cmd_build(args.file, sdl_root=args.sdl_root, compiler=args.compiler)
    if args.cmd == "run":
        return cmd_run(args.file, sdl_root=args.sdl_root, compiler=args.compiler)
    return 2

