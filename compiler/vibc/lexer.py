from __future__ import annotations

from dataclasses import dataclass

from .tokens import KEYWORDS, Span, Token, TokenKind


class LexError(Exception):
    pass


@dataclass(slots=True)
class Lexer:
    src: str
    file: str
    i: int = 0
    line: int = 1
    col: int = 1

    def _at_end(self) -> bool:
        return self.i >= len(self.src)

    def _peek(self, n: int = 0) -> str:
        j = self.i + n
        return "\0" if j >= len(self.src) else self.src[j]

    def _advance(self) -> str:
        ch = self._peek()
        self.i += 1
        if ch == "\n":
            self.line += 1
            self.col = 1
        else:
            self.col += 1
        return ch

    def _span(self) -> Span:
        return Span(self.file, self.line, self.col)

    def _match(self, expected: str) -> bool:
        if self._peek() != expected:
            return False
        self._advance()
        return True

    def _skip_ws_and_comments(self) -> None:
        while not self._at_end():
            ch = self._peek()
            if ch in (" ", "\t", "\r", "\n"):
                self._advance()
                continue
            if ch == "/" and self._peek(1) == "/":
                while not self._at_end() and self._peek() != "\n":
                    self._advance()
                continue
            if ch == "/" and self._peek(1) == "*":
                self._advance()
                self._advance()
                while not self._at_end():
                    if self._peek() == "*" and self._peek(1) == "/":
                        self._advance()
                        self._advance()
                        break
                    self._advance()
                continue
            break

    def _ident_or_kw(self) -> Token:
        s = self._span()
        start = self.i
        while self._peek().isalnum() or self._peek() == "_":
            self._advance()
        lex = self.src[start:self.i]
        kind = KEYWORDS.get(lex, TokenKind.IDENT)
        return Token(kind, lex, s)

    def _number(self) -> Token:
        s = self._span()
        start = self.i
        while self._peek().isdigit():
            self._advance()
        if self._peek() == "." and self._peek(1).isdigit():
            self._advance()
            while self._peek().isdigit():
                self._advance()
        return Token(TokenKind.NUMBER, self.src[start:self.i], s)

    def _string(self) -> Token:
        s = self._span()
        self._advance()  # opening quote
        out: list[str] = []
        while not self._at_end() and self._peek() != '"':
            ch = self._advance()
            if ch == "\\":
                esc = self._advance()
                table = {"n": "\n", "r": "\r", "t": "\t", '"': '"', "\\": "\\"}
                out.append(table.get(esc, esc))
            else:
                out.append(ch)
        if self._at_end():
            raise LexError(f"{s.file}:{s.line}:{s.col}: unterminated string literal")
        self._advance()  # closing quote
        return Token(TokenKind.STRING, "".join(out), s)

    def lex(self) -> list[Token]:
        toks: list[Token] = []
        single = {
            "(": TokenKind.LPAREN,
            ")": TokenKind.RPAREN,
            "{": TokenKind.LBRACE,
            "}": TokenKind.RBRACE,
            "[": TokenKind.LBRACKET,
            "]": TokenKind.RBRACKET,
            ",": TokenKind.COMMA,
            ".": TokenKind.DOT,
            ";": TokenKind.SEMI,
            "+": TokenKind.PLUS,
            "*": TokenKind.STAR,
            "/": TokenKind.SLASH,
            "%": TokenKind.PERCENT,
        }
        while True:
            self._skip_ws_and_comments()
            s = self._span()
            if self._at_end():
                toks.append(Token(TokenKind.EOF, "", s))
                return toks
            ch = self._peek()
            if ch.isalpha() or ch == "_":
                toks.append(self._ident_or_kw())
            elif ch.isdigit():
                toks.append(self._number())
            elif ch == '"':
                toks.append(self._string())
            elif ch in single:
                self._advance()
                toks.append(Token(single[ch], ch, s))
            elif ch == ":":
                self._advance()
                if self._match(":"):
                    toks.append(Token(TokenKind.COLON_COLON, "::", s))
                else:
                    toks.append(Token(TokenKind.COLON, ":", s))
            elif ch == "-":
                self._advance()
                if self._match(">"):
                    toks.append(Token(TokenKind.ARROW, "->", s))
                else:
                    toks.append(Token(TokenKind.MINUS, "-", s))
            elif ch == "=":
                self._advance()
                if self._match("="):
                    toks.append(Token(TokenKind.EQ, "==", s))
                elif self._match(">"):
                    toks.append(Token(TokenKind.FAT_ARROW, "=>", s))
                else:
                    toks.append(Token(TokenKind.ASSIGN, "=", s))
            elif ch == "!":
                self._advance()
                if self._match("="):
                    toks.append(Token(TokenKind.NE, "!=", s))
                else:
                    toks.append(Token(TokenKind.BANG, "!", s))
            elif ch == "<":
                self._advance()
                if self._match("="):
                    toks.append(Token(TokenKind.LE, "<=", s))
                else:
                    toks.append(Token(TokenKind.LT, "<", s))
            elif ch == ">":
                self._advance()
                if self._match("="):
                    toks.append(Token(TokenKind.GE, ">=", s))
                else:
                    toks.append(Token(TokenKind.GT, ">", s))
            elif ch == "&":
                self._advance()
                if self._match("&"):
                    toks.append(Token(TokenKind.AND_AND, "&&", s))
                else:
                    raise LexError(f"{s.file}:{s.line}:{s.col}: expected '&' for &&")
            elif ch == "|":
                self._advance()
                if self._match("|"):
                    toks.append(Token(TokenKind.OR_OR, "||", s))
                else:
                    raise LexError(f"{s.file}:{s.line}:{s.col}: expected '|' for ||")
            else:
                raise LexError(f"{s.file}:{s.line}:{s.col}: unexpected character {ch!r}")


def lex_text(src: str, file: str) -> list[Token]:
    return Lexer(src=src, file=file).lex()

