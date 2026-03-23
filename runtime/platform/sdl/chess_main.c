#include "sdl_backend.h"

#include <stdio.h>

typedef enum Side { SIDE_WHITE = 0, SIDE_BLACK = 1 } Side;
typedef enum Kind { K_P = 0, K_R, K_N, K_B, K_Q, K_K } Kind;

typedef struct Piece {
    int empty;
    Side side;
    Kind kind;
} Piece;

typedef struct ChessState {
    Piece board[64];
    Side turn;
    int sel;
    float square;
    float field;
} ChessState;

static int idx(int r, int c) { return r * 8 + c; }
static int row_of(int i) { return i / 8; }
static int col_of(int i) { return i % 8; }
static int abs_i(int x) { return x < 0 ? -x : x; }
static int sign_i(int x) { return x < 0 ? -1 : (x > 0 ? 1 : 0); }

static Piece empty_piece(void) {
    Piece p;
    p.empty = 1;
    p.side = SIDE_WHITE;
    p.kind = K_P;
    return p;
}

static Piece make_piece(Side s, Kind k) {
    Piece p;
    p.empty = 0;
    p.side = s;
    p.kind = k;
    return p;
}

static void init_board(ChessState* s) {
    int c;
    Kind back[8] = {K_R, K_N, K_B, K_Q, K_K, K_B, K_N, K_R};
    for (c = 0; c < 64; ++c) s->board[c] = empty_piece();
    for (c = 0; c < 8; ++c) {
        s->board[idx(1, c)] = make_piece(SIDE_BLACK, K_P);
        s->board[idx(6, c)] = make_piece(SIDE_WHITE, K_P);
        s->board[idx(0, c)] = make_piece(SIDE_BLACK, back[c]);
        s->board[idx(7, c)] = make_piece(SIDE_WHITE, back[c]);
    }
    s->turn = SIDE_WHITE;
    s->sel = -1;
}

static int is_path_clear(Piece b[64], int fr, int to) {
    int r1 = row_of(fr), c1 = col_of(fr), r2 = row_of(to), c2 = col_of(to);
    int dr = sign_i(r2 - r1), dc = sign_i(c2 - c1);
    int r = r1 + dr, c = c1 + dc;
    while (r != r2 || c != c2) {
        if (!b[idx(r, c)].empty) return 0;
        r += dr;
        c += dc;
    }
    return 1;
}

static int can_move(Piece b[64], int fr, int to, Side turn) {
    Piece p = b[fr];
    Piece d = b[to];
    int r1, c1, r2, c2, dr, dc;
    if (p.empty) return 0;
    if (p.side != turn) return 0;
    if (!d.empty && d.side == turn) return 0;

    r1 = row_of(fr); c1 = col_of(fr); r2 = row_of(to); c2 = col_of(to);
    dr = r2 - r1; dc = c2 - c1;
    switch (p.kind) {
        case K_P: {
            int dir = (turn == SIDE_WHITE) ? -1 : 1;
            int start = (turn == SIDE_WHITE) ? 6 : 1;
            if (dc == 0 && dr == dir && d.empty) return 1;
            if (dc == 0 && dr == 2 * dir && r1 == start && d.empty && b[idx(r1 + dir, c1)].empty) return 1;
            if (abs_i(dc) == 1 && dr == dir && !d.empty) return 1;
            return 0;
        }
        case K_N: return (abs_i(dr) == 2 && abs_i(dc) == 1) || (abs_i(dr) == 1 && abs_i(dc) == 2);
        case K_R: if (dr != 0 && dc != 0) return 0; return is_path_clear(b, fr, to);
        case K_B: if (abs_i(dr) != abs_i(dc) || dr == 0) return 0; return is_path_clear(b, fr, to);
        case K_Q: if (!(dr == 0 || dc == 0 || abs_i(dr) == abs_i(dc))) return 0; return is_path_clear(b, fr, to);
        case K_K: return abs_i(dr) <= 1 && abs_i(dc) <= 1 && (dr != 0 || dc != 0);
        default: return 0;
    }
}

