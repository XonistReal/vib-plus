from __future__ import annotations

from dataclasses import dataclass
from enum import Enum, auto


class TokenKind(Enum):
    EOF = auto()
    IDENT = auto()
    NUMBER = auto()
    STRING = auto()

    LPAREN = auto()
    RPAREN = auto()
    LBRACE = auto()
    RBRACE = auto()
    LBRACKET = auto()
    RBRACKET = auto()
    COMMA = auto()
    DOT = auto()
    COLON = auto()
    COLON_COLON = auto()
    SEMI = auto()

    PLUS = auto()
    MINUS = auto()
    STAR = auto()
    SLASH = auto()
    PERCENT = auto()
    ASSIGN = auto()
    BANG = auto()

    EQ = auto()
    NE = auto()
    LT = auto()
    LE = auto()
    GT = auto()
    GE = auto()
    AND_AND = auto()
    OR_OR = auto()
    ARROW = auto()  # ->
    FAT_ARROW = auto()  # =>

    IMPORT = auto()
    TYPE = auto()
    ENUM = auto()
    FN = auto()
    LET = auto()
    MUT = auto()
    IF = auto()
    ELSE = auto()
    WHILE = auto()
    FOR = auto()
    IN = auto()
    RETURN = auto()
    MATCH = auto()
    TRUE = auto()
    FALSE = auto()


KEYWORDS: dict[str, TokenKind] = {
    "import": TokenKind.IMPORT,
    "type": TokenKind.TYPE,
    "enum": TokenKind.ENUM,
    "fn": TokenKind.FN,
    "let": TokenKind.LET,
    "mut": TokenKind.MUT,
    "if": TokenKind.IF,
    "else": TokenKind.ELSE,
    "while": TokenKind.WHILE,
    "for": TokenKind.FOR,
    "in": TokenKind.IN,
    "return": TokenKind.RETURN,
    "match": TokenKind.MATCH,
    "true": TokenKind.TRUE,
    "false": TokenKind.FALSE,
}


@dataclass(frozen=True, slots=True)
class Span:
    file: str
    line: int
    col: int


@dataclass(frozen=True, slots=True)
class Token:
    kind: TokenKind
    lexeme: str
    span: Span

