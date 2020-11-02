#pragma once

#include <stdint.h>

typedef struct {
    float x, y;     // position on map
    int angle;      // angle of view direction (wrt to x-axis)
    int height;     // height of the player
    int v;          // velocity of the player in x/y
    int v_height;   // height change rate
    int v_angular;  // angular velocity when turning
} player_t;

typedef struct {
    int scr_width;   // screen width in pixels
    int scr_height;  // screen height in pixels
    int map_size;
    int distance;        // number of lines to render
    uint32_t sky_color;  // RGBA value of the sky color

    uint8_t* height_map;  // The height map. Points to a buffer containing
                          // a sequence of map_size lines, each with
                          // map_size height entries.
                          // Entry 0 corresponds to the top-left pixel of
                          // the map, entry (map_size * map_size) - 1
                          // corresponds to the bottom-right pixel of the
                          // map.

    uint32_t* color_map;  // The color map. Points to a buffer containing
                          // a sequence of map_size lines, each with
                          // map_size color entries.
                          // Entry 0 corresponds to the top-left pixel of
                          // the map, entry (map_size * map_size) - 1
                          // corresponds to the bottom-right pixel of the
                          // map.

    uint32_t* out;  // The buffer to render to.
                    // This buffer is interpreted as a sequence of
                    // scr_height lines with scr_width entries each
                    // that are written from top to bottom to the
                    // screen.
} ctx_t;

void update_player(player_t* p, ctx_t const* ctx);

void draw_line(ctx_t* c, int u, int v_from, int v_to, uint32_t color);

void render(const player_t* p, ctx_t* c);

int bonus_implemented(void);
