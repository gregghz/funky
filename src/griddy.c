#include "griddy.h"

static unsigned char grid_page_hash(const grid_t *gridpage, const char *key){
    unsigned char hash = 0;
    while(*key) hash = gridpage->hash_helper[hash ^ *key++];
    return hash;
}

static void set_straight_hash(grid_t *grid) {
    unsigned int i;
    for(i=0;i<GRID_PAGE_SIZE;++i)
        grid->hash_helper[i]=i;
}

static void maybe_swap_things(unsigned char *x, unsigned char *y) {
    unsigned char t=*x;
    if(rand()%2)
        return;
    (*x)=*y;
    (*y)=t;
}

static void jigger_hash(grid_t *gridpage) {
    unsigned int i;
    for (i=0;i<GRID_PAGE_SIZE;++i) {
        maybe_swap_things(&gridpage->hash_helper[rand()%GRID_PAGE_HALF], 
                          &gridpage->hash_helper[GRID_PAGE_HALF+rand()%GRID_PAGE_HALF]);
    }
}

static void randomize_hash(grid_t *gridpage, unsigned int passes) {
    for(;passes>0;--passes)
        jigger_hash(gridpage);
}

static void initialize_hash(grid_t *gridpage) {
    set_straight_hash(gridpage);
    randomize_hash(gridpage, 10);
}

static grid_t *new_grid_page(grid_t *new_page) {
    if(!new_page)
        return NULL;
    initialize_hash(new_page);
    return new_page;
}

grid_t *new_grid(void) {
    return new_grid_page(calloc(1, sizeof(grid_t)));
}

static grid_t *wipe_page_and_get_next(grid_t *pg) {
    unsigned int i;
    grid_t *nxt;
    if(!pg)
        return NULL;
    nxt=pg->next_page;
    for(i=0;i<GRID_PAGE_SIZE;++i)
        if(pg->keys[i])
            erase_string(pg->keys[i]);
    free(pg);
    return nxt;
}

int wipe_grid(grid_t *grid) {
    while((grid=wipe_page_and_get_next(grid)));
    return 0;
}

const char *key_at_page_idx(const grid_t *pg, const unsigned char idx) {
    return pg->keys[idx];
}

static void *pointer_at_page_idx(const grid_t *pg, const unsigned char idx) {
    return pg->page_dat[idx];
}

static int safely_set_page_item_or_bail(grid_t *pg, const char *key, void *val) {
    unsigned char idx=grid_page_hash(pg, key);
    if(pg->keys[idx]) {
        if(strcmp(pg->keys[idx],key)!=0)
            return 0;
    } else {
        asprintf(&pg->keys[idx], "%s", key);
    }
    pg->page_dat[idx]=val;
    return 1;
}

int set_grid_item(grid_t *grid, const char *key, void *val) {
    grid_t *gp=grid;
    unsigned int page_depth=1;
    while(!safely_set_page_item_or_bail(gp, key, val)) {
        if(!gp->next_page)
            gp->next_page=new_grid();
        gp=gp->next_page;
        page_depth++;
    }
    return page_depth;
}

static int key_is_found_in_page(const grid_t *gridpage, 
                                const char *key, 
                                const unsigned char idx) {
    return streq(key, key_at_page_idx(gridpage, idx));
}

void *get_grid_item(grid_t *pg, const char *key) {
    while(pg) {
        unsigned char idx=grid_page_hash(pg, key);
        if(key_is_found_in_page(pg, key, idx))
            return pointer_at_page_idx(pg, idx);
        pg=pg->next_page;
    }
    return NULL;
}

int grid_key_exists(grid_t *pg, const char *key) {
    while(pg) {
        unsigned char idx=grid_page_hash(pg, key);
        if(key_is_found_in_page(pg, key, idx))
            return GRID_KEY_EXISTS;
        pg=pg->next_page;
    }
    return GRID_KEY_NOT_FOUND;
}

int grid_item_count(const grid_t *pg) {
    int i;
    int ref_count=0;
    while(pg) {
        for(i=0;i<GRID_PAGE_SIZE;++i)
            if(pg->keys[i])
                ++ref_count;
        pg=pg->next_page;
    }
    return ref_count;
}

int grid_page_count(const grid_t *pg) {
    int quantity;
    for(quantity=0;pg && ++quantity; pg=pg->next_page);
    return quantity;
}

char **grid_keys_list(grid_t *pg) {
    unsigned int i;
    unsigned int cur=0;
    unsigned long quantity=grid_item_count(pg)+1;
    char **keys_list=calloc(quantity, sizeof(char *));
    while (pg) {
        for(i=0;i<GRID_PAGE_SIZE;++i)
            if(pg->keys[i])
                asprintf(&keys_list[cur++], "%s", pg->keys[i]);
        pg=pg->next_page;
    }
    keys_list[cur]=NULL;
    return keys_list;
}

int wipe_keys_list(char **keys_list) {
    char **current=keys_list;
    while(*current) {
        erase_string(*current);
        *current=NULL;
        ++current;
    }
    free(keys_list);
    return 0;
}
