from __future__ import annotations

from dataclasses import dataclass

from . import ast
from .tokens import Span, Token, TokenKind


class ParseError(Exception):
    pass


BIN_PREC: dict[TokenKind, int] = {
    TokenKind.OR_OR: 1,
    TokenKind.AND_AND: 2,
    TokenKind.EQ: 3,
    TokenKind.NE: 3,
    TokenKind.LT: 4,
    TokenKind.LE: 4,
    TokenKind.GT: 4,
    TokenKind.GE: 4,
    TokenKind.PLUS: 5,
    TokenKind.MINUS: 5,
    TokenKind.STAR: 6,
    TokenKind.SLASH: 6,
    TokenKind.PERCENT: 6,
}


@dataclass(slots=True)
class Parser:
    toks: list[Token]
    i: int = 0

    def _peek(self, n: int = 0) -> Token:
        j = self.i + n
        if j >= len(self.toks):
            return self.toks[-1]
        return self.toks[j]

    def _at(self, kind: TokenKind) -> bool:
        return self._peek().kind == kind

    def _take(self) -> Token:
        t = self._peek()
        self.i += 1
        return t

    def _expect(self, kind: TokenKind, msg: str | None = None) -> Token:
        if not self._at(kind):
            t = self._peek()
            note = msg or f"expected {kind.name}, got {t.kind.name}"
            raise ParseError(f"{t.span.file}:{t.span.line}:{t.span.col}: {note}")
        return self._take()

    def parse_module(self) -> ast.Module:
        m = ast.Module()
        while not self._at(TokenKind.EOF):
            if self._at(TokenKind.IMPORT):
                m.decls.append(self._parse_import())
            elif self._at(TokenKind.TYPE):
                m.decls.append(self._parse_type_decl())
            elif self._at(TokenKind.ENUM):
                m.decls.append(self._parse_enum_decl())
            elif self._at(TokenKind.FN):
                m.decls.append(self._parse_fn_decl(named_required=True))
            else:
                t = self._peek()
                raise ParseError(f"{t.span.file}:{t.span.line}:{t.span.col}: expected declaration")
        return m

    def _parse_import(self) -> ast.ImportDecl:
        kw = self._expect(TokenKind.IMPORT)
        parts = [self._expect(TokenKind.IDENT, "expected import path").lexeme]
        while self._at(TokenKind.SLASH):
            self._take()
            parts.append(self._expect(TokenKind.IDENT, "expected path segment").lexeme)
        return ast.ImportDecl(path="/".join(parts), span=kw.span)

    def _parse_type_name(self) -> str:
        allowed = {
            TokenKind.IDENT,
            TokenKind.NUMBER,
            TokenKind.DOT,
            TokenKind.COLON_COLON,
            TokenKind.LBRACKET,
            TokenKind.RBRACKET,
            TokenKind.SEMI,
        }
        stop = {TokenKind.COMMA, TokenKind.RPAREN, TokenKind.RBRACE, TokenKind.ASSIGN}
        parts: list[str] = []
        if self._peek().kind not in allowed:
            t = self._peek()
            raise ParseError(f"{t.span.file}:{t.span.line}:{t.span.col}: expected type")
        while self._peek().kind in allowed and self._peek().kind not in stop:
            parts.append(self._take().lexeme)
        return "".join(parts)

    def _parse_type_decl(self) -> ast.TypeDecl:
        kw = self._expect(TokenKind.TYPE)
        name = self._expect(TokenKind.IDENT, "expected type name").lexeme
        self._expect(TokenKind.LBRACE, "expected '{' in type declaration")
        fields: list[ast.TypeField] = []
        while not self._at(TokenKind.RBRACE):
            fname = self._expect(TokenKind.IDENT, "expected field name").lexeme
            ftype = self._parse_type_name()
            if self._at(TokenKind.COMMA):
                self._take()
            fields.append(ast.TypeField(fname, ftype))
        self._expect(TokenKind.RBRACE)
        return ast.TypeDecl(name=name, fields=fields, span=kw.span)

    def _parse_enum_decl(self) -> ast.EnumDecl:
        kw = self._expect(TokenKind.ENUM)
        name = self._expect(TokenKind.IDENT, "expected enum name").lexeme
        self._expect(TokenKind.LBRACE, "expected '{' in enum declaration")
        variants: list[str] = []
        while not self._at(TokenKind.RBRACE):
            variants.append(self._expect(TokenKind.IDENT, "expected enum variant").lexeme)
            if self._at(TokenKind.COMMA):
                self._take()
        self._expect(TokenKind.RBRACE)
        return ast.EnumDecl(name=name, variants=variants, span=kw.span)

    def _parse_param(self) -> ast.Param:
        mutable = False
        if self._at(TokenKind.MUT):
            self._take()
            mutable = True
        name = self._expect(TokenKind.IDENT, "expected parameter name").lexeme
        ptype = self._parse_type_name()
        return ast.Param(name=name, type_name=ptype, mutable=mutable)

    def _parse_fn_decl(self, named_required: bool) -> ast.FnDecl:
        kw = self._expect(TokenKind.FN)
        name: str | None = None
        params: list[ast.Param] = []
        if self._at(TokenKind.IDENT):
            name = self._take().lexeme
        elif named_required:
            t = self._peek()
            raise ParseError(f"{t.span.file}:{t.span.line}:{t.span.col}: expected function name")
        if self._at(TokenKind.LPAREN):
            self._take()
            if not self._at(TokenKind.RPAREN):
                while True:
                    params.append(self._parse_param())
                    if self._at(TokenKind.COMMA):
                        self._take()
                        continue
                    break
            self._expect(TokenKind.RPAREN)
        elif not named_required and name is not None:
            # closure shorthand: fn x { ... }
            params = [ast.Param(name=name, type_name="", mutable=False)]
            name = None
        else:
            t = self._peek()
            raise ParseError(f"{t.span.file}:{t.span.line}:{t.span.col}: expected '(' after fn")
        ret: str | None = None
        if self._at(TokenKind.ARROW):
            self._take()
            if self._at(TokenKind.LPAREN):
                self._take()
                self._expect(TokenKind.RPAREN)
                ret = "()"
            else:
                ret = self._parse_type_name()
        body = self._parse_block()
        return ast.FnDecl(name=name, params=params, return_type=ret, body=body, span=kw.span)

    def _parse_block(self) -> ast.BlockStmt:
        lb = self._expect(TokenKind.LBRACE, "expected '{' to start block")
        out: list[ast.Stmt] = []
        while not self._at(TokenKind.RBRACE):
            out.append(self._parse_stmt())
        self._expect(TokenKind.RBRACE)
        return ast.BlockStmt(stmts=out, span=lb.span)

    def _parse_stmt(self) -> ast.Stmt:
        if self._at(TokenKind.LET):
            return self._parse_let_stmt(mutable=False)
        if self._at(TokenKind.MUT):
            return self._parse_let_stmt(mutable=True)
        if self._at(TokenKind.IF):
            return self._parse_if_stmt()
        if self._at(TokenKind.WHILE):
            return self._parse_while_stmt()
        if self._at(TokenKind.RETURN):
            return self._parse_return_stmt()

        expr = self._parse_expr()
        if self._at(TokenKind.ASSIGN):
            eq = self._take()
            rhs = self._parse_expr()
            return ast.AssignStmt(target=expr, value=rhs, span=eq.span)
        return ast.ExprStmt(expr=expr, span=expr.span)

    def _parse_let_stmt(self, mutable: bool) -> ast.LetStmt:
        kw = self._take()
        name = self._expect(TokenKind.IDENT, "expected variable name").lexeme
        self._expect(TokenKind.ASSIGN, "expected '=' in binding")
        value = self._parse_expr()
        return ast.LetStmt(name=name, mutable=mutable, value=value, span=kw.span)

    def _parse_if_stmt(self) -> ast.IfStmt:
        kw = self._expect(TokenKind.IF)
        cond = self._parse_expr()
        then_block = self._parse_block()
        else_block = None
        if self._at(TokenKind.ELSE):
            self._take()
            else_block = self._parse_block()
        return ast.IfStmt(cond=cond, then_block=then_block, else_block=else_block, span=kw.span)

    def _parse_while_stmt(self) -> ast.WhileStmt:
        kw = self._expect(TokenKind.WHILE)
        cond = self._parse_expr()
        body = self._parse_block()
        return ast.WhileStmt(cond=cond, body=body, span=kw.span)

    def _parse_return_stmt(self) -> ast.ReturnStmt:
        kw = self._expect(TokenKind.RETURN)
        if self._at(TokenKind.RBRACE):
            return ast.ReturnStmt(value=None, span=kw.span)
        return ast.ReturnStmt(value=self._parse_expr(), span=kw.span)

    def _parse_expr(self, min_prec: int = 0) -> ast.Expr:
        lhs = self._parse_prefix()
        while True:
            t = self._peek()
            if t.kind in (TokenKind.LPAREN, TokenKind.DOT, TokenKind.COLON_COLON):
                lhs = self._parse_postfix(lhs)
                continue
            prec = BIN_PREC.get(t.kind, -1)
            if prec < min_prec:
                break
            op = self._take()
            rhs = self._parse_expr(prec + 1)
            lhs = ast.BinaryExpr(op=op.lexeme, lhs=lhs, rhs=rhs, span=op.span)
        return lhs

    def _parse_prefix(self) -> ast.Expr:
        t = self._peek()
        if t.kind == TokenKind.IDENT:
            # Struct literal: TypeName { field: expr, ... }
            if self._peek(1).kind == TokenKind.LBRACE:
                type_tok = self._take()
                self._take()  # {
                fields: list[ast.StructFieldInit] = []
                while not self._at(TokenKind.RBRACE):
                    fname = self._expect(TokenKind.IDENT, "expected field name").lexeme
                    self._expect(TokenKind.COLON, "expected ':' in struct literal")
                    fvalue = self._parse_expr()
                    if self._at(TokenKind.COMMA):
                        self._take()
                    fields.append(ast.StructFieldInit(name=fname, value=fvalue))
                self._expect(TokenKind.RBRACE)
                return ast.StructLiteralExpr(type_name=type_tok.lexeme, fields=fields, span=type_tok.span)
            tok = self._take()
            return ast.IdentExpr(name=tok.lexeme, span=tok.span)
        if t.kind == TokenKind.NUMBER:
            tok = self._take()
            return ast.NumberExpr(value=tok.lexeme, span=tok.span)
        if t.kind == TokenKind.STRING:
            tok = self._take()
            return ast.StringExpr(value=tok.lexeme, span=tok.span)
        if t.kind == TokenKind.TRUE:
            tok = self._take()
            return ast.BoolExpr(value=True, span=tok.span)
        if t.kind == TokenKind.FALSE:
            tok = self._take()
            return ast.BoolExpr(value=False, span=tok.span)
        if t.kind in (TokenKind.MINUS, TokenKind.BANG):
            op = self._take()
            rhs = self._parse_expr(7)
            return ast.UnaryExpr(op=op.lexeme, rhs=rhs, span=op.span)
        if t.kind == TokenKind.LPAREN:
            self._take()
            e = self._parse_expr()
            self._expect(TokenKind.RPAREN)
            return e
        if t.kind == TokenKind.IF:
            # parse `if ... { ... } else { ... }` as expression-ish placeholder by statement form
            stmt = self._parse_if_stmt()
            return ast.FnExpr(
                fn=ast.FnDecl(name=None, params=[], return_type=None, body=stmt.then_block, span=stmt.span),
                span=stmt.span,
            )
        if t.kind == TokenKind.FN:
            fn = self._parse_fn_decl(named_required=False)
            return ast.FnExpr(fn=fn, span=fn.span)
        raise ParseError(f"{t.span.file}:{t.span.line}:{t.span.col}: expected expression")

    def _parse_postfix(self, lhs: ast.Expr) -> ast.Expr:
        if self._at(TokenKind.DOT) or self._at(TokenKind.COLON_COLON):
            dot = self._take()
            name = self._expect(TokenKind.IDENT, "expected member name").lexeme
            return ast.MemberExpr(obj=lhs, name=name, span=dot.span)
        if self._at(TokenKind.LPAREN):
            lp = self._take()
            args: list[ast.Expr] = []
            if not self._at(TokenKind.RPAREN):
                while True:
                    # Support call-site mut marker: foo(mut x)
                    if self._at(TokenKind.MUT):
                        self._take()
                    args.append(self._parse_expr())
                    if self._at(TokenKind.COMMA):
                        self._take()
                        continue
                    break
            self._expect(TokenKind.RPAREN)
            return ast.CallExpr(callee=lhs, args=args, span=lp.span)
        return lhs


def parse_tokens(tokens: list[Token]) -> ast.Module:
    return Parser(tokens).parse_module()

