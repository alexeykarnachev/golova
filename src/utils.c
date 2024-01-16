#include "utils.h"

#include "raylib.h"
#include <dirent.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_FILE_NAMES 1024

char* read_cstr_file(const char* restrict file_path, const char* mode, long* n_bytes) {
    FILE* file = NULL;
    char* content = NULL;

    file = fopen(file_path, mode);
    if (file == NULL) goto fail;
    if (fseek(file, 0, SEEK_END) < 0) goto fail;

    long size = ftell(file);
    if (size < 0) goto fail;
    if (fseek(file, 0, SEEK_SET) < 0) goto fail;

    content = malloc(size + 1);
    if (content == NULL) goto fail;

    long read_size = fread(content, 1, size, file);
    if (ferror(file)) goto fail;

    content[size] = '\0';
    if (file) {
        fclose(file);
        errno = 0;
    }

    if (n_bytes != NULL) {
        *n_bytes = size;
    }
    return content;

fail:
    if (file) {
        int e = errno;
        fclose(file);
        errno = e;
    }
    if (content) {
        free(content);
    }
    return NULL;
}

char** get_file_names_in_dir(const char* path, int* n_file_names) {
    DIR* dir = opendir(path);
    if (dir == NULL) {
        TraceLog(LOG_ERROR, "Failed to open directory %s", path);
        exit(1);
    }

    char** file_names = (char**)malloc(MAX_FILE_NAMES * sizeof(char*));

    int i = 0;
    struct dirent* entry;
    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_type == DT_REG) {
            file_names[i] = strdup(entry->d_name);
            i += 1;
        }
    }

    *n_file_names = i;
    closedir(dir);
    return file_names;
}

void get_file_name(char* dst, const char* path, bool strip_ext) {
    // Find the last occurrence of the path separator ('/' or '\') to locate the
    // start of the file name
    const char* file_name_start = strrchr(path, '/');
    if (file_name_start == NULL) {
        file_name_start = strrchr(path, '\\');
    }

    // If no path separator found, consider the entire path as the file name
    if (file_name_start == NULL) {
        file_name_start = path;
    } else {
        // Move one position ahead to skip the path separator
        file_name_start++;
    }

    // Find the last occurrence of the dot('.') to locate the start of the file
    // extension
    const char* file_extension_start = strrchr(file_name_start, '.');

    // Calculate the length of the file name (excluding extension if needed)
    size_t file_name_length = (file_extension_start && strip_ext)
                                  ? (size_t)(file_extension_start - file_name_start)
                                  : strlen(file_name_start);

    // Copy the file name into the provided result buffer
    strncpy(dst, file_name_start, file_name_length);
    dst[file_name_length] = '\0';  // Null-terminate the string
}
