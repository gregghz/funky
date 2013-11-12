#include "funky.h"

static thing_th *spawn_env(thing_th *prevEnv, thing_th *scope) {
    thing_th *anon;
    if(th_kind(scope)!=grid_k)
        return env;
    anon=Primordial_Cons(NULL, NULL, GC_SKIPREG);
    Set(scope, ANONYMOUS_KW, anon);
    Set(scope, ANON_TAIL_KW, anon);
    return env=Primordial_Cons(scope, prevEnv, GC_SKIPREG);
}

thing_th *push_env(thing_th *newScope) {
    return spawn_env(env, newScope);
}

int new_env(void) {
    return spawn_env(env, Primordial_Grid(GC_SKIPREG)) ? 0 : 1;
}

static int establish_bacros(thing_th *bacroGrid) {
    if(!bacroGrid) return 1;
    Set(bacroGrid, "~>", Atom("rapply"));
    Set(bacroGrid, "<~", Atom("safely-apply"));
    Set(bacroGrid, "->", Atom("rcall"));
    Set(bacroGrid, "<-", Atom("call"));
    Set(bacroGrid, "<S", Atom("strict-apply"));
    Set(bacroGrid, "S>", Atom("strict-rapply"));
    Set(bacroGrid, ":", Atom("pair"));
    Set(bacroGrid, ".", Atom("get"));
    return 0;
}

int establish_root_environment(void) {
    spawn_env(NULL, Primordial_Grid(GC_SKIPREG));
    rootEnvironment=Car(env);
    rootBacros=Grid();
    unknownSymbolError=Err(Cons(String("Unknown symbol"), NULL));
    Set(rootEnvironment, "nil", NULL);
    Set(rootEnvironment, "true", Atom("true"));
    Set(rootEnvironment, "add", Routine(&dirty_sum));
    Set(rootEnvironment, "+", Get(rootEnvironment, "add"));
    Set(rootEnvironment, "subtract", Routine(&dirty_sub));
    Set(rootEnvironment, "-", Get(rootEnvironment, "subtract"));
    Set(rootEnvironment, "if", Method(&funky_if));
    Set(rootEnvironment, "&ver", String("Funky Lisp Draft 3"));
    Set(rootEnvironment, "set!", Routine(&funky_set));
    Set(rootEnvironment, "print_", Routine(&funky_print));
    Set(rootEnvironment, "list", Routine(&funky_list));
    Set(rootEnvironment, "pair", Routine(&funky_pair));
    Set(rootEnvironment, "grid", Routine(&funky_grid));
    Set(rootEnvironment, "get", Routine(&funky_grid_get));
    Set(rootEnvironment, "quote", Method(&funky_quote));
    Set(rootEnvironment, "apply", Routine(&apply));
    Set(rootEnvironment, "mac", Method(&funky_macro));
    Set(rootEnvironment, "def", Method(&funky_def));
    Set(rootEnvironment, "head", Routine(&funky_head));
    Set(rootEnvironment, "rest_", Routine(&funky_rest));
    Set(rootEnvironment, "last", Routine(&funky_last));
    Set(rootEnvironment, "err", Routine(&funky_err));
    Set(rootEnvironment, "dump", Routine(&funky_dump));
    Set(rootEnvironment, "&bacros", rootBacros);
    Set(rootEnvironment, ">", Routine(&funky_greater_than));
    Set(rootEnvironment, "<", Routine(&funky_less_than));
    Set(rootEnvironment, "=", Routine(&funky_equivalent));
    Set(rootEnvironment, "not", Routine(&funky_not_operator));
    Set(rootEnvironment, "eval", Method(&funky_evaluator));
    Set(rootEnvironment, "true?", Routine(&funky_truthy));
    Set(rootEnvironment, "false?", Routine(&funky_nilly));
    Set(rootEnvironment, "lambda?", Routine(&funky_callable));
    Set(rootEnvironment, "atom?", Routine(&funky_is_atom));
    Set(rootEnvironment, "gen?", Routine(&funky_is_gen));
    Set(rootEnvironment, "len", Routine(&funky_length));
    Set(rootEnvironment, "gen", Routine(&funky_gen));
    Set(rootEnvironment, "cons", Routine(&funky_cons));
    Set(rootEnvironment, "append", Routine(&funky_append));
    Set(rootEnvironment, "error?", Routine(&funky_is_error));
    Set(rootEnvironment, "grid?", Routine(&funky_is_grid));
    Set(rootEnvironment, "txt-concatenate_", Routine(&funky_make_txt));
    Set(rootEnvironment, "type", Routine(&funky_type_symbol));
    Set(rootEnvironment, UNKNOWN_HANDLER, Atom(UNKNOWN_LIT));
    establish_bacros(rootBacros);
    return new_env();
}

