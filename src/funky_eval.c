#include "funky.h"

static thing_th *evoke_c_level_function(c_routine func, 
                                        thing_th *args) {
    return func(args);
}

static thing_th *call_c_level_code(thing_th *methodContainer, 
                                   thing_th *args) {
    return evoke_c_level_function(call_rt(methodContainer), args);
}

static thing_th *finalize_ugly_apply(thing_th *expr) {
    pop_env();
    return expr;
}

static thing_th *evaluate_each_subexpr(thing_th *exprs) {
    thing_th *result;
    while(exprs) {
        result=eval(Car(exprs));
        exprs=Cdr(exprs);
    }
    return result;
}

static thing_th *pre_skid(thing_th *atom) {
    thing_th *newThing;
    char *newName;
    asprintf(&newName, "_%s", sym(atom));
    newThing=Atom(newName);
    erase_string(newName);
    return newThing;
}

static thing_th *curry_this_expr(thing_th *lambda,
                                 thing_th *ops,
                                 thing_th *unprovided) {
    thing_th *provided;
    thing_th *head;
    if(ops==unprovided)
        return lambda;
    provided=Cons(eval(Car(ops)), NULL);
    head=provided;
    ops=Cdr(ops);
    while(ops!=unprovided) {
        provided=set_cdr(provided, Cons(eval(Car(ops)), NULL));
        ops=Cdr(ops);
    }
    set_cdr(provided, unprovided);
    thing_th *arggyments=Cons(Cons(Atom("list"), head), NULL);
    thing_th *fooby=Cons(Atom("apply"), Cons(lambda, arggyments));
    return reg_to_parent(Proc(unprovided, Cons(fooby, NULL)));
}

static thing_th *ugly_apply(thing_th *lambda,
                            thing_th *args,
                            thing_th *ops,
                            thing_th *exprBody) {
    thing_th *operands=ops;
    while(operands && args) {
        env_set(sym(Car(operands)), Car(args));
        args=Cdr(args);
        operands=Cdr(operands);
    }
    if(args)
        env_set("&args", duplicate(args));
    if(operands)
        return curry_this_expr(lambda, ops, operands);
    return reg_to_parent(evaluate_each_subexpr(exprBody));
}

static thing_th *horrendously_ugly_apply(thing_th *lambda, 
                                         thing_th *args) {
    new_env();
    return finalize_ugly_apply(ugly_apply(lambda, 
                                          args, 
                                          duplicate(Car(lambda)),
                                          duplicate(Cdr(lambda))));
}


static thing_th *reconstitute_args_from_grid(thing_th *ops,
                                             thing_th *grid) {
    thing_th *arguments;
    thing_th *args;
    arguments=Cons(Get(grid, sym(Car(ops))), NULL);
    args=arguments;
    while((ops=Cdr(ops)))
        args=set_cdr(args, Cons(Get(grid, sym(Car(ops))), NULL));
    return arguments;
}

static thing_th *apply_args_over_lambda(thing_th *lambda, 
                                        thing_th *args) {
    if(th_kind(args)==grid_k)
        args=reconstitute_args_from_grid(Car(lambda), args);
    switch(th_kind(lambda)) {
        case routine_k: case method_k: 
            return call_c_level_code(lambda, args);
        case procedure_k: case macro_k: 
            return horrendously_ugly_apply(lambda, args);
        default:
            return NULL;
    }
}

thing_th *apply(thing_th *lambdaAndArgs) {
    return apply_args_over_lambda(Car(lambdaAndArgs),
                                  Car(Cdr(lambdaAndArgs)));
}

static thing_th *recursive_lambda_not_functor(thing_th *label,
                                              thing_th *head,
                                              thing_th *arguments) {
    thing_th *args=Cons(eval(Car(arguments)), NULL);
    thing_th *arg=args;
    arguments=Cdr(arguments);
    while(arguments) {
        arg=set_cdr(arg, Cons(eval(Car(arguments)), NULL));
        arguments=Cdr(arguments);
    }
    if(th_kind(head)==procedure_k)
        return apply_args_over_lambda(head, args);
    if(th_kind(head)==routine_k)
        return call_rt(head)(args);
    fprintf(stderr, "Trying to call a non-procedure %s: %s:%s\n", debug_lbl(head), sym(label), sym(head));
    return NULL;
}

static thing_th *recursive_lambda_expr(thing_th *label, 
                                       thing_th *head, 
                                       thing_th *arguments) {
    switch(th_kind(head)) {
        case method_k: 
            return call_c_level_code(head, arguments);
        case macro_k: 
            return apply_args_over_lambda(head, arguments);
        case procedure_k: case routine_k: 
            return recursive_lambda_not_functor(label, head, arguments);
        case atom_k:
            return recursive_lambda_expr(head, lookup_sym(head), arguments);
        default: 
            fprintf(stderr,
                    "%s is not of type <lambda> but <%s>\n",
                    sym(label), 
                    debug_lbl(head));
            return NULL;
    }
}

static thing_th *default_symbol_lookup(thing_th *symbol, thing_th *represented) {
    if(represented==unknownSymbolError)
        return symbol;
    return represented;
}

static thing_th *recursive_eval(thing_th *expr) {
    switch(th_kind(expr)) {
        case cons_k:
            if(Car(expr))
                return recursive_lambda_expr(Car(expr), eval(Car(expr)), Cdr(expr));
            return expr;
        case number_k: case string_k:
        case null_k: case routine_k:
        case method_k: case macro_k:
        case error_k: case gen_k:
            return expr;
        default:
            return default_symbol_lookup(expr, lookup_sym(expr));
    }
}

thing_th *eval(thing_th *expr) {
    if(!expr)
        return NULL;
    return recursive_eval(expr);
}
