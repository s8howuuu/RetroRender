#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "common.h"
#include "render.h"
#include "test_common.h"

void make_flat_ctx(ctx_t* c, int width, int height, int map_size,
                   int distance) {
    memset(c, 0, sizeof(*c));
    c->scr_width = width;
    c->scr_height = height;
    c->sky_color = 0x66a3ff00;
    c->distance = distance;
    c->map_size = map_size;

    c->height_map = malloc(map_size * map_size);
    memset(c->height_map, 10, map_size * map_size);

    c->color_map = malloc(map_size * map_size * sizeof(*c->color_map));
    for (int i = 0; i < map_size; ++i) {
        for (int j = 0; j < map_size; ++j) {
            if ((i % 16 < 8) ^ (j % 16 < 8)) {
                c->color_map[i * map_size + j] = 0x66ff6600;
            } else {
                c->color_map[i * map_size + j] = 0xff66ff00;
            }
        }
    }

    c->out = malloc(width * height * sizeof(*c->out));
}

/** Create an image to manually inspect the results of draw_line.
 */
bool create_test_image(const char* outfile) {
    int width = 321;
    int u_middle = (width / 2) + 1;
    int height = 241;
    int v_middle = (height / 2) + 1;

    ctx_t c;
    make_flat_ctx(&c, width, height, 256, 200);

    uint32_t red = 0xff000000;
    uint32_t green = 0x00ff0000;
    uint32_t blue = 0x0000ff00;

    uint32_t yellow = 0xffff0000;
    uint32_t magenta = 0xff00ff00;
    uint32_t cyan = 0x00ffff00;

    uint32_t gray = 0x55555500;
    uint32_t black = 0x00000000;
    uint32_t white = 0xffffff00;

    for (int u = 0; u < width; u++) {
        for (int v = 0; v < height; v++) {
            c.out[v * width + u] = gray;
        }
    }

    draw_line(&c, 1, 0, height, red);
    draw_line(&c, width, 0, height, red);
    for (int u = 1; u <= width; u++) {
        draw_line(&c, u, 0, 1, blue);
    }
    for (int u = 1; u <= width; u++) {
        draw_line(&c, u, height - 1, height, blue);
    }

    draw_line(&c, u_middle, 0, height, black);
    for (int u = 1; u <= width; u++) {
        draw_line(&c, u, v_middle - 1, v_middle, white);
    }

    int step = u_middle / 5;
    draw_line(&c, step * 1, v_middle, height - 90, red);
    draw_line(&c, step * 2, v_middle, height - 60, green);
    draw_line(&c, step * 3, v_middle, height - 30, blue);

    draw_line(&c, u_middle + step * 1, 30, v_middle, cyan);
    draw_line(&c, u_middle + step * 2, 60, v_middle, magenta);
    draw_line(&c, u_middle + step * 3, 90, v_middle, yellow);

    bool result = store_img(outfile, c.out, c.scr_width, c.scr_height);

    destroy_ctx(&c);

    return result;
}

result_t test_image_wrapper(const char* test) {
    // for generating a test image
    // 'test' is expected to have the form
    //   "testimage:<outfile>"
    char* buf = strdup(test);
    const char* delim = ":";

    char* ptr = NULL;

    ptr = strtok(buf, delim);
    assert(ptr != NULL);
    // ptr contains "testimage"

    const char* filename = NULL;
    ptr = strtok(NULL, delim);
    if (ptr == NULL) {
        filename = "test_image.ppm";
    } else {
        filename = ptr;
    }

    bool res = create_test_image(filename);

    free(buf);

    return res ? SUCCESS : FAILURE;
}

