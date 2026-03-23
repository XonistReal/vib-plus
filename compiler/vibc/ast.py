from __future__ import annotations

from dataclasses import dataclass, field
from typing import Optional

from .tokens import Span


@dataclass(slots=True)
class Module:
    decls: list["Decl"] = field(default_factory=list)


class Decl:
    span: Span


@dataclass(slots=True)
class ImportDecl(Decl):
    path: str
    span: Span


@dataclass(slots=True)
class TypeField:
    name: str
    type_name: str


@dataclass(slots=True)
class TypeDecl(Decl):
    name: str
    fields: list[TypeField]
    span: Span


@dataclass(slots=True)
class EnumDecl(Decl):
    name: str
    variants: list[str]
    span: Span


@dataclass(slots=True)
class Param:
    name: str
    type_name: str
    mutable: bool


@dataclass(slots=True)
class FnDecl(Decl):
    name: Optional[str]
    params: list[Param]
    return_type: Optional[str]
    body: "BlockStmt"
    span: Span


class Stmt:
    span: Span


@dataclass(slots=True)
class BlockStmt(Stmt):
    stmts: list[Stmt]
    span: Span


@dataclass(slots=True)
class LetStmt(Stmt):
    name: str
    mutable: bool
    value: "Expr"
    span: Span


@dataclass(slots=True)
class AssignStmt(Stmt):
    target: "Expr"
    value: "Expr"
    span: Span


@dataclass(slots=True)
class ExprStmt(Stmt):
    expr: "Expr"
    span: Span


@dataclass(slots=True)
class IfStmt(Stmt):
    cond: "Expr"
    then_block: BlockStmt
    else_block: Optional[BlockStmt]
    span: Span


@dataclass(slots=True)
class WhileStmt(Stmt):
    cond: "Expr"
    body: BlockStmt
    span: Span


@dataclass(slots=True)
class ReturnStmt(Stmt):
    value: Optional["Expr"]
    span: Span


class Expr:
    span: Span


@dataclass(slots=True)
class IdentExpr(Expr):
    name: str
    span: Span


@dataclass(slots=True)
class NumberExpr(Expr):
    value: str
    span: Span


@dataclass(slots=True)
class StringExpr(Expr):
    value: str
    span: Span


@dataclass(slots=True)
class BoolExpr(Expr):
    value: bool
    span: Span


@dataclass(slots=True)
class UnaryExpr(Expr):
    op: str
    rhs: Expr
    span: Span


@dataclass(slots=True)
class BinaryExpr(Expr):
    op: str
    lhs: Expr
    rhs: Expr
    span: Span


@dataclass(slots=True)
class CallExpr(Expr):
    callee: Expr
    args: list[Expr]
    span: Span


@dataclass(slots=True)
class MemberExpr(Expr):
    obj: Expr
    name: str
    span: Span


@dataclass(slots=True)
class StructFieldInit:
    name: str
    value: Expr


@dataclass(slots=True)
class StructLiteralExpr(Expr):
    type_name: str
    fields: list[StructFieldInit]
    span: Span


@dataclass(slots=True)
class FnExpr(Expr):
    fn: FnDecl
    span: Span

