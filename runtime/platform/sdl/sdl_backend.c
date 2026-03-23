#include "sdl_backend.h"

#include <SDL.h>
#include <ctype.h>
#include <math.h>
#include <string.h>

struct VibWindow {
    SDL_Window* window;
    SDL_Renderer* renderer;
    int32_t width;
    int32_t height;
    uint8_t running;
};

struct VibRenderer {
    SDL_Renderer* raw;
};

static VibKey vib_map_key(SDL_Keycode k) {
    switch (k) {
        case SDLK_w: return VIB_KEY_W;
        case SDLK_a: return VIB_KEY_A;
        case SDLK_s: return VIB_KEY_S;
        case SDLK_d: return VIB_KEY_D;
        case SDLK_UP: return VIB_KEY_UP;
        case SDLK_DOWN: return VIB_KEY_DOWN;
        case SDLK_LEFT: return VIB_KEY_LEFT;
        case SDLK_RIGHT: return VIB_KEY_RIGHT;
        case SDLK_SPACE: return VIB_KEY_SPACE;
        case SDLK_RETURN: return VIB_KEY_ENTER;
        case SDLK_ESCAPE: return VIB_KEY_ESCAPE;
        default: return VIB_KEY_UNKNOWN;
    }
}

int vib_sdl_init(void) {
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS | SDL_INIT_TIMER) != 0) {
        return 0;
    }
    return 1;
}

void vib_sdl_shutdown(void) {
    SDL_Quit();
}

VibWindow* vib_window_create(const char* title, int32_t width, int32_t height) {
    VibWindow* out;
    SDL_Window* win;
    SDL_Renderer* ren;

    win = SDL_CreateWindow(title, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width, height, SDL_WINDOW_SHOWN);
    if (!win) return NULL;

    ren = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!ren) {
        SDL_DestroyWindow(win);
        return NULL;
    }

    out = (VibWindow*)SDL_malloc(sizeof(VibWindow));
    if (!out) {
        SDL_DestroyRenderer(ren);
        SDL_DestroyWindow(win);
        return NULL;
    }

    out->window = win;
    out->renderer = ren;
    out->width = width;
    out->height = height;
    out->running = 1;
    return out;
}

void vib_window_destroy(VibWindow* window) {
    if (!window) return;
    if (window->renderer) SDL_DestroyRenderer(window->renderer);
    if (window->window) SDL_DestroyWindow(window->window);
    SDL_free(window);
}

static void vib_input_begin_frame(VibInputState* input) {
    memset(input->pressed, 0, sizeof(input->pressed));
    memset(input->released, 0, sizeof(input->released));
    input->mouse.left_pressed = 0;
    input->mouse.left_released = 0;
    input->mouse.right_pressed = 0;
    input->mouse.right_released = 0;
}

static void vib_handle_event(VibWindow* window, VibInputState* input, const SDL_Event* ev) {
    VibKey k;
    switch (ev->type) {
        case SDL_QUIT:
            window->running = 0;
            break;
        case SDL_KEYDOWN:
            if (ev->key.repeat) break;
            k = vib_map_key(ev->key.keysym.sym);
            if (k != VIB_KEY_UNKNOWN) {
                input->held[k] = 1;
                input->pressed[k] = 1;
            }
            break;
        case SDL_KEYUP:
            k = vib_map_key(ev->key.keysym.sym);
            if (k != VIB_KEY_UNKNOWN) {
                input->held[k] = 0;
                input->released[k] = 1;
            }
            break;
        case SDL_MOUSEBUTTONDOWN:
            if (ev->button.button == SDL_BUTTON_LEFT) {
                input->mouse.left_held = 1;
                input->mouse.left_pressed = 1;
            } else if (ev->button.button == SDL_BUTTON_RIGHT) {
                input->mouse.right_held = 1;
                input->mouse.right_pressed = 1;
            }
            break;
        case SDL_MOUSEBUTTONUP:
            if (ev->button.button == SDL_BUTTON_LEFT) {
                input->mouse.left_held = 0;
                input->mouse.left_released = 1;
            } else if (ev->button.button == SDL_BUTTON_RIGHT) {
                input->mouse.right_held = 0;
                input->mouse.right_released = 1;
            }
            break;
        default:
            break;
    }
}

void vib_game_run(VibWindow* window, VibFrameCallback callback, void* user_data) {
    uint64_t last_counter;
    VibInputState input;
    VibRenderer renderer;

    if (!window || !callback) return;

    memset(&input, 0, sizeof(input));
    renderer.raw = window->renderer;
    last_counter = SDL_GetPerformanceCounter();

    while (window->running) {
        SDL_Event ev;
        uint64_t now;
        uint64_t freq;
        float dt;
        VibFrame frame;

        vib_input_begin_frame(&input);
        while (SDL_PollEvent(&ev)) {
            vib_handle_event(window, &input, &ev);
        }

        SDL_GetMouseState(&input.mouse.x, &input.mouse.y);
        SDL_GetWindowSize(window->window, &window->width, &window->height);

        now = SDL_GetPerformanceCounter();
        freq = SDL_GetPerformanceFrequency();
        dt = (float)(now - last_counter) / (float)freq;
        last_counter = now;

        frame.dt_seconds = dt;
        frame.width = window->width;
        frame.height = window->height;
        frame.input = input;
        frame.renderer = &renderer;
        frame.request_quit = 0;

        callback(&frame, user_data);
        if (frame.request_quit) {
            window->running = 0;
        }
    }
}

