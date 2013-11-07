#ifndef _GRIDDY_HASH_TABLE_
#define _GRIDDY_HASH_TABLE_

#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>

#include "texty.h"

#define GRID_PAGE_SIZE 256
#define LAST_PAGE_ITEM 255
#define GRID_PAGE_HALF 128
#define FIRST_PAGE_ITEM 0

#define GRID_KEY_NOT_FOUND 0
#define GRID_KEY_EXISTS 1

typedef struct {
    char *keys[GRID_PAGE_SIZE];
    unsigned char hash_helper[GRID_PAGE_SIZE];
    void *page_dat[GRID_PAGE_SIZE];
    void *next_page;
} grid_t;

grid_t *new_grid(void);
int wipe_grid(grid_t *grid);
const char *key_at_page_idx(const grid_t *pg, const unsigned char idx);
int set_grid_item(grid_t *grid, const char *key, void *val);
void *get_grid_item(grid_t *pg, const char *key);
int grid_key_exists(grid_t *pg, const char *key);
int grid_page_count(const grid_t *pg);
int grid_item_count(const grid_t *pg);
char **grid_keys_list(grid_t *pg);

int wipe_keys_list(char **keys_list);

#endif
