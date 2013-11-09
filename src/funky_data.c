#include "funky.h"

REPR_BLDR(Atom,atom_th,atom_k)
REPR_BLDR(Number,number_th,number_k)
REPR_BLDR(String,string_th,string_k)

REPR_DSTR(del_atom,atom_th)
REPR_DSTR(del_number,number_th)
REPR_DSTR(del_string,string_th)

LST_BLDR(Cons,cons_th,cons_k)
LST_BLDR(Proc,procedure_th,procedure_k)
LST_BLDR(Mac,macro_th,macro_k)
LST_BLDR(Gen,gen_th,gen_k)

LST_DSTR(del_cons,cons_th)
LST_DSTR(del_err,err_th)
LST_DSTR(del_proc,procedure_th)
LST_DSTR(del_mac,macro_th)
LST_DSTR(del_gen,gen_th)

thing_th *Err(thing_th *messages) {
    err_th *this_error=calloc(1, sizeof(err_th));
    this_error->kind=error_k;
    this_error->CAR=Atom("err");
    this_error->CDR=messages;
    return reg_thing((thing_th *)this_error);
}

thing_th *Routine(c_routine theFunction) {
    routine_th *rout=calloc(1, sizeof(routine_th));
    rout->kind=routine_k;
    rout->routine=theFunction;
    return reg_thing((thing_th *)rout);
}

thing_th *Method(c_routine theFunction) {
    routine_th *rout=calloc(1, sizeof(routine_th));
    rout->kind=method_k;
    rout->routine=theFunction;
    return reg_thing((thing_th *)rout);
}

int del_routine(thing_th *thing) {
    routine_th *rout=(routine_th *)thing;
    rout->routine=NULL;
    free(rout);
    return 0;
}

int del_method(thing_th *thing) {
    method_th *mthd=(method_th *)thing;
    mthd->routine=NULL;
    free(mthd);
    return 0;
}

thing_th *Grid(void) {
    grid_th *grid=calloc(1, sizeof(grid_th));
    grid->kind=grid_k;
    grid->data=new_grid();
    return reg_thing((thing_th *)grid);
}

int del_grid(thing_th *grid) {
    grid_th *grd=(grid_th *)grid;
    wipe_grid(grd->data);
    free(grd);
    return 0;
}

kind_t th_kind(const thing_th *thing) {
    if(!thing)
        return null_k;
    return thing->kind;
}

int is_list(const thing_th *thing) {
    if(!thing)
        return 0;
    return th_kind(thing)==cons_k || th_kind(thing)==error_k || th_kind(thing)==procedure_k || th_kind(thing)==macro_k || th_kind(thing)==gen_k;
}

int is_lambda(const thing_th *thing) {
    return th_kind(thing)==procedure_k || th_kind(thing)==routine_k || th_kind(thing)==macro_k || th_kind(thing)==method_k;
}

int is_functor(const thing_th *thing) {
    switch(th_kind(thing)) {
        case method_k: case macro_k: return 1;
        default: return FUNKY_NEGATIVE;
    }
}

int has_repr(const thing_th *thing) {
    if(!thing)
        return FUNKY_NEGATIVE;
    return thing->kind==atom_k || thing->kind==number_k || thing->kind==string_k;
}

static char *th_repr(const thing_th *thing) {
    atom_th *toRepr=(atom_th *)thing;
    return toRepr->repr;
}

const char *sym(const thing_th *thing) {
    switch(th_kind(thing)) {
        case atom_k:
        case number_k:
        case string_k:
            return th_repr(thing);
        case error_k:
            return "(error";
        case cons_k:
        case procedure_k:
        case macro_k:
        case gen_k:
            return "(";
        case grid_k:
            return "{";
        case routine_k:
            return "(routine";
        case method_k:
            return "(method";
        case null_k:
            return "nil";
    }
    return "<thing>";
}

