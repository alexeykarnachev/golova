#pragma once
#include <stdbool.h>

#define STRINGIFY(x) #x

char* read_cstr_file(const char* restrict file_path, const char* mode, long* n_bytes);

char** get_file_names_in_dir(char* path, int* n_file_names);
void get_file_name(char* dst, char* path, bool strip_ext);
