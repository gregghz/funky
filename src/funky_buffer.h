#ifndef _FUNKY_BUFFER_HEADER_FILE_
#define _FUNKY_BUFFER_HEADER_FILE_

#include <stdlib.h>
#include "texty.h"

typedef struct {
    unsigned long len;
    unsigned long siz;
    char *txt;
} text_buffer;

text_buffer *tb_new(void);
int tb_wipe(text_buffer *tb);
char tb_append(text_buffer *tb, const char nc);
void tb_clear(text_buffer *tb);

#endif