result_t render_frame(const char* test) {
    // for rendering tests
    // 'test' is expected to have the form
    //   "frame:<width>:<height>:<distance>:<map_prefix>:<xpos>:<ypos>:<zpos>:<angle>:<outfile>"
    char* buf = strdup(test);
    const char* delim = ":";

    char* ptr = NULL;

    ptr = strtok(buf, delim);
    assert(ptr != NULL);
    // ptr contains "frame"

#define extract(var, type, fun) \
    ptr = strtok(NULL, delim);  \
    assert(ptr != NULL);        \
    type var = fun(ptr);

    extract(width, int, atoi);
    extract(height, int, atoi);
    extract(distance, int, atoi);
    extract(map_prefix, const char*, (const char*));
    extract(xpos, float, atof);
    extract(ypos, float, atof);
    extract(zpos, int, atoi);
    extract(angle, int, atoi);
    extract(outfile, const char*, (const char*));

#undef extract

    ctx_t c;

    if (!initialize_ctx(&c, width, height, distance, map_prefix)) {
        return 1;
    }

    player_t p;
    memset(&p, 0, sizeof(p));

    p.x = xpos;
    p.y = ypos;
    p.height = zpos;
    p.angle = angle;

    render(&p, &c);

    bool result = store_img(outfile, c.out, c.scr_width, c.scr_height);

    destroy_ctx(&c);
    free(buf);

    return result ? SUCCESS : FAILURE;
}

bool player_equal(player_t* p, player_t* p_ref) {
    float epsilon = 0.001;
    if (fabsf(p->x - p_ref->x) > epsilon) {
        fprintf(stderr, "unexpected x position\n");
        fprintf(stderr, "  res: %f , ref: %f\n", p->x, p_ref->x);
        return false;
    }
    if (fabsf(p->y - p_ref->y) > epsilon) {
        fprintf(stderr, "unexpected y position\n");
        fprintf(stderr, "  res: %f , ref: %f\n", p->y, p_ref->y);
        return false;
    }
    if (p->angle != p_ref->angle) {
        fprintf(stderr, "unexpected angle\n");
        fprintf(stderr, "  res: %d , ref: %d\n", p->angle, p_ref->angle);
        return false;
    }
    if (p->height != p_ref->height) {
        fprintf(stderr, "unexpected height\n");
        fprintf(stderr, "  res: %d , ref: %d\n", p->height, p_ref->height);
        return false;
    }
    if (p->v != p_ref->v) {
        fprintf(stderr, "unexpected horizontal velocity\n");
        fprintf(stderr, "  res: %d , ref: %d\n", p->v, p_ref->v);
        return false;
    }
    if (p->v_height != p_ref->v_height) {
        fprintf(stderr, "unexpected vertical velocity\n");
        fprintf(stderr, "  res: %d , ref: %d\n", p->v_height, p_ref->v_height);
        return false;
    }
    if (p->v_angular != p_ref->v_angular) {
        fprintf(stderr, "unexpected angular velocity\n");
        fprintf(stderr, "  res: %d , ref: %d\n", p->v_angular,
                p_ref->v_angular);
        return false;
    }
    return true;
}

void default_player(player_t* p) {
    p->x = 100.0;
    p->y = 100.0;
    p->angle = 90;
    p->height = 100;
    p->v = 0;
    p->v_height = 0;
    p->v_angular = 0;
}

result_t player_test(player_t* p, player_t* p_ref) {
    ctx_t c;
    make_flat_ctx(&c, 320, 240, 256, 200);
    update_player(p, &c);
    destroy_ctx(&c);
    return (player_equal(p, p_ref)) ? SUCCESS : FAILURE;
}

result_t player_move_01_test(const char* test) {
    (void)test;
    player_t p;
    default_player(&p);
    p.v = 5;

    player_t p_ref = p;
    p_ref.y = 95.0;

    return player_test(&p, &p_ref);
}

result_t player_move_02_test(const char* test) {
    (void)test;
    player_t p;
    default_player(&p);
    p.v_height = 5;

    player_t p_ref = p;
    p_ref.height = 105.0;

    return player_test(&p, &p_ref);
}

result_t player_move_03_test(const char* test) {
    (void)test;
    player_t p;
    default_player(&p);
    p.v_angular = 5;
    player_t p_ref = p;
    p_ref.angle = 95;

    return player_test(&p, &p_ref);
}

test_fun_t get_test(const char* test) {
    TEST("public.player.move_01", player_move_01_test);
    TEST("public.player.move_02", player_move_02_test);
    TEST("public.player.move_03", player_move_03_test);

    TEST("frame", render_frame)
    TEST("testimage", test_image_wrapper)

    return NULL;
}
