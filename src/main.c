#include <SDL.h>
#include <limits.h>
#include <stdio.h>
#include <unistd.h>

#include "common.h"
#include "render.h"

// Disable leak sanitizer since SDL2 inherently causes memory leaks.
int __lsan_is_turned_off() { return 1; }

int main(int argc, char* argv[]) {
    // default values
    int scr_width = 640;
    int scr_height = 480;
    int distance = 400;
    int update_fps = INT_MAX;
    int target_fps = 60;

    // parse command line arguments
    int ch;
    while ((ch = getopt(argc, argv, "l:f:d:w:h:?")) != -1) {
        switch (ch) {
            case 'l':
                target_fps = atoi(optarg);
                break;
            case 'f':
                update_fps = atoi(optarg);
                break;
            case 'w':
                scr_width = atoi(optarg);
                break;
            case 'h':
                scr_height = atoi(optarg);
                break;
            case 'd':
                distance = atoi(optarg);
                break;
            case '?':
            default:
                printf(
                    "%s [-l fps limit] [-f frames until fps display] [-w "
                    "window width] [-h window height] [-d view distance] "
                    "<prefix of map files>\n",
                    argv[0]);
                return 0;
        }
    }

    argc -= optind;
    argv += optind;

    if (argc < 1) {
        fprintf(stderr, "no map specified\n");
        return 1;
    }

    // Set up context, load image files
    ctx_t c;

    if (!initialize_ctx(&c, scr_width, scr_height, distance, argv[0])) {
        return 1;
    }

    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        fprintf(stderr, "could not initialize SDL\n");
        return 1;
    }

    SDL_Window* win =
        SDL_CreateWindow("RetroRenderer", SDL_WINDOWPOS_CENTERED,
                         SDL_WINDOWPOS_CENTERED, c.scr_width, c.scr_height, 0);

    SDL_Renderer* renderer = SDL_CreateRenderer(
        win, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_TARGETTEXTURE);

    SDL_Texture* text = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888,
                                          SDL_TEXTUREACCESS_STREAMING,
                                          c.scr_width, c.scr_height);

    // initialize player
    player_t p;
    memset(&p, 0, sizeof(p));

    SDL_SetRenderTarget(renderer, NULL);

    // for limiting fps
    uint32_t last = SDL_GetTicks();
    uint32_t now = 0;

    // for computing and printing fps
    uint32_t last_frame = SDL_GetTicks();
    uint32_t now_frame = 0;
    int frames = 0;

    int target_ms_per_frame = 1000 / target_fps;

    for (;;) {
        SDL_Event event;

        while (SDL_PollEvent(&event)) {
            switch (event.type) {
                case SDL_QUIT:
                    goto done;

                case SDL_KEYDOWN:
                    switch (event.key.keysym.sym) {
                        case 'a':
                            p.v_angular = 5;
                            break;
                        case 'd':
                            p.v_angular = -5;
                            break;
                        case 'w':
                            p.v = 4;
                            break;
                        case 's':
                            p.v = -4;
                            break;
                        case 'e':
                            p.v_height = 4;
                            break;
                        case 'q':
                            p.v_height = -4;
                            break;
                        default:;
                    }
                    break;

                case SDL_KEYUP:
                    switch (event.key.keysym.sym) {
                        case 'a':
                        case 'd':
                            p.v_angular = 0;
                            break;
                        case 'w':
                        case 's':
                            p.v = 0;
                            break;
                        case 'e':
                        case 'q':
                            p.v_height = 0;
                            break;
                        default:;
                    }
                    break;
            }
        }

        now_frame = SDL_GetTicks();
        int d = now_frame - last_frame;
        if (d < target_ms_per_frame) {
            SDL_Delay(target_ms_per_frame - d);
        }
        last_frame = SDL_GetTicks();

        if (frames >= update_fps) {
            now = SDL_GetTicks();
            uint32_t dmsecs = (now - last);
            if (dmsecs > 0) {
                printf("fps: %d\n", (frames * 1000) / dmsecs);
                frames = 0;
                last = now;
            }
        }

        update_player(&p, &c);
        render(&p, &c);

        SDL_UpdateTexture(text, NULL, c.out, c.scr_width * sizeof(c.out[0]));
        SDL_RenderCopy(renderer, text, NULL, NULL);
        SDL_RenderPresent(renderer);

        frames++;
    }

done:
    destroy_ctx(&c);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(win);
    SDL_Quit();
    return 0;
}
