#include "sdl_backend.h"

#include <math.h>
#include <stdio.h>

typedef struct Pipe {
    float x;
    float gap_y;
    float gap;
    int done;
} Pipe;

typedef struct FlappyState {
    float bird_x;
    float bird_y;
    float bird_vy;
    float gravity;
    float flap_imp;
    Pipe pipes[4];
    int score;
    int game_over;
    float field_w;
    float field_h;
    float pipe_w;
    float scroll;
    float spacing;
} FlappyState;

static float clampf(float x, float lo, float hi) {
    if (x < lo) return lo;
    if (x > hi) return hi;
    return x;
}

static float rand_gap_y(float h, float seed) {
    float g = 148.0f;
    float margin = 100.0f;
    float span = h - 2.0f * margin - g;
    float t = fmodf(fabsf(seed), 317.0f) / 317.0f;
    return margin + g * 0.5f + t * span;
}

static Pipe spawn_at(float h, float x, float seed) {
    Pipe p;
    p.x = x;
    p.gap = 148.0f;
    p.gap_y = rand_gap_y(h, seed);
    p.done = 0;
    return p;
}

static int circle_hits_rect(float cx, float cy, float r, float rx, float ry, float rw, float rh) {
    float nx = clampf(cx, rx, rx + rw);
    float ny = clampf(cy, ry, ry + rh);
    float dx = cx - nx;
    float dy = cy - ny;
    return (dx * dx + dy * dy) <= (r * r);
}

static void reset_game(FlappyState* s) {
    s->bird_x = s->field_w * 0.28f;
    s->bird_y = s->field_h * 0.45f;
    s->bird_vy = 0.0f;
    s->gravity = 1850.0f;
    s->flap_imp = -430.0f;
    s->pipe_w = 62.0f;
    s->scroll = 220.0f;
    s->spacing = 260.0f;
    s->score = 0;
    s->game_over = 0;
    s->pipes[0] = spawn_at(s->field_h, s->field_w * 0.65f, 11.0f);
    s->pipes[1] = spawn_at(s->field_h, s->field_w * 0.65f + 260.0f, 29.0f);
    s->pipes[2] = spawn_at(s->field_h, s->field_w * 0.65f + 520.0f, 47.0f);
    s->pipes[3] = spawn_at(s->field_h, s->field_w * 0.65f + 780.0f, 61.0f);
}

static void flappy_frame(VibFrame* frame, void* user_data) {
    FlappyState* s = (FlappyState*)user_data;
    float dt = frame->dt_seconds;
    float r = 14.0f;
    int i;
    char score_text[64];

    if (frame->input.pressed[VIB_KEY_ESCAPE]) vib_frame_request_quit(frame);

    if (s->game_over) {
        if (frame->input.pressed[VIB_KEY_SPACE] || frame->input.pressed[VIB_KEY_ENTER]) {
            reset_game(s);
        }
    } else {
        if (frame->input.pressed[VIB_KEY_SPACE] || frame->input.pressed[VIB_KEY_W]) {
            s->bird_vy = s->flap_imp;
        }
        s->bird_vy += s->gravity * dt;
        s->bird_y += s->bird_vy * dt;
        if (s->bird_y - r <= 0.0f || s->bird_y + r >= s->field_h) {
            s->game_over = 1;
        }

        for (i = 0; i < 4; ++i) {
            Pipe* p = &s->pipes[i];
            float top_h;
            float bot_y;
            float bot_h;
            p->x -= s->scroll * dt;
            top_h = p->gap_y - p->gap * 0.5f;
            bot_y = p->gap_y + p->gap * 0.5f;
            bot_h = s->field_h - bot_y;
            if (circle_hits_rect(s->bird_x, s->bird_y, r, p->x, 0.0f, s->pipe_w, top_h)) s->game_over = 1;
            if (circle_hits_rect(s->bird_x, s->bird_y, r, p->x, bot_y, s->pipe_w, bot_h)) s->game_over = 1;
            if (!p->done && p->x + s->pipe_w < s->bird_x) {
                p->done = 1;
                s->score++;
            }
            if (p->x + s->pipe_w < -30.0f) {
                float max_x = s->pipes[0].x;
                int j;
                for (j = 1; j < 4; ++j) if (s->pipes[j].x > max_x) max_x = s->pipes[j].x;
                *p = spawn_at(s->field_h, max_x + s->spacing, s->field_w + (float)s->score + (float)i * 17.0f);
            }
        }
    }

    vib_canvas_clear(frame->renderer, 52, 152, 219, 255);
    vib_canvas_fill_rect(frame->renderer, 0, (int)s->field_h - 36, (int)s->field_w, 36, 39, 174, 96, 255);
    for (i = 0; i < 4; ++i) {
        Pipe p = s->pipes[i];
        int top_h = (int)(p.gap_y - p.gap * 0.5f);
        int bot_y = (int)(p.gap_y + p.gap * 0.5f);
        int bot_h = (int)(s->field_h - bot_y);
        vib_canvas_fill_rect(frame->renderer, (int)p.x, 0, (int)s->pipe_w, top_h, 34, 139, 34, 255);
        vib_canvas_fill_rect(frame->renderer, (int)p.x, bot_y, (int)s->pipe_w, bot_h, 34, 139, 34, 255);
        vib_canvas_stroke_rect(frame->renderer, (int)p.x, 0, (int)s->pipe_w, top_h, 20, 90, 20, 255);
        vib_canvas_stroke_rect(frame->renderer, (int)p.x, bot_y, (int)s->pipe_w, bot_h, 20, 90, 20, 255);
    }
    vib_canvas_fill_circle(frame->renderer, (int)s->bird_x, (int)s->bird_y, 14, 241, 196, 15, 255);
    snprintf(score_text, sizeof(score_text), "SCORE %d", s->score);
    vib_canvas_text(frame->renderer, 20, 20, score_text, 2, 236, 240, 241, 255);
    if (s->game_over) {
        vib_canvas_text(frame->renderer, (int)s->field_w / 2 - 120, (int)s->field_h / 2 - 10, "GAME OVER  SPACE RESTART", 1, 236, 240, 241, 255);
    } else {
        vib_canvas_text(frame->renderer, 20, 50, "SPACE OR W FLAP  ESC QUIT", 1, 236, 240, 241, 255);
    }
    vib_canvas_present(frame->renderer);
}

int main(void) {
    VibWindow* win;
    FlappyState s;

    if (!vib_sdl_init()) return 1;
    win = vib_window_create("VIB FLAPPY", 520, 680);
    if (!win) {
        vib_sdl_shutdown();
        return 2;
    }

    s.field_w = 520.0f;
    s.field_h = 680.0f;
    reset_game(&s);
    vib_game_run(win, flappy_frame, &s);

    vib_window_destroy(win);
    vib_sdl_shutdown();
    return 0;
}