void vib_frame_request_quit(VibFrame* frame) {
    if (!frame) return;
    frame->request_quit = 1;
}

void vib_canvas_clear(VibRenderer* renderer, uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
    if (!renderer || !renderer->raw) return;
    SDL_SetRenderDrawColor(renderer->raw, r, g, b, a);
    SDL_RenderClear(renderer->raw);
}

void vib_canvas_fill_rect(VibRenderer* renderer, int32_t x, int32_t y, int32_t w, int32_t h, uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
    SDL_Rect rect;
    if (!renderer || !renderer->raw || w <= 0 || h <= 0) return;
    rect.x = x;
    rect.y = y;
    rect.w = w;
    rect.h = h;
    SDL_SetRenderDrawColor(renderer->raw, r, g, b, a);
    SDL_RenderFillRect(renderer->raw, &rect);
}

void vib_canvas_stroke_rect(VibRenderer* renderer, int32_t x, int32_t y, int32_t w, int32_t h, uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
    SDL_Rect rect;
    if (!renderer || !renderer->raw || w <= 0 || h <= 0) return;
    rect.x = x;
    rect.y = y;
    rect.w = w;
    rect.h = h;
    SDL_SetRenderDrawColor(renderer->raw, r, g, b, a);
    SDL_RenderDrawRect(renderer->raw, &rect);
}

void vib_canvas_line(VibRenderer* renderer, int32_t x1, int32_t y1, int32_t x2, int32_t y2, uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
    if (!renderer || !renderer->raw) return;
    SDL_SetRenderDrawColor(renderer->raw, r, g, b, a);
    SDL_RenderDrawLine(renderer->raw, x1, y1, x2, y2);
}

void vib_canvas_fill_circle(VibRenderer* renderer, int32_t cx, int32_t cy, int32_t radius, uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
    int32_t y;
    if (!renderer || !renderer->raw || radius <= 0) return;
    SDL_SetRenderDrawColor(renderer->raw, r, g, b, a);
    for (y = -radius; y <= radius; ++y) {
        float dx = sqrtf((float)(radius * radius - y * y));
        int32_t x0 = cx - (int32_t)dx;
        int32_t x1 = cx + (int32_t)dx;
        SDL_RenderDrawLine(renderer->raw, x0, cy + y, x1, cy + y);
    }
}

