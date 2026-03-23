from __future__ import annotations

from dataclasses import dataclass

from . import ast


@dataclass(slots=True)
class IrGen:
    out: list[str]
    tmp_i: int = 0

    def tmp(self) -> str:
        self.tmp_i += 1
        return f"%t{self.tmp_i}"

    def emit(self, line: str) -> None:
        self.out.append(line)

    def gen_module(self, m: ast.Module) -> str:
        for d in m.decls:
            if isinstance(d, ast.ImportDecl):
                self.emit(f"import {d.path}")
            elif isinstance(d, ast.TypeDecl):
                self.emit(f"type {d.name} {{...}}")
            elif isinstance(d, ast.EnumDecl):
                self.emit(f"enum {d.name} [{', '.join(d.variants)}]")
            elif isinstance(d, ast.FnDecl):
                self.gen_fn(d)
            self.emit("")
        return "\n".join(self.out).rstrip() + "\n"

    def gen_fn(self, fn: ast.FnDecl) -> None:
        name = fn.name or "<lambda>"
        params = ", ".join(("mut " if p.mutable else "") + p.name for p in fn.params)
        self.emit(f"fn {name}({params}) -> {fn.return_type or 'infer'}:")
        for s in fn.body.stmts:
            self.gen_stmt(s, "  ")

    def gen_stmt(self, s: ast.Stmt, ind: str) -> None:
        if isinstance(s, ast.LetStmt):
            v = self.gen_expr(s.value)
            self.emit(f"{ind}{'mut ' if s.mutable else 'let '}{s.name} = {v}")
        elif isinstance(s, ast.AssignStmt):
            lhs = self.gen_expr(s.target)
            rhs = self.gen_expr(s.value)
            self.emit(f"{ind}{lhs} <- {rhs}")
        elif isinstance(s, ast.ExprStmt):
            v = self.gen_expr(s.expr)
            self.emit(f"{ind}eval {v}")
        elif isinstance(s, ast.IfStmt):
            c = self.gen_expr(s.cond)
            self.emit(f"{ind}if {c} then")
            for x in s.then_block.stmts:
                self.gen_stmt(x, ind + "  ")
            if s.else_block is not None:
                self.emit(f"{ind}else")
                for x in s.else_block.stmts:
                    self.gen_stmt(x, ind + "  ")
            self.emit(f"{ind}end_if")
        elif isinstance(s, ast.WhileStmt):
            c = self.gen_expr(s.cond)
            self.emit(f"{ind}while {c} do")
            for x in s.body.stmts:
                self.gen_stmt(x, ind + "  ")
            self.emit(f"{ind}end_while")
        elif isinstance(s, ast.ReturnStmt):
            if s.value is None:
                self.emit(f"{ind}ret")
            else:
                v = self.gen_expr(s.value)
                self.emit(f"{ind}ret {v}")
        else:
            self.emit(f"{ind}; unhandled stmt {type(s).__name__}")

    def gen_expr(self, e: ast.Expr) -> str:
        if isinstance(e, ast.IdentExpr):
            return e.name
        if isinstance(e, ast.NumberExpr):
            return e.value
        if isinstance(e, ast.StringExpr):
            return repr(e.value)
        if isinstance(e, ast.BoolExpr):
            return "true" if e.value else "false"
        if isinstance(e, ast.UnaryExpr):
            r = self.gen_expr(e.rhs)
            t = self.tmp()
            self.emit(f"  {t} = {e.op}{r}")
            return t
        if isinstance(e, ast.BinaryExpr):
            l = self.gen_expr(e.lhs)
            r = self.gen_expr(e.rhs)
            t = self.tmp()
            self.emit(f"  {t} = {l} {e.op} {r}")
            return t
        if isinstance(e, ast.MemberExpr):
            o = self.gen_expr(e.obj)
            t = self.tmp()
            self.emit(f"  {t} = getmember {o}.{e.name}")
            return t
        if isinstance(e, ast.CallExpr):
            callee = self.gen_expr(e.callee)
            args = [self.gen_expr(a) for a in e.args]
            t = self.tmp()
            self.emit(f"  {t} = call {callee}({', '.join(args)})")
            return t
        if isinstance(e, ast.StructLiteralExpr):
            t = self.tmp()
            fields = ", ".join(f"{f.name}: {self.gen_expr(f.value)}" for f in e.fields)
            self.emit(f"  {t} = struct {e.type_name} {{{fields}}}")
            return t
        if isinstance(e, ast.FnExpr):
            t = self.tmp()
            self.emit(f"  {t} = fnexpr")
            return t
        t = self.tmp()
        self.emit(f"  {t} = <unhandled {type(e).__name__}>")
        return t


def module_to_ir(m: ast.Module) -> str:
    return IrGen(out=[]).gen_module(m)

