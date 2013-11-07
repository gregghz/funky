#include "funky_buffer.h"

text_buffer *tb_new(void) {
    text_buffer *tb=calloc(1, sizeof(text_buffer));
    tb->len=0;
    tb->siz=16;
    tb->txt=calloc(tb->siz, sizeof(char));
    return tb;
}

int tb_wipe(text_buffer *tb) {
    tb->len=0;
    tb->siz=0;
    erase_string(tb->txt);
    free(tb);
    return 0;
}

static int tb_grow(text_buffer *tb) {
    char *tmp;
    unsigned long oldsize=tb->siz;
    while((tb->siz-tb->len)<2) 
        tb->siz+=16;
    if(oldsize==tb->siz)
        return 0;
    tmp=calloc(tb->siz, sizeof(char));
    sprintf(tmp, "%s", tb->txt);
    erase_string(tb->txt);
    tb->txt=tmp;
    return 0;
}

char tb_append(text_buffer *tb, const char nc) {
    tb_grow(tb);
    tb->txt[tb->len++]=nc;
    return nc;
}

void tb_clear(text_buffer *tb) {
    for(;tb->len>0;--tb->len)
        tb->txt[tb->len]='\0';
    tb->txt[tb->len]='\0';
}
