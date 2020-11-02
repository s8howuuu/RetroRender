#include "common.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "render.h"

static uint8_t* load_map(char const* filename, int* width) {
    FILE* f;
    *width = 0;

    if ((f = fopen(filename, "rt")) != NULL) {
        int h;
        char kind;

        if (fscanf(f, "P%c %d %d 255", &kind, width, &h) == 3) {
            uint8_t dummy;
            if (h != *width) {
                fprintf(stderr,
                        "%s: map file needs to be quadratic, not %dx%d\n",
                        filename, *width, h);
                return NULL;
            }

            size_t size = h * h;
            switch (kind) {
                case '5':
                    break;
                case '6':
                    size *= 3;
                    break;
                default:
                    fprintf(stderr,
                            "%s, illegal pnm file format (5 or 6 required)\n",
                            filename);
                    return NULL;
            }
            // read one whitespace
            if (fread(&dummy, 1, 1, f) != 1) {
                fprintf(stderr, "%s: invalid image file\n", filename);
                return NULL;
            }
            uint8_t* res = malloc(size);
            if (fread(res, 1, size, f) != size) {
                fprintf(stderr, "%s: invalid image file\n", filename);
                return NULL;
            }
            return res;
        } else {
            fprintf(stderr,
                    "%s: image file has illegal format (pnm required)\n",
                    filename);
            return NULL;
        }
    } else {
        fprintf(stderr, "%s: could not open map file\n", filename);
        return NULL;
    }
}

bool store_img(char const* filename, uint32_t* img, int width, int height) {
    FILE* f;
    if ((f = fopen(filename, "wt")) != NULL) {
        fprintf(f, "P6\n%d %d\n255\n", width, height);
        for (int i = 0; i < width * height; ++i) {
            uint32_t val = img[i];
            fputc((val >> 24) & 0xff, f);
            fputc((val >> 16) & 0xff, f);
            fputc((val >> 8) & 0xff, f);
        }
        fclose(f);
    } else {
        fprintf(stderr, "%s: could not open image file\n", filename);
        return false;
    }
    return true;
}

bool initialize_ctx(ctx_t* c, int scr_width, int scr_height, int distance,
                    const char* map_prefix) {
    int color_map_size = 0;
    uint8_t* color_map = NULL;

    memset(c, 0, sizeof(*c));
    c->scr_width = scr_width;
    c->scr_height = scr_height;
    c->sky_color = 0x66a3ff00;
    c->distance = distance;

    {
        char buf[1024];
        snprintf(buf, sizeof(buf), "%s.ppm", map_prefix);
        color_map = load_map(buf, &color_map_size);
        snprintf(buf, sizeof(buf), "%s.pgm", map_prefix);
        c->height_map = load_map(buf, &c->map_size);
    }

    if (!color_map) {
        fprintf(stderr, "color map could not be loaded\n");
        goto err;
    }

    if (!c->height_map) {
        fprintf(stderr, "height map could not be loaded\n");
        goto err_free_color;
    }

    if (color_map_size != c->map_size) {
        fprintf(stderr,
                "height map size (%d) must be equal to color map size (%d)\n",
                c->map_size, color_map_size);
        goto err_free_both;
    }

    // convert color map from RGB to RGBA
    {
        int n_pixels = color_map_size * color_map_size;
        c->color_map = malloc(n_pixels * sizeof(c->color_map[0]));
        uint32_t* q = c->color_map;
        uint8_t* p = color_map;

        for (int cnt = n_pixels; cnt != 0; cnt--) {
            *q++ = ((uint32_t)p[0] << 24) | ((uint32_t)p[1] << 16) |
                   ((uint32_t)p[2] << 8);
            p += 3;
        }
        free(color_map);
    }

    c->out = malloc(c->scr_width * c->scr_height * sizeof(c->out[0]));

    return true;

err_free_both:
    free(c->height_map);
err_free_color:
    free(color_map);
err:
    return false;
}

void destroy_ctx(ctx_t* c) {
    free(c->color_map);
    free(c->height_map);
    free(c->out);
}
