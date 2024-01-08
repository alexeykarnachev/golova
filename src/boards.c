#include "boards.h"

#include "raylib.h"
#include "utils.h"
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int N_BOARDS = 0;
Board BOARDS[MAX_N_BOARDS];

static bool read_int(const char* data, int* dst, size_t* p) {
    *dst = data[*p];
    *p += sizeof(int);
    if (data[*p] != 0x1F) return false;
    *p += 1;
    return true;
}

void load_boards(char* file_path) {
    long n_bytes;
    char* data = read_cstr_file(file_path, "rb", &n_bytes);
    size_t p = 0;

    while (data[p] != 0x1C) {
        Board board = {0};

        if (!read_int(data, &board.n_misses_allowed, &p)) goto fail;
        if (!read_int(data, &board.n_hits_required, &p)) goto fail;
        if (!read_int(data, &board.n_correct_items, &p)) goto fail;

        // items
        while (data[p] != 0x1F) {
            Item item;
            item.is_alive = true;
            item.is_correct = board.n_items < board.n_correct_items;
            strcpy(item.name, &data[p]);

            board.items[board.n_items] = item;
            p += strlen(item.name) + 1;
            board.n_items += 1;
        }
        p += 1;

        // rule
        strcpy(board.rule, &data[p]);
        p += strlen(board.rule) + 1;
        if (data[p] != 0x1F) goto fail;
        p += 1;

        if (data[p] != 0x1E) goto fail;
        p += 1;

        N_BOARDS += 1;
    }

    return;

fail:
    TraceLog(LOG_ERROR, "Failed to parse board %d", N_BOARDS);
    exit(1);
}
