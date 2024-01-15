#pragma once

#include <stdbool.h>

extern int GHOST_WINDOW_FLAGS;

void load_imgui(void);
void begin_imgui(void);
void end_imgui(void);

void ig_fix_window_top_left(void);
void ig_fix_window_bot_left(void);
bool ig_collapsing_header(const char* name, bool is_opened);
