#pragma once

#include <stdbool.h>

#include "render.h"

bool store_img(char const* filename, uint32_t* img, int width, int height);

bool initialize_ctx(ctx_t* c, int scr_width, int scr_height, int distance,
                    const char* map_prefix);

void destroy_ctx(ctx_t* c);