static thing_th *str_first_char(string_th *string) {
    char *character;
    thing_th *atom;
    if(!string->repr)
        return NULL;
    asprintf(&character, "%c", string->repr[0]);
    atom=Atom(character);
    erase_string(character);
    return atom;
}

static thing_th *str_remaining_text(string_th *string) {
    char *text;
    thing_th *atom;
    if(!string->repr || strlen(string->repr)<2)
        return NULL;
    asprintf(&text, "%s", string->repr+1);
    atom=Atom(text);
    erase_string(text);
    return atom;
}

static thing_th *cons_first_thing(const cons_th *cons) {
    if(!cons)
        return NULL;
    return cons->CAR;
}

static thing_th *cons_second_thing(const cons_th *cons) {
    if(!cons)
        return NULL;
    return cons->CDR;
}

thing_th *Car(const thing_th *thing) {
    switch(th_kind(thing)) {
        case atom_k:
        case number_k:
        case method_k:
        case routine_k:
            return NULL;
        case string_k:
            return str_first_char((string_th *)thing);
        case error_k:
        case cons_k:
        case procedure_k:
        case macro_k:
        case gen_k:
            return cons_first_thing((cons_th *)thing);
        case grid_k:
            return Keys(thing);
        case null_k:
            return NULL;
    }
    return NULL;
}

thing_th *Cdr(const thing_th *thing) {
    switch(th_kind(thing)) {
        case atom_k:
        case number_k:
        case string_k:
        case method_k:
        case routine_k:
            return NULL;
        case error_k:
        case cons_k:
        case procedure_k:
        case macro_k:
        case gen_k:
            return cons_second_thing((cons_th *)thing);
        case grid_k:
            return Vals(thing);
        case null_k:
            return NULL;
    }
    return NULL;
}

static c_routine extract_routine(routine_th *rt) {
    return rt->routine;
}

c_routine call_rt(thing_th *thing) {
    switch(th_kind(thing)) {
        case method_k: case routine_k:
            return extract_routine((routine_th *)thing);
        default:
            return NULL;
    }
}

int del_thing(thing_th *thing) {
    switch(th_kind(thing)) {
        case atom_k: return del_atom(thing);
        case number_k: return del_number(thing);
        case routine_k: return del_routine(thing);
        case method_k: return del_method(thing);
        case string_k: return del_string(thing);
        case error_k: return del_err(thing);
        case cons_k: return del_cons(thing);
        case procedure_k: return del_proc(thing);
        case macro_k: return del_mac(thing);
        case grid_k: return del_grid(thing);
        case gen_k: return del_gen(thing);
        case null_k: return 1;
    }
    return 1;
}

static thing_th *set_cons_cdr(cons_th *cons, thing_th *cdr) {
    cons->CDR=cdr;
    return cdr;
}

static thing_th *set_cons_car(cons_th *cons, thing_th *car) {
    cons->CAR=car;
    return car;
}

thing_th *set_cdr(thing_th *cell, thing_th *cdr) {
    if(!cell || !is_list(cell))
        return NULL;
    return set_cons_cdr((cons_th *)cell, cdr);
}

thing_th *set_car(thing_th *cell, thing_th *car) {
    if(!is_list(cell))
        return NULL;
    return set_cons_car((cons_th *)cell, car);
}

static thing_th *grid_set(grid_th *grid, const char *kw, thing_th *val) {
    set_grid_item(grid->data, kw, (void *)val);
    return val;
}

static thing_th *grid_get(grid_th *grid, const char *kw) {
    return (thing_th *)get_grid_item(grid->data, kw);
}

thing_th *Set(thing_th *grid, const char *kw, thing_th *val) {
    if(!grid || th_kind(grid)!=grid_k)
        return NULL;
    return grid_set((grid_th *)grid, kw, val);
}

thing_th *Get(thing_th *grid, const char *kw) {
    if(!grid || th_kind(grid)!=grid_k)
        return NULL;
    return grid_get((grid_th *)grid, kw);
}

int grid_has(grid_th *grid, const char *kw) {
    return grid_key_exists(grid->data, kw);
}