static int screen_to_square(float mx, float my, ChessState* s) {
    int col, row, br;
    if (mx < 0 || my < 0 || mx >= s->field || my >= s->field) return -1;
    col = (int)(mx / s->square);
    row = (int)(my / s->square);
    if (row < 0 || row > 7 || col < 0 || col > 7) return -1;
    br = 7 - row;
    return idx(br, col);
}

static const char* label(Kind k) {
    switch (k) {
        case K_P: return "P";
        case K_R: return "R";
        case K_N: return "N";
        case K_B: return "B";
        case K_Q: return "Q";
        case K_K: return "K";
        default: return "?";
    }
}

static void chess_frame(VibFrame* frame, void* user_data) {
    ChessState* s = (ChessState*)user_data;
    int r, c;

    if (frame->input.pressed[VIB_KEY_ESCAPE]) vib_frame_request_quit(frame);

    if (frame->input.mouse.left_pressed) {
        int sq = screen_to_square((float)frame->input.mouse.x, (float)frame->input.mouse.y, s);
        if (sq >= 0) {
            Piece target = s->board[sq];
            if (!target.empty && target.side == s->turn) {
                s->sel = sq;
            } else if (s->sel >= 0 && can_move(s->board, s->sel, sq, s->turn)) {
                Piece moving = s->board[s->sel];
                s->board[s->sel] = empty_piece();
                s->board[sq] = moving;
                if (moving.kind == K_P) {
                    int pr = row_of(sq);
                    if ((moving.side == SIDE_WHITE && pr == 0) || (moving.side == SIDE_BLACK && pr == 7)) {
                        s->board[sq].kind = K_Q;
                    }
                }
                s->turn = (s->turn == SIDE_WHITE) ? SIDE_BLACK : SIDE_WHITE;
                s->sel = -1;
            }
        }
    }

    vib_canvas_clear(frame->renderer, 28, 32, 40, 255);
    for (r = 0; r < 8; ++r) {
        for (c = 0; c < 8; ++c) {
            int i = idx(r, c);
            int x = (int)(c * s->square);
            int y = (int)((7 - r) * s->square);
            int light = ((r + c) % 2 == 0);
            Piece p = s->board[i];
            vib_canvas_fill_rect(frame->renderer, x, y, (int)s->square, (int)s->square,
                light ? 240 : 181, light ? 217 : 136, light ? 181 : 99, 255);
            if (i == s->sel) {
                vib_canvas_fill_rect(frame->renderer, x, y, (int)s->square, (int)s->square, 120, 200, 255, 80);
            }
            if (!p.empty) {
                char piece_text[8];
                snprintf(piece_text, sizeof(piece_text), "%s%s", (p.side == SIDE_WHITE ? "W" : "B"), label(p.kind));
                vib_canvas_text(frame->renderer, x + 8, y + 10, piece_text, 2, 30, 30, 30, 255);
            }
        }
    }

    vib_canvas_text(frame->renderer, 12, (int)s->field + 8, "CLICK PIECE THEN TARGET  ESC QUIT", 1, 200, 210, 220, 255);
    vib_canvas_text(frame->renderer, 12, (int)s->field + 24, s->turn == SIDE_WHITE ? "TURN WHITE" : "TURN BLACK", 1, 200, 210, 220, 255);
    vib_canvas_present(frame->renderer);
}

int main(void) {
    VibWindow* win;
    ChessState s;
    if (!vib_sdl_init()) return 1;

    s.field = 560.0f;
    s.square = s.field / 8.0f;
    init_board(&s);
    win = vib_window_create("VIB CHESS", 580, 620);
    if (!win) {
        vib_sdl_shutdown();
        return 2;
    }
    vib_game_run(win, chess_frame, &s);
    vib_window_destroy(win);
    vib_sdl_shutdown();
    return 0;
}

