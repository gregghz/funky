#include "funky.h"

static thing_th *rejigger_with_left_as_cons(thing_th *left,
                                            thing_th *right,
                                            thing_th *bacro) {
    thing_th *arg1=Cons(Car(left), Cdr(left));
    set_car(left, Atom(sym(bacro)));
    set_cdr(left, Cons(arg1, Cons(right, NULL)));
    return left;
}

static thing_th *rejigger_with_left_as_atom(thing_th *left,
                                            thing_th *right,
                                            thing_th *bacro) {
    thing_th *bcall=Cons(Atom(sym(bacro)),
                         Cons(Car(left),
                              Cons(right, NULL)));
    set_car(left, bcall);
    return left;
}

static thing_th *rejigger_cells(thing_th *left,
                                thing_th *right,
                                thing_th *bacro) {
    if(th_kind(Car(left))==cons_k)
        return rejigger_with_left_as_cons(Car(left), right, bacro);
    return rejigger_with_left_as_atom(left, right, bacro);
}

static thing_th *expand_bacros_in_this_level(thing_th *bacroSrc, 
                                            thing_th *trace, 
                                            thing_th *cur, 
                                            thing_th *prev) {
    thing_th *bacr=NULL;
    thing_th *subs=NULL;
    while(cur) {
        subs=Cdr(cur);
        set_car(trace, cur);
        if(th_kind(Car(cur))==cons_k)
            return Cons(Car(cur), trace);
        if((bacr=Get(bacroSrc, sym(Car(cur))))) {
            if(!prev) {
                fprintf(stderr, "Can't expand bacro onto nothing.\n");
                return NULL;
            }
            rejigger_cells(prev, Car(subs), bacr);
            subs=Cdr(subs);
            set_cdr(prev, subs);
            set_car(trace, subs);
        } else {
            prev=cur;
        }
        cur=subs;
    }
    return Cdr(trace);
}

static thing_th *inner_expand_bacros(thing_th *bacroSrc, thing_th *head) {
    thing_th *trace=Cons(head, NULL);
    thing_th *prev=NULL;
    while(trace) {
        trace=expand_bacros_in_this_level(bacroSrc, trace, Car(trace), prev);
        prev=Car(trace);
        set_car(trace, Cdr(Car(trace)));
    }
    return head;
}

thing_th *expand_bacros(thing_th *head) {
    if(!head)
        return NULL;
    return inner_expand_bacros(lookup_txt("&bacros"), head);
}

static thing_th *read_expressions(FILE *src, text_buffer *tb); 

static thing_th *finalize_reading(text_buffer *tb, 
                                  thing_th *expr) {
    tb_wipe(tb);
    pop_env();
    return expr;
}

static thing_th *read_construct(FILE *src,
                                text_buffer *tb,
                                const char *constructor) {
    return Cons(lookup_txt(constructor), read_expressions(src, tb));
}

static thing_th *read_subcons(thing_th *first, 
                              FILE *src, 
                              text_buffer *tb) {
    return Cons(first, read_expressions(src, tb));
}

static char get_character(FILE *src) {
    char inputChar;
    do {
        inputChar=getc(src);
        if(inputChar!='#')
            return inputChar;
        while((inputChar=getc(src))!='\n');
    } while (inputChar!='#');
    return inputChar;
}

static thing_th *read_string(char starting,
                             FILE *src, 
                             text_buffer *tb) {
    char inputChar;
    while((inputChar=get_character(src))!=starting && inputChar!=EOF)
        tb_append(tb, inputChar);
    return read_subcons(String(tb->txt), src, tb);
}

static thing_th *open_sub_cons(FILE *src, text_buffer *tb, char opener) {
    switch(opener) {
        case '{': 
            return Cons(Atom("grid"), read_expressions(src, tb));
        case '[': 
            return Cons(Atom("list"), read_expressions(src, tb));
        default: 
            return read_expressions(src, tb);
    }
}

static int character_is_break_out_token(char inputChar) {
    switch(inputChar) {
        case ':': case '.': return 1;
        default: return 0;
    }
}

static int literal_terminator_char(char inputChar) {
    return inputChar==EOF || character_is_break_out_token(inputChar) ||
        is_whitespace(inputChar) || 
        is_paren(inputChar) || 
        is_quotation(inputChar);
        
}

static thing_th *read_literals(FILE *src, text_buffer *tb, char inputChar) {
    while(!literal_terminator_char(inputChar)) {
        tb_append(tb, inputChar);
        inputChar=get_character(src);
    }
    if(!is_whitespace(inputChar))
        ungetc(inputChar, src);
    if(is_decimal_text(tb->txt))
        return read_subcons(Number(tb->txt), src, tb);
    return read_subcons(Atom(tb->txt), src, tb);
}

static thing_th *breakout_character(char inputChar) {
    char *label;
    thing_th *atom;
    asprintf(&label, "%c", inputChar);
    atom=Atom(label);
    erase_string(label);
    return atom;
}

static thing_th *read_expressions(FILE *src, text_buffer *tb) {
    char inputChar;
    tb_clear(tb);
    while(is_whitespace(inputChar=get_character(src)));
    if(inputChar==EOF || is_closer(inputChar))
        return NULL;
    if(character_is_break_out_token(inputChar))
        return read_subcons(breakout_character(inputChar), src, tb);
    if(is_quotation(inputChar))
        return read_string(inputChar, src, tb);
    if(is_opener(inputChar)) 
        return read_subcons(open_sub_cons(src, tb, inputChar), src, tb);
    return read_literals(src, tb, inputChar);
}

static int possible_feature_to_skip_bacros(void) {
    return 0;
}

static thing_th *expand_if_necessary(thing_th *rawInputConses) {
    if(possible_feature_to_skip_bacros())
        return rawInputConses;
    return expand_bacros(rawInputConses);
}

static thing_th *read_then_expand_if_necessary(FILE *src, text_buffer *tb) {
    return reg_to_parent(expand_if_necessary(read_expressions(src, tb)));
}

thing_th *read_exprs(FILE *src) {
    text_buffer *tb=tb_new();
    new_env();
    return finalize_reading(tb, read_then_expand_if_necessary(src, tb));
}
