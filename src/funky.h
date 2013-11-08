#ifndef _FUNKY_HEADER_FILE_
#define _FUNKY_HEADER_FILE_

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "texty.h"
#include "griddy.h"

#include "funky_buffer.h"

#define FUNKY_NEGATIVE 0
#define FUNKY_AFFIRMATIVE 1

#define ANONYMOUS_KW "&anon"
#define UNKNOWN_HANDLER "&unknown-handler"
#define UNKNOWN_NIL "&unknown-nil"
#define UNKNOWN_LIT "&unknown-lit"
#define UNKNOWN_ERR "&unknown-err"

#define ERRMSG_TYPES "Type error"
#define ERRMSG_BADDEF "Cannot construct a procedure with the data given."

typedef enum {
    null_k,
    error_k,
    atom_k,
    number_k,
    string_k,
    cons_k,
    grid_k,
    routine_k,
    method_k,
    procedure_k,
    macro_k,
    gen_k
} kind_t;

typedef struct {
    kind_t kind;
} thing_th;

#define REPR_TH(x) typedef struct { kind_t kind; char *repr; } x
#define LST_TH(x) typedef struct { kind_t kind; thing_th *CAR; thing_th *CDR; } x

REPR_TH(atom_th);
REPR_TH(number_th);
REPR_TH(string_th);

LST_TH(cons_th);
LST_TH(err_th);
LST_TH(procedure_th);
LST_TH(macro_th);
LST_TH(gen_th);

typedef thing_th *(*c_routine)(thing_th *);

typedef struct {
    kind_t kind; 
    c_routine routine;
} routine_th;

typedef struct {
    kind_t kind; 
    c_routine routine;
} method_th;

typedef struct {
    kind_t kind; 
    grid_t *data;
} grid_th;

#define REPR_BLDR(fnm,typ,knd) thing_th *fnm(const char *label) { typ *newThing=calloc(1, sizeof(typ)); newThing->kind=knd; asprintf(&newThing->repr, "%s", label); return reg_thing((thing_th *)newThing); } 

#define REPR_DSTR(fnm,typ) int fnm(thing_th *delMe) { typ *thing=(typ *)delMe; erase_string(thing->repr); free(thing); return 0; }

#define LST_BLDR(fnm,typ,knd) thing_th *fnm(thing_th *car, thing_th *cdr) { typ *newThing=calloc(1, sizeof(typ)); newThing->kind=knd; newThing->CAR=car; newThing->CDR=cdr; return reg_thing((thing_th *)newThing); }

#define LST_DSTR(fnm,typ) int fnm(thing_th *delMe) { typ *thing=(typ *)delMe; thing->CAR=NULL; thing->CDR=NULL; free(thing); return 0; }

int del_atom(thing_th *delMe);
int del_number(thing_th *delMe);
int del_string(thing_th *delMe);

int del_cons(thing_th *delMe);
int del_err(thing_th *delMe);
int del_proc(thing_th *delMe);
int del_mac(thing_th *delMe);
int del_gen(thing_th *delMe);

thing_th *reg_thing(thing_th *thing);

thing_th *Atom(const char *label);
thing_th *Number(const char *label);
thing_th *String(const char *label);
thing_th *Cons(thing_th *car, thing_th *cdr);
thing_th *Err(thing_th *messages);
thing_th *Proc(thing_th *car, thing_th *cdr);
thing_th *Mac(thing_th *car, thing_th *cdr);
thing_th *Gen(thing_th *car, thing_th *cdr);

thing_th *Routine(c_routine theFunction);
thing_th *Method(c_routine theFunction);
int del_routine(thing_th *thing);
thing_th *Grid(void);
int del_grid(thing_th *grid);
kind_t th_kind(const thing_th *thing);
int is_list(const thing_th *thing);
int is_lambda(const thing_th *thing);
int is_functor(const thing_th *thing);
int has_repr(const thing_th *thing);
const char *sym(const thing_th *thing);
thing_th *Car(const thing_th *thing);
thing_th *Cdr(const thing_th *thing);
c_routine call_rt(thing_th *thing);
int del_thing(thing_th *thing);
thing_th *set_cdr(thing_th *cell, thing_th *cdr);
thing_th *set_car(thing_th *cell, thing_th *car);
thing_th *Set(thing_th *grid, const char *kw, thing_th *val);
thing_th *Get(thing_th *grid, const char *kw);
int Has(thing_th *grid, const char *kw);
thing_th *last_el(thing_th *thing);
thing_th *append(thing_th *left, thing_th *right);
thing_th *Keys(thing_th *grid);
thing_th *Vals(thing_th *grid);
thing_th *insert(thing_th *left, thing_th *right);
thing_th *accumulate(thing_th *thing);
thing_th *duplicate(thing_th *thing);
unsigned int count_env_levels(void);
thing_th *reg_to_parent(thing_th *thing);

int SKIP_REG;
thing_th *rootEnvironment;
thing_th *rootBacros;
thing_th *env;
unsigned int env_levels;

int establish_root_environment(void);
thing_th *scope_containing(const char *label);
thing_th *lookup_txt(const char *label);
thing_th *lookup_sym(const thing_th *label);
int new_env(void);
thing_th *push_env(thing_th *newScope);
int pop_env(void);
thing_th *env_set(const char *label, thing_th *thing);
void wipe_env(void);

thing_th *eval(thing_th *expr);
thing_th *apply(thing_th *lambdaAndArgs);

thing_th *dirty_sum(thing_th *args); 
thing_th *dirty_sub(thing_th *args);
thing_th *funky_if(thing_th *args);
thing_th *funky_set(thing_th *args);
thing_th *funky_print(thing_th *args);
thing_th *funky_macro(thing_th *args);
thing_th *funky_quote(thing_th *args);
thing_th *funky_head(thing_th *args);
thing_th *funky_rest(thing_th *args);
thing_th *funky_list(thing_th *args);
thing_th *funky_def(thing_th *args);
thing_th *funky_last(thing_th *args);
thing_th *funky_err(thing_th *args);
thing_th *funky_dump(thing_th *args);
thing_th *funky_greater_than(thing_th *args);
thing_th *funky_less_than(thing_th *args);
thing_th *funky_equivalent(thing_th *args);
thing_th *funky_not_operator(thing_th *args);
thing_th *funky_evaluator(thing_th *args);
thing_th *funky_pair(thing_th *args);
thing_th *funky_grid(thing_th *args);
thing_th *funky_grid_get(thing_th *args);
thing_th *funky_truthy(thing_th *args);
thing_th *funky_nilly(thing_th *args);
thing_th *funky_callable(thing_th *args);
thing_th *funky_is_atom(thing_th *args);
thing_th *funky_is_gen(thing_th *args);
thing_th *funky_length(thing_th *args);
thing_th *funky_gen(thing_th *args);
thing_th *funky_cons(thing_th *args);
thing_th *funky_append(thing_th *args);

thing_th *depict(thing_th *thing);
thing_th *depict_error(thing_th *errThing);
const char *debug_lbl(const thing_th *thing);
int debug_list(const thing_th *thing);

thing_th *read_exprs(FILE *src);
thing_th *expand_bacros(thing_th *head);

#endif

