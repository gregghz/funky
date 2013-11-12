#include "funky.h"

static long text_to_long(thing_th *numsrc) {
    char *lastValidChar;
    long output=strtol(sym(numsrc), &lastValidChar, 10);
    if(*lastValidChar!='\0')
        fprintf(stderr, "Invalid numeric value: %s\n", sym(numsrc));
    return output;
}

thing_th *dirty_sum(thing_th *args) {
    long num=0;
    char *outty;
    thing_th *output;
    if(!args)
        return Number("0");
    while(args) {
        num+=text_to_long(Car(args));
        args=Cdr(args);
    }
    asprintf(&outty, "%ld", num);
    output=Number(outty);
    erase_string(outty);
    return output;
}

thing_th *dirty_sub(thing_th *args) {
    long num=0;
    char *outty;
    thing_th *output;
    if(!args)
        return Number("0");
    if(Cdr(args)) {
        num=text_to_long(Car(args));
        args=Cdr(args);
    }
    while(args) {
        num-=text_to_long(Car(args));
        args=Cdr(args);
    }
    asprintf(&outty, "%ld", num);
    output=Number(outty);
    erase_string(outty);
    return output;
}

static thing_th *inner_funky_if(thing_th *predicate,
                                thing_th *positive, 
                                thing_th *negative) {
    return eval(eval(predicate) ? positive : negative);
}

thing_th *funky_if(thing_th *args) {
    return inner_funky_if(Car(args),
                          Car(Cdr(args)),
                          Car(Cdr(Cdr(args))));
}

static thing_th *inner_funky_set(thing_th *label,
                                 thing_th *expr) {
    return env_set(sym(label), expr);
}

thing_th *funky_set(thing_th *args) {
    return inner_funky_set(Car(args),
                           Car(Cdr(args)));
}

thing_th *funky_print(thing_th *args) {
    thing_th *prev;
    while(args) {
        prev=Car(args);
        printf("%s", sym(prev));
        args=Cdr(args);
        if(args)
            printf(" ");
    }
    printf("\n");
    return prev;
}

static thing_th *build_macro(thing_th *args) {
    return Mac(Car(args), Cdr(args));
}

thing_th *funky_macro(thing_th *args) {
    if(has_repr(Car(args))) {
        return env_set(sym(Car(args)), build_macro(Cdr(args)));
    }
    return build_macro(args);
}

thing_th *funky_quote(thing_th *args) {
    return Car(args);
}

thing_th *funky_head(thing_th *args) {
    return Car(Car(args));
}

thing_th *funky_rest(thing_th *args) {
    return Cdr(Car(args));
}

thing_th *funky_list(thing_th *args) {
    return args;
}

static thing_th *define_procedure(thing_th *args) {
    return Proc(Car(args), Cdr(args));
}

thing_th *funky_def(thing_th *args) {
    switch(th_kind(Car(args))) {
        case atom_k:
            return env_set(sym(Car(args)),
                           define_procedure(Cdr(args)));
        case cons_k:
            return define_procedure(args);
        default:
            return Err(Cons(Atom(ERRMSG_TYPES),
                            Cons(Atom(ERRMSG_BADDEF), NULL)));
    }
    if(th_kind(Car(args))==atom_k)
        return env_set(sym(Car(args)), 
                       define_procedure(Cdr(args)));
    return define_procedure(args);
}

thing_th *funky_last(thing_th *args) {
    return Car(last_el(Car(args)));
}

thing_th *funky_err(thing_th *args) {
    return Err(args);
}

thing_th *funky_dump(thing_th *args) {
    while(args) {
        debug_list(Car(args));
        args=Cdr(args);
    }
    return args;
}

static int funky_compare_numbers(thing_th *left, thing_th *right) {
    long lnum=text_to_long(left);
    long rnum=text_to_long(right);
    return lnum>rnum;
}

static int funky_compare_strings(const char *left, const char *right) {
    int i=0;
    int llen=strlen(left);
    int rlen=strlen(right);
    int max=(llen>rlen) ? rlen : llen;
    for(i=0;i<max;++i)
        if(left[i]<right[i])
            return 0;
    return 1;
}

static int lft_greater_than_rit(thing_th *lft, thing_th *rit) {
    if(th_kind(lft)==number_k && th_kind(rit)==number_k)
        return funky_compare_numbers(lft, rit);
    if(th_kind(lft)==string_k && th_kind(rit)==string_k)
        return funky_compare_strings(sym(lft), sym(rit));
    fprintf(stderr, "Don't know how to compare the types given.\n");
    return 0;
}

thing_th *funky_greater_than(thing_th *args) {
    thing_th *cur=args;
    while(cur && Cdr(cur)) {
        if(!lft_greater_than_rit(Car(cur), Car(Cdr(cur))))
           return NULL;
        cur=Cdr(cur);
    }
    return lookup_txt("true");
}

thing_th *funky_less_than(thing_th *args) {
    thing_th *cur=args;
    while(cur && Cdr(cur)) {
        if(!lft_greater_than_rit(Car(Cdr(cur)), Car(cur)))
           return NULL;
        cur=Cdr(cur);
    }
    return lookup_txt("true");
}

static int funky_things_are_equivalent(thing_th *left, thing_th *right) {
    return th_kind(left)==th_kind(right) && streq(sym(left), sym(right));
}

