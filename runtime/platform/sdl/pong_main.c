#include "sdl_backend.h"

#include <math.h>
#include <stdio.h>

typedef struct PongState {
    float ball_x;
    float ball_y;
    float ball_vx;
    float ball_vy;
    float paddle_l;
    float paddle_r;
    int score_l;
    int score_r;
    float paddle_h;
    float paddle_w;
    float ball_r;
    float field_w;
    float field_h;
    float paddle_spd;
} PongState;

static float clampf(float x, float lo, float hi) {
    if (x < lo) return lo;
    if (x > hi) return hi;
    return x;
}

static void reset_ball(PongState* s) {
    float sign = (s->ball_vx >= 0.0f) ? 1.0f : -1.0f;
    s->ball_x = s->field_w * 0.5f;
    s->ball_y = s->field_h * 0.5f;
    s->ball_vx = sign * 320.0f;
    s->ball_vy = (s->ball_vy >= 0.0f) ? 180.0f : -180.0f;
}

static void pong_frame(VibFrame* frame, void* user_data) {
    PongState* s = (PongState*)user_data;
    float dt = frame->dt_seconds;
    float margin = 24.0f;
    float lx = margin;
    float rx = s->field_w - margin - s->paddle_w;
    float half = s->paddle_h * 0.5f;
    char score_text[64];

    if (frame->input.pressed[VIB_KEY_ESCAPE]) vib_frame_request_quit(frame);

    if (frame->input.held[VIB_KEY_W]) s->paddle_l -= s->paddle_spd * dt;
    if (frame->input.held[VIB_KEY_S]) s->paddle_l += s->paddle_spd * dt;
    if (frame->input.held[VIB_KEY_UP]) s->paddle_r -= s->paddle_spd * dt;
    if (frame->input.held[VIB_KEY_DOWN]) s->paddle_r += s->paddle_spd * dt;

    s->paddle_l = clampf(s->paddle_l, half, s->field_h - half);
    s->paddle_r = clampf(s->paddle_r, half, s->field_h - half);

    s->ball_x += s->ball_vx * dt;
    s->ball_y += s->ball_vy * dt;

    if (s->ball_y - s->ball_r <= 0.0f) {
        s->ball_y = s->ball_r;
        s->ball_vy = fabsf(s->ball_vy);
    }
    if (s->ball_y + s->ball_r >= s->field_h) {
        s->ball_y = s->field_h - s->ball_r;
        s->ball_vy = -fabsf(s->ball_vy);
    }

    if (s->ball_x - s->ball_r <= lx + s->paddle_w &&
        s->ball_x - s->ball_r >= lx &&
        s->ball_y >= s->paddle_l - half - s->ball_r &&
        s->ball_y <= s->paddle_l + half + s->ball_r &&
        s->ball_vx < 0.0f) {
        float off = (s->ball_y - s->paddle_l) / half;
        s->ball_x = lx + s->paddle_w + s->ball_r;
        s->ball_vx = fabsf(s->ball_vx) * 1.05f;
        s->ball_vy += off * 120.0f;
    }

    if (s->ball_x + s->ball_r >= rx &&
        s->ball_x + s->ball_r <= rx + s->paddle_w + s->ball_r &&
        s->ball_y >= s->paddle_r - half - s->ball_r &&
        s->ball_y <= s->paddle_r + half + s->ball_r &&
        s->ball_vx > 0.0f) {
        float off = (s->ball_y - s->paddle_r) / half;
        s->ball_x = rx - s->ball_r;
        s->ball_vx = -fabsf(s->ball_vx) * 1.05f;
        s->ball_vy += off * 120.0f;
    }

    if (s->ball_x < -40.0f) {
        s->score_r++;
        reset_ball(s);
    }
    if (s->ball_x > s->field_w + 40.0f) {
        s->score_l++;
        reset_ball(s);
    }
    if (frame->input.pressed[VIB_KEY_SPACE]) {
        reset_ball(s);
    }

    vib_canvas_clear(frame->renderer, 12, 18, 32, 255);
    vib_canvas_fill_rect(frame->renderer, (int)lx, (int)(s->paddle_l - half), (int)s->paddle_w, (int)s->paddle_h, 90, 200, 255, 255);
    vib_canvas_fill_rect(frame->renderer, (int)rx, (int)(s->paddle_r - half), (int)s->paddle_w, (int)s->paddle_h, 90, 200, 255, 255);
    vib_canvas_fill_circle(frame->renderer, (int)s->ball_x, (int)s->ball_y, (int)s->ball_r, 220, 230, 245, 255);

    snprintf(score_text, sizeof(score_text), "%d - %d", s->score_l, s->score_r);
    vib_canvas_text(frame->renderer, 30, 24, score_text, 3, 220, 230, 245, 255);
    vib_canvas_text(frame->renderer, 30, (int)s->field_h - 36, "W S VS UP DOWN  SPACE RESET  ESC QUIT", 1, 170, 190, 220, 255);
    vib_canvas_present(frame->renderer);
}

int main(void) {
    VibWindow* win;
    PongState s;

    if (!vib_sdl_init()) return 1;
    win = vib_window_create("VIB PONG", 900, 560);
    if (!win) {
        vib_sdl_shutdown();
        return 2;
    }

    s.field_w = 900.0f;
    s.field_h = 560.0f;
    s.ball_x = s.field_w * 0.5f;
    s.ball_y = s.field_h * 0.5f;
    s.ball_vx = 320.0f;
    s.ball_vy = 180.0f;
    s.paddle_h = 100.0f;
    s.paddle_w = 14.0f;
    s.ball_r = 8.0f;
    s.paddle_l = s.field_h * 0.5f;
    s.paddle_r = s.field_h * 0.5f;
    s.paddle_spd = 420.0f;
    s.score_l = 0;
    s.score_r = 0;

    vib_game_run(win, pong_frame, &s);
    vib_window_destroy(win);
    vib_sdl_shutdown();
    return 0;
}

