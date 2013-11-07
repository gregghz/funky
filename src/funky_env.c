#include "funky.h"

static thing_th *resume_registering(thing_th *thing) {
    SKIP_REG=0;
    return thing;
}

static thing_th *Primordial_Grid(void) {
    SKIP_REG=1;
    return resume_registering(Grid());
}

static thing_th *Primordial_Cons(thing_th *car, thing_th *cdr) {
    SKIP_REG=1;
    return resume_registering(Cons(car, cdr));
}

static thing_th *spawn_env(thing_th *prevEnv, thing_th *scope) {
    if(th_kind(scope)!=grid_k)
        return env;
    return env=Primordial_Cons(scope, prevEnv);
}

unsigned int count_env_levels(void) {
    return env_levels;
}

thing_th *push_env(thing_th *newScope) {
    ++env_levels;
    return spawn_env(env, newScope);
}

int new_env(void) {
    return spawn_env(env, Primordial_Grid()) ? 0 : 1;
}

static int establish_bacros(thing_th *bacroGrid) {
    if(!bacroGrid) return 1;
    Set(bacroGrid, "~>", Atom("rapply"));
    Set(bacroGrid, "<~", Atom("apply"));
    Set(bacroGrid, "->", Atom("rcall"));
    Set(bacroGrid, "<-", Atom("call"));
    Set(bacroGrid, "<S", Atom("strict-apply"));
    Set(bacroGrid, "S>", Atom("strict-rapply"));
    Set(bacroGrid, ":", Atom("pair"));
    Set(bacroGrid, ".", Atom("get"));
    return 0;
}

int establish_root_environment(void) {
    SKIP_REG=0;
    env_levels=0;
    globalRegistry=Primordial_Grid();
    spawn_env(NULL, Primordial_Grid());
    rootEnvironment=Car(env);
    rootBacros=Grid();
    Set(rootEnvironment, "nil", NULL);
    Set(rootEnvironment, "true", Atom("true"));
    Set(rootEnvironment, "+", Routine(&dirty_sum));
    Set(rootEnvironment, "-", Routine(&dirty_sub));
    Set(rootEnvironment, "if", Method(&funky_if));
    Set(rootEnvironment, "&ver", String("Funky Lisp Draft 3"));
    Set(rootEnvironment, "set!", Routine(&funky_set));
    Set(rootEnvironment, "print", Routine(&funky_print));
    Set(rootEnvironment, "list", Routine(&funky_list));
    Set(rootEnvironment, "pair", Routine(&funky_pair));
    Set(rootEnvironment, "grid", Routine(&funky_grid));
    Set(rootEnvironment, "get", Routine(&funky_grid_get));
    Set(rootEnvironment, "quote", Method(&funky_quote));
    Set(rootEnvironment, "apply", Routine(&apply));
    Set(rootEnvironment, "mac", Method(&funky_macro));
    Set(rootEnvironment, "def", Method(&funky_def));
    Set(rootEnvironment, "head", Routine(&funky_head));
    Set(rootEnvironment, "&rest", Routine(&funky_rest));
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
   return Err(Cons(String("Unknown symbol"),
                   Cons(Atom(label), NULL)));
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

static int wipe_registry(grid_th *grid) {
    char **keywords=grid_keys_list(grid->data);
    char **kw=keywords;
    while(kw && *kw) {
        del_thing((thing_th *)get_grid_item(grid->data, *kw));
        ++kw;
    }
    wipe_keys_list(keywords);    
    del_grid((thing_th *)grid);
    return 0;
}

static int clean_registry(grid_th *grid) {
    char **keywords=grid_keys_list(grid->data);
    char **kw=keywords;
    while(kw && *kw) {
        thing_th *thing=(thing_th *)get_grid_item(grid->data, *kw);
        if(thing && thing->references<1)
            del_thing(thing);
        set_grid_item(grid->data, *kw, NULL);
        ++kw;
    } 
    wipe_keys_list(keywords);    
    del_grid((thing_th *)grid);
    return 0;
}

static int knock_reference_count(thing_th *cell) {
    return cell && cell->references && --cell->references;
}

static thing_th *inner_empty_address_for_kind(char **kw, kind_t kind) {
    while(kw && *kw) {
        thing_th *cur=Get(globalRegistry, *kw);
        if(cur->references==0 && cur->kind==kind) {
            ++cur->references;
            return cur;
        }
    }
    return NULL;
}

static thing_th *find_empty_address_for_kind(kind_t kind) {
    char **keywords=grid_keys_list(((grid_th *)globalRegistry)->data);
    thing_th *result=inner_empty_address_for_kind(keywords, kind);
    wipe_keys_list(keywords);    
    return result;
}

static thing_th *spawn_new_thing(kind_t kind) {
    fprintf(stderr, "This needs to be finished, Ishy!");
    switch(kind) {
        case error_k:
            return (thing_th *)calloc(1, sizeof(err_th));
        case atom_k:
            return (thing_th *)calloc(1, sizeof(atom_th));
        case number_k:
            return (thing_th *)calloc(1, sizeof(number_th));
        case string_k:
            return (thing_th *)calloc(1, sizeof(string_th));
        case cons_k:
            return (thing_th *)calloc(1, sizeof(cons_th));
        case grid_k:
            return (thing_th *)calloc(1, sizeof(grid_th));
        case routine_k:
            return (thing_th *)calloc(1, sizeof(routine_th));
        case method_k:
            return (thing_th *)calloc(1, sizeof(method_th));
        case procedure_k:
            return (thing_th *)calloc(1, sizeof(procedure_th));
        case macro_k:
            return (thing_th *)calloc(1, sizeof(macro_th));
        case gen_k:
            return (thing_th *)calloc(1, sizeof(gen_th));
        case null_k:
            return NULL;
    }
    return NULL;
}

thing_th *spawn_thing(kind_t kind) {
    thing_th *result=NULL;
    if(!SKIP_REG) 
        result=find_empty_address_for_kind(kind);
    if(result) 
        return result;
    return spawn_new_thing(kind);
}

static int pop_scope_grids(grid_th *grid) {
    char **keywords=grid_keys_list(grid->data);
    char **kw=keywords;
    while(kw && *kw) {
        knock_reference_count((thing_th *)get_grid_item(grid->data, *kw));
        ++kw;
    }
    wipe_keys_list(keywords);    
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
    --env_levels;
    return 1;
}

static int current_env_is_not_top_env() {
    return env && Cdr(env) && Car(Cdr(env))!=rootEnvironment;
}

int pop_env(void) {
    if(current_env_is_not_top_env())
        return del_env();
    clean_registry((grid_th *)globalRegistry);
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

static thing_th *regist_anonymous_thing(thing_th *registry, 
                                        thing_th *thing) {
    char *idstr;
    asprintf(&idstr, "%d", (int)thing);
    Set(registry, idstr, thing);
    erase_string(idstr);
    return thing;
}

thing_th *reg_thing(thing_th *thing) {
    if(SKIP_REG)
        return thing;
    return regist_anonymous_thing(globalRegistry, thing);
}

thing_th *recursive_reg_2_parent(thing_th *thing) {
    if(!thing)
        return NULL;
    if(Car(thing))
        recursive_reg_2_parent(Car(thing));
    if(Cdr(thing))
        recursive_reg_2_parent(Cdr(thing));
    thing->level--;
    return thing;
}

thing_th *reg_to_parent(thing_th *thing) {
    return recursive_reg_2_parent(thing);
}

void wipe_env(void) {
    while(del_env());
    wipe_registry((grid_th*)globalRegistry);
}
