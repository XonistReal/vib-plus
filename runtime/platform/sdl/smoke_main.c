#include "sdl_backend.h"
#include <math.h>

typedef struct SmokeState {
    float t;
} SmokeState;

static void smoke_frame(VibFrame* frame, void* user_data) {
    SmokeState* s = (SmokeState*)user_data;
    float osc = (sinf(s->t * 1.2f) + 1.0f) * 0.5f;
    int x = (int)(80.0f + osc * (frame->width - 160.0f));
    int y = frame->height / 2;
    int w = frame->width;
    int h = frame->height;

    /* Use key release to avoid accidental immediate exit on startup focus events. */
    if (frame->input.released[VIB_KEY_ESCAPE]) {
        vib_frame_request_quit(frame);
    }

    vib_canvas_clear(frame->renderer, 20, 24, 36, 255);
    vib_canvas_fill_rect(frame->renderer, 24, 24, w - 48, h - 48, 34, 42, 62, 255);
    vib_canvas_stroke_rect(frame->renderer, 24, 24, w - 48, h - 48, 100, 130, 200, 255);

    vib_canvas_line(frame->renderer, 40, h - 60, w - 40, h - 60, 90, 180, 255, 255);
    vib_canvas_fill_circle(frame->renderer, x, y, 28, 241, 196, 15, 255);
    vib_canvas_text(frame->renderer, 40, 36, "VIB SDL SMOKE - ESC TO QUIT", 2, 220, 230, 245, 255);
    vib_canvas_text(frame->renderer, 40, 68, "BACKEND CHECK: WINDOW INPUT DRAW LOOP", 1, 170, 190, 220, 255);
    vib_canvas_present(frame->renderer);

    s->t += frame->dt_seconds;
}

int main(void) {
    VibWindow* win;
    SmokeState state;
    state.t = 0.0f;

    if (!vib_sdl_init()) return 1;

    win = vib_window_create("Vib+ SDL Smoke", 900, 560);
    if (!win) {
        vib_sdl_shutdown();
        return 2;
    }

    vib_game_run(win, smoke_frame, &state);
    vib_window_destroy(win);
    vib_sdl_shutdown();
    return 0;
}