static uint8_t vib_font5x7(char c, int row) {
    switch (c) {
        case 'A': { static const uint8_t p[7] = {0x0E,0x11,0x11,0x1F,0x11,0x11,0x11}; return p[row]; }
        case 'B': { static const uint8_t p[7] = {0x1E,0x11,0x11,0x1E,0x11,0x11,0x1E}; return p[row]; }
        case 'C': { static const uint8_t p[7] = {0x0E,0x11,0x10,0x10,0x10,0x11,0x0E}; return p[row]; }
        case 'D': { static const uint8_t p[7] = {0x1E,0x11,0x11,0x11,0x11,0x11,0x1E}; return p[row]; }
        case 'E': { static const uint8_t p[7] = {0x1F,0x10,0x10,0x1E,0x10,0x10,0x1F}; return p[row]; }
        case 'F': { static const uint8_t p[7] = {0x1F,0x10,0x10,0x1E,0x10,0x10,0x10}; return p[row]; }
        case 'G': { static const uint8_t p[7] = {0x0E,0x11,0x10,0x10,0x13,0x11,0x0E}; return p[row]; }
        case 'H': { static const uint8_t p[7] = {0x11,0x11,0x11,0x1F,0x11,0x11,0x11}; return p[row]; }
        case 'I': { static const uint8_t p[7] = {0x1F,0x04,0x04,0x04,0x04,0x04,0x1F}; return p[row]; }
        case 'J': { static const uint8_t p[7] = {0x01,0x01,0x01,0x01,0x11,0x11,0x0E}; return p[row]; }
        case 'K': { static const uint8_t p[7] = {0x11,0x12,0x14,0x18,0x14,0x12,0x11}; return p[row]; }
        case 'L': { static const uint8_t p[7] = {0x10,0x10,0x10,0x10,0x10,0x10,0x1F}; return p[row]; }
        case 'M': { static const uint8_t p[7] = {0x11,0x1B,0x15,0x15,0x11,0x11,0x11}; return p[row]; }
        case 'N': { static const uint8_t p[7] = {0x11,0x19,0x15,0x13,0x11,0x11,0x11}; return p[row]; }
        case 'O': { static const uint8_t p[7] = {0x0E,0x11,0x11,0x11,0x11,0x11,0x0E}; return p[row]; }
        case 'P': { static const uint8_t p[7] = {0x1E,0x11,0x11,0x1E,0x10,0x10,0x10}; return p[row]; }
        case 'Q': { static const uint8_t p[7] = {0x0E,0x11,0x11,0x11,0x15,0x12,0x0D}; return p[row]; }
        case 'R': { static const uint8_t p[7] = {0x1E,0x11,0x11,0x1E,0x14,0x12,0x11}; return p[row]; }
        case 'S': { static const uint8_t p[7] = {0x0F,0x10,0x10,0x0E,0x01,0x01,0x1E}; return p[row]; }
        case 'T': { static const uint8_t p[7] = {0x1F,0x04,0x04,0x04,0x04,0x04,0x04}; return p[row]; }
        case 'U': { static const uint8_t p[7] = {0x11,0x11,0x11,0x11,0x11,0x11,0x0E}; return p[row]; }
        case 'V': { static const uint8_t p[7] = {0x11,0x11,0x11,0x11,0x11,0x0A,0x04}; return p[row]; }
        case 'W': { static const uint8_t p[7] = {0x11,0x11,0x11,0x15,0x15,0x15,0x0A}; return p[row]; }
        case 'X': { static const uint8_t p[7] = {0x11,0x11,0x0A,0x04,0x0A,0x11,0x11}; return p[row]; }
        case 'Y': { static const uint8_t p[7] = {0x11,0x11,0x0A,0x04,0x04,0x04,0x04}; return p[row]; }
        case 'Z': { static const uint8_t p[7] = {0x1F,0x01,0x02,0x04,0x08,0x10,0x1F}; return p[row]; }
        case '0': { static const uint8_t p[7] = {0x0E,0x11,0x13,0x15,0x19,0x11,0x0E}; return p[row]; }
        case '1': { static const uint8_t p[7] = {0x04,0x0C,0x04,0x04,0x04,0x04,0x0E}; return p[row]; }
        case '2': { static const uint8_t p[7] = {0x0E,0x11,0x01,0x02,0x04,0x08,0x1F}; return p[row]; }
        case '3': { static const uint8_t p[7] = {0x1E,0x01,0x01,0x06,0x01,0x01,0x1E}; return p[row]; }
        case '4': { static const uint8_t p[7] = {0x02,0x06,0x0A,0x12,0x1F,0x02,0x02}; return p[row]; }
        case '5': { static const uint8_t p[7] = {0x1F,0x10,0x10,0x1E,0x01,0x01,0x1E}; return p[row]; }
        case '6': { static const uint8_t p[7] = {0x0E,0x10,0x10,0x1E,0x11,0x11,0x0E}; return p[row]; }
        case '7': { static const uint8_t p[7] = {0x1F,0x01,0x02,0x04,0x08,0x08,0x08}; return p[row]; }
        case '8': { static const uint8_t p[7] = {0x0E,0x11,0x11,0x0E,0x11,0x11,0x0E}; return p[row]; }
        case '9': { static const uint8_t p[7] = {0x0E,0x11,0x11,0x0F,0x01,0x01,0x0E}; return p[row]; }
        case '-': { static const uint8_t p[7] = {0x00,0x00,0x00,0x1F,0x00,0x00,0x00}; return p[row]; }
        case ':': { static const uint8_t p[7] = {0x00,0x04,0x00,0x00,0x04,0x00,0x00}; return p[row]; }
        case '.': { static const uint8_t p[7] = {0x00,0x00,0x00,0x00,0x00,0x06,0x06}; return p[row]; }
        default: { static const uint8_t p[7] = {0x00,0x1F,0x11,0x11,0x11,0x1F,0x00}; return p[row]; }
    }
}

static void vib_draw_char(VibRenderer* renderer, int32_t x, int32_t y, char c, int32_t scale) {
    int row;
    int col;
    if (scale <= 0) scale = 1;
    for (row = 0; row < 7; ++row) {
        uint8_t bits = vib_font5x7(c, row);
        for (col = 0; col < 5; ++col) {
            if (bits & (1u << (4 - col))) {
                SDL_Rect px;
                px.x = x + col * scale;
                px.y = y + row * scale;
                px.w = scale;
                px.h = scale;
                SDL_RenderFillRect(renderer->raw, &px);
            }
        }
    }
}

void vib_canvas_text(VibRenderer* renderer, int32_t x, int32_t y, const char* text, int32_t scale, uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
    int32_t pen_x = x;
    const char* p = text;
    if (!renderer || !renderer->raw || !text) return;
    if (scale <= 0) scale = 1;
    SDL_SetRenderDrawColor(renderer->raw, r, g, b, a);
    while (*p) {
        char c = *p;
        if (c == ' ') {
            pen_x += 4 * scale;
        } else {
            vib_draw_char(renderer, pen_x, y, (char)toupper((unsigned char)c), scale);
            pen_x += 6 * scale;
        }
        ++p;
    }
}

void vib_canvas_present(VibRenderer* renderer) {
    if (!renderer || !renderer->raw) return;
    SDL_RenderPresent(renderer->raw);
}

