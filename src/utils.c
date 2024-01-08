#include "utils.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>

char* read_cstr_file(
    const char* restrict file_path, const char* mode, long* n_bytes
) {
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