#define TH_CMP(cmpr) recursive_lists_are_equivalent(cmpr(left), cmpr(right))

static int recursive_lists_are_equivalent(thing_th *left, thing_th *right) {
    if(!funky_things_are_equivalent(left, right))
        return 0;
    return !left || (TH_CMP(Car) && TH_CMP(Cdr));
}

thing_th *funky_equivalent(thing_th *args) {
    thing_th *cur=args;
    while(cur && Cdr(cur)) {
        if(!recursive_lists_are_equivalent(Car(cur), Car(Cdr(cur))))
            return NULL;
        cur=Cdr(cur);
    }
    return lookup_txt("true");
}

thing_th *funky_not_operator(thing_th *args) {
    if(!Car(args))
        return lookup_txt("true");
    return NULL;
}

static thing_th *horrendous_recursion_trap_eval(thing_th *args) {
    return eval(Car(args));
}

thing_th *funky_evaluator(thing_th *args) {
    return horrendous_recursion_trap_eval(args);
}

thing_th *funky_pair(thing_th *args) {
    return Cons(Car(args), 
                Cons(Car(Cdr(args)), 
                     NULL));
}

static int set_grid_pair_data(thing_th *grid,
                              thing_th *key,
                              thing_th *val) {
    if(!key)
        return 0;
    Set(grid, sym(key), val);
    return 1;
}

static int set_grid_pair(thing_th *grid, thing_th *pair) {
    return pair && set_grid_pair_data(grid, Car(pair), Car(Cdr(pair)));
}

thing_th *funky_grid(thing_th *args) {
    thing_th *grid=Grid();
    while(set_grid_pair(grid, Car(args)))
        args=Cdr(args);
    return grid;
}

static thing_th *funky_grid_getter(thing_th *grid,
                                   thing_th *keyword) {
    return Get(grid, sym(keyword));
}

thing_th *funky_grid_get(thing_th *args) {
    return funky_grid_getter(Car(args),
                             Car(Cdr(args)));
}

static int this_cell_is_truthy(thing_th *cell) {
    switch(th_kind(cell)) {
        case grid_k:
            return Keys(cell)!=NULL;
        case error_k: case null_k:
            return 0;
        case cons_k: case macro_k: 
        case gen_k: case procedure_k:
            return Car(cell)!=NULL || Cdr(cell)!=NULL;
        case atom_k:
            return !streq(sym(cell), "nil");
        case number_k: case string_k:
            return 1;
        case routine_k: case method_k:
            return call_rt(cell)!=NULL;
    }
}

thing_th *funky_truthy(thing_th *args) {
    return this_cell_is_truthy(Car(args)) ? lookup_txt("true") : NULL;
}

thing_th *funky_nilly(thing_th *args) {
    return Car(args) ? NULL : lookup_txt("true");
}

thing_th *funky_callable(thing_th *args) {
    while(args) {
        if(!is_lambda(Car(args)))
            return NULL;
        args=Cdr(args);
    }
    return lookup_txt("true");
}

thing_th *funky_is_atom(thing_th *args) {
    while(args) {
        if(th_kind(Car(args))!=atom_k)
            return NULL;
        args=Cdr(args);
    }
    return lookup_txt("true");
}

thing_th *funky_is_gen(thing_th *args) {
    return (th_kind(Car(args))==gen_k) ? lookup_txt("true") : NULL;
}

static thing_th *inner_funky_length(thing_th *args) {
    unsigned long len=0;
    char *num;
    thing_th *outcome;
    while(args && (Cdr(args) || Car(args))) {
        ++len;
        args=Cdr(args);
    }
    asprintf(&num, "%ld", len);
    outcome=Number(num);
    erase_string(num);
    return outcome;
}

thing_th *funky_length(thing_th *args) {
    return inner_funky_length(Car(args));
}

thing_th *funky_gen(thing_th *args) {
    if(!Car(args) || !Cdr(args))
        return Err(Cons(String("Incomplete Gen construction"), NULL));
    return Gen(Car(args), Cdr(args));
}

thing_th *funky_cons(thing_th *args) {
    return Cons(Car(args), Car(Cdr(args)));
}

thing_th *funky_append(thing_th *args) {
    thing_th *output=duplicate(Car(args));
    while(Cdr(args)) {
        output=append(output, Car(Cdr(args)));
        args=Cdr(args);
    }
    return output;
}

thing_th *funky_is_error(thing_th *args) {
    return (th_kind(Car(args))==error_k) ? lookup_txt("true") : NULL;
}

thing_th *funky_is_grid(thing_th *args) {
    return (th_kind(Car(args))==grid_k) ? lookup_txt("true") : NULL;
}

static thing_th *inner_mktxt(text_buffer *tb, thing_th *args) {
    while(args) {
        const char *txt=sym(Car(args));
        long i;
        long m=strlen(txt);
        for(i=0;i<m;++i) {
            tb_append(tb, txt[i]);
        }
        args=Cdr(args);
    }
    return String(tb->txt);
}

thing_th *funky_make_txt(thing_th *args) {
    text_buffer *tb=tb_new();
    thing_th *ret=inner_mktxt(tb, args);
    tb_wipe(tb);
    return ret;
}