int Has(thing_th *grid, const char *kw) {
    if(!grid || th_kind(grid)!=grid_k) 
        return 0;
    return grid_has((grid_th *)grid, kw);
}

thing_th *last_el(thing_th *thing) {
    if(!thing || !is_list(thing))
        return NULL;
    while(thing && is_list(Cdr(thing)))
        thing=Cdr(thing);
    return thing;
}

thing_th *append(thing_th *left, thing_th *right) {
    if(!left || !is_list(left))
        return NULL;
    set_cdr(last_el(left), right);
    return left;
}

static thing_th *grid_keys(grid_th *grid) {
    char **keys=grid_keys_list(grid->data);
    char **kw=keys;
    thing_th *allKeys;
    if(!keys || !*keys)
        return NULL;
    allKeys=Cons(Atom(*kw++), NULL);
    while(kw && *kw) {
        append(allKeys, Cons(Atom(*kw), NULL));
        kw++;
    }
    wipe_keys_list(keys);
    return allKeys;
}

thing_th *Keys(const thing_th *grid) {
    if(!grid || th_kind(grid)!=grid_k)
        return NULL;
    return grid_keys((grid_th *)grid);
}

static thing_th *grid_vals(grid_th *grid) {
    thing_th *keys=grid_keys(grid);
    thing_th *values;
    if(!keys)
        return NULL;
    values=Cons(grid_get(grid, sym(Car(keys))), NULL);
    keys=Cdr(keys);
    while(keys) {
        append(values, Cons(grid_get(grid, sym(Car(keys))), NULL));
        keys=Cdr(keys);
    }
    return values;
}

thing_th *Vals(const thing_th *grid) {
    if(!grid || th_kind(grid)!=grid_k) 
        return NULL;
    return grid_vals((grid_th *)grid);
}

thing_th *insert(thing_th *left, thing_th *right) {
    thing_th *tmp;
    if(!left || !is_list(left))
        return NULL;
    if(!is_list(right))
        right=Cons(right, NULL);
    tmp=Cdr(left);
    set_cdr(left, append(right, tmp));
    return right;
}

thing_th *accumulate(thing_th *thing) {
    thing_th *accum=Cons(thing, NULL);
    thing_th *cur=accum;
    while(cur) {
        thing_th *item=Car(cur);
        if(th_kind(item)==grid_k)
            insert(cur, Vals(item));
        if(Cdr(item))
            insert(cur, Cons(Cdr(item), NULL));
        if(Car(item))
            insert(cur, Cons(Car(item), NULL));
        cur=Cdr(cur);
    }
    return accum;
}

static thing_th *duplicate_grid(thing_th *gridsrc) {
    return NULL;
}

static thing_th *dup_cell(thing_th *thing) {
    switch(th_kind(thing)) {
        case number_k:
            return Number(sym(thing));
        case string_k:
            return String(sym(thing));
        case atom_k:
            return Atom(sym(thing));
        case cons_k:
            return Cons(Car(thing), Cdr(thing));
        case error_k:
            return Err(Cdr(thing));
        case procedure_k:
            return Proc(Car(thing), Cdr(thing));
        case macro_k:
            return Mac(Car(thing), Cdr(thing));
        case gen_k:
            return Gen(Car(thing), Cdr(thing));
        case routine_k:
            return Routine(call_rt(thing));
        case method_k:
            return Method(call_rt(thing));
        case grid_k:
            return duplicate_grid(thing);
        case null_k:
            return NULL;
    }
}

static thing_th *inner_dup(thing_th *head) {
    if(!head)
        return NULL;
    if(Car(head))
        set_car(head, dup_cell(Car(head)));
    if(Cdr(head))
        set_cdr(head, dup_cell(Cdr(head)));
    inner_dup(Car(head));
    inner_dup(Cdr(head));
    return head;
}

thing_th *duplicate(thing_th *head) {
    return inner_dup(dup_cell(head));
}