thing_th *scope_containing(const char *label) {
    thing_th *cur=env;
    while(cur) {
        if(Has(Car(cur), label))
            return Car(cur);
        cur=Cdr(cur);
    }
    return NULL;
}

static thing_th *unknown_symbol_handling(const char *label,
                                         thing_th *handlingInstructions) {
    if(streq(sym(handlingInstructions), UNKNOWN_ERR))
        return Err(Cons(String("Unknown symbol"),
                        Cons(Atom(label), NULL)));
    if(streq(sym(handlingInstructions), UNKNOWN_LIT))
        return Atom(label);
    return NULL;
}

static thing_th *inner_lookup(const char *label, 
                              thing_th *containingEnv) {
   if(containingEnv)
        return Get(containingEnv, label);
   return unknownSymbolError;
}

thing_th *lookup_txt(const char *label) {
    return inner_lookup(label, scope_containing(label));
}

thing_th *lookup_sym(const thing_th *label) {
    return lookup_txt(sym(label));
}

static int identified(grid_th *catalog, void *addy) {
    int result;
    char *idstr;
    if(!catalog)
        return 0;
    asprintf(&idstr, "%d", (int)addy);
    result=grid_key_exists(catalog->data, idstr);
    erase_string(idstr);
    return result;
}

static thing_th *pop_anonymous_elements(thing_th *cur, thing_th *next) {
    del_thing(Car(cur));
    del_thing(cur);
    return next;
}

static int pop_scope_grids(grid_th *grid) {
    thing_th *cur=(thing_th *)get_grid_item(grid->data, ANONYMOUS_KW);
    while(cur && (cur=pop_anonymous_elements(cur, Cdr(cur))));
    del_grid((thing_th *)grid);
    return 0;
}

static int safely_pop_scope(thing_th *scope) {
    return pop_scope_grids((grid_th *)scope);
}

static int pop_scope(thing_th *scope) {
    if(!scope)
        return 1;
    return safely_pop_scope(scope);
}

static thing_th *delete_enviro_get_nxt(thing_th *enviro, 
                                       thing_th *nxt) {
    pop_scope(Car(enviro));
    del_thing(enviro);
    return nxt;
}

static thing_th *delete_env(thing_th *enviro) {
    return delete_enviro_get_nxt(enviro, Cdr(enviro));
}

int del_env(void) {
    if(!env)
        return 0;
    env=delete_env(env);
    return 1;
}

static int current_env_is_not_top_env() {
    return env && Cdr(env) && Car(Cdr(env))!=rootEnvironment;
}

int pop_env(void) {
    if(current_env_is_not_top_env())
        return del_env();
    return 1;
}

thing_th *env_set(const char *label, thing_th *thing) {
    if(!env)
        return NULL;
    if(Has(Car(env), label))
        return NULL;
    Set(Car(env), label, thing);
    return thing;
}

static thing_th *safely_register_thing(thing_th *anon, thing_th *regMe) {
    thing_th *attachMe;
    if(!anon)
        return NULL;
    attachMe=Primordial_Cons(regMe, NULL, GC_SKIPREG);
    set_cdr(anon, attachMe);
    return attachMe;
}

static thing_th *reg_to_scope(thing_th *scope, thing_th *regMe) {
    if(!scope)
        return NULL;
    Set(Car(scope), ANON_TAIL_KW, 
        safely_register_thing(Get(Car(scope), ANON_TAIL_KW), regMe));
    return regMe;
}

thing_th *reg_thing(thing_th *thing) {
    return reg_to_scope(env, thing);
}

thing_th *reg_to_parent(thing_th *thing) {
    thing_th *tmpChild=env;
    thing_th *copyInParentScope;
    env=Cdr(env);
    copyInParentScope=duplicate(thing);
    env=tmpChild;
    return copyInParentScope;
}

void wipe_env(void) {
    while(del_env());
}
