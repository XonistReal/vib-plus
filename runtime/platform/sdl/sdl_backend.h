#ifndef VIB_RUNTIME_SDL_BACKEND_H
#define VIB_RUNTIME_SDL_BACKEND_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct VibWindow VibWindow;
typedef struct VibRenderer VibRenderer;

typedef enum VibKey {
    VIB_KEY_UNKNOWN = 0,
    VIB_KEY_W,
    VIB_KEY_A,
    VIB_KEY_S,
    VIB_KEY_D,
    VIB_KEY_UP,
    VIB_KEY_DOWN,
    VIB_KEY_LEFT,
    VIB_KEY_RIGHT,
    VIB_KEY_SPACE,
    VIB_KEY_ENTER,
    VIB_KEY_ESCAPE,
    VIB_KEY_COUNT
} VibKey;

typedef struct VibMouseState {
    int32_t x;
    int32_t y;
    uint8_t left_held;
    uint8_t left_pressed;
    uint8_t left_released;
    uint8_t right_held;
    uint8_t right_pressed;
    uint8_t right_released;
} VibMouseState;

typedef struct VibInputState {
    uint8_t held[VIB_KEY_COUNT];
    uint8_t pressed[VIB_KEY_COUNT];
    uint8_t released[VIB_KEY_COUNT];
    VibMouseState mouse;
} VibInputState;

typedef struct VibFrame {
    float dt_seconds;
    int32_t width;
    int32_t height;
    VibInputState input;
    VibRenderer* renderer;
    uint8_t request_quit;
} VibFrame;

typedef void (*VibFrameCallback)(VibFrame* frame, void* user_data);

int vib_sdl_init(void);
void vib_sdl_shutdown(void);

VibWindow* vib_window_create(const char* title, int32_t width, int32_t height);
void vib_window_destroy(VibWindow* window);

void vib_game_run(VibWindow* window, VibFrameCallback callback, void* user_data);
void vib_frame_request_quit(VibFrame* frame);

/* Drawing API scaffold */
void vib_canvas_clear(VibRenderer* renderer, uint8_t r, uint8_t g, uint8_t b, uint8_t a);
void vib_canvas_fill_rect(VibRenderer* renderer, int32_t x, int32_t y, int32_t w, int32_t h, uint8_t r, uint8_t g, uint8_t b, uint8_t a);
void vib_canvas_stroke_rect(VibRenderer* renderer, int32_t x, int32_t y, int32_t w, int32_t h, uint8_t r, uint8_t g, uint8_t b, uint8_t a);
void vib_canvas_line(VibRenderer* renderer, int32_t x1, int32_t y1, int32_t x2, int32_t y2, uint8_t r, uint8_t g, uint8_t b, uint8_t a);
void vib_canvas_fill_circle(VibRenderer* renderer, int32_t cx, int32_t cy, int32_t radius, uint8_t r, uint8_t g, uint8_t b, uint8_t a);
void vib_canvas_text(VibRenderer* renderer, int32_t x, int32_t y, const char* text, int32_t scale, uint8_t r, uint8_t g, uint8_t b, uint8_t a);
void vib_canvas_present(VibRenderer* renderer);

#ifdef __cplusplus
}
#endif

#endif /* VIB_RUNTIME_SDL_BACKEND_H */

