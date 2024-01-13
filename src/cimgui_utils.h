#pragma once

#include <stdbool.h>

void load_imgui(void);
void begin_imgui(void);
void end_imgui(void);

void ig_fix_window_top_left(void);
bool ig_collapsing_header(const char* name, bool is_opened);
