#pragma once
#include <stdbool.h>

#define MAX_N_BOARDS 16
#define MAX_N_ITEMS 36
#define MAX_ITEM_NAME_LENGTH 32
#define MAX_RULE_LENGTH 128

typedef struct Item {
    char name[MAX_ITEM_NAME_LENGTH];
    bool is_correct;
    bool is_alive;
} Item;

typedef struct Board {
    int n_misses_allowed;
    int n_hits_required;

    int n_items;
    int n_correct_items;
    Item items[MAX_N_ITEMS];

    char rule[MAX_RULE_LENGTH];
} Board;

extern int N_BOARDS;
extern Board BOARDS[MAX_N_BOARDS];

void load_boards(char* file_path);
void unload_boards(void);
