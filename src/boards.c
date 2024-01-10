#include "boards.h"

#include "raylib.h"
#include "utils.h"
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int N_BOARDS;
int BOARD_ID;
Board BOARDS[MAX_N_BOARDS];

static bool read_int(const char* data, int* dst, size_t* p) {
    *dst = data[*p];
    *p += sizeof(int);
    if (data[*p] != 0x1F) return false;
    *p += 1;
    return true;
}

void preload_boards(char* file_path) {
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

        board.board_scale = 0.7;
        board.item_scale = 0.1;
        board.item_elevation = 0.5;
        BOARDS[N_BOARDS] = board;

        N_BOARDS += 1;
    }

    load_board(0);
    return;
fail:
    TraceLog(LOG_ERROR, "Failed to parse board %d", N_BOARDS);
    exit(1);
}

void load_board(int board_id) {
    if (board_id == BOARD_ID) {
        TraceLog(LOG_WARNING, "Board %d is already loaded", board_id);
    }

    CHECK_ID_INBOUND(board_id, N_BOARDS, board);
    unload_board();

    BOARD_ID = board_id;
    Board* board = &BOARDS[board_id];

    static char file_path[1024];
    for (size_t i = 0; i < board->n_items; ++i) {
        sprintf(
            file_path, "resources/items/sprites/%s.png", board->items[i].name
        );
        Image image = LoadImage(file_path);
        board->items[i].texture = LoadTextureFromImage(image);
        UnloadImage(image);
    }
}

void unload_board(void) {
    Board* board = &BOARDS[BOARD_ID];
    for (size_t i = 0; i < board->n_items; ++i) {
        Texture2D texture = board->items[i].texture;
        if (IsTextureReady(texture)) {
            UnloadTexture(texture);
        }
    }
}
