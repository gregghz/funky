static int tinker_flat(void) {
    thing_th *expr=Cons(Atom("+"), 
                        Cons(Number("3"), 
                             Cons(Number("6"), 
                                  Cons(Atom("foo"), NULL))));
    thing_th *result;
    env_set("foo", Number("-2"));
    result=eval(expr);
    depict(result);
    depict(env);
    return 0;
}

static int third_tinker(void) {
    thing_th *grid=Grid();
    thing_th *keys;
    thing_th *nesty=Cons(Cons(Atom("foo"), 
                              Cons(Atom("bar"), 
                                   Cons(Atom("gig"), NULL))), 
                         Cons(Atom("bas"), 
                              Cons(grid, NULL)));
    Set(grid, "foo", Atom("Foo"));
    Set(grid, "bar", Atom("Bar"));
    depict(grid);
    depict(Get(grid, "foo"));
    depict(Get(grid, "bar"));
    depict(lookup_txt("&anon"));
    depict(lookup_txt("true"));

    keys=Vals(grid);
    depict(keys);

    new_env();
    Atom("poopy");
    depict(lookup_txt("&anon"));
    pop_env();

    depict(lookup_txt("&anon"));
    return 0;
}

static int nested_eval_tinker(void) {
    thing_th *expr=Cons(Atom("+"), 
                        Cons(Number("3"), 
                             Cons(Cons(Atom("+"),
                                       Cons(Number("3"),
                                            Cons(Number("4"), 
                                                 NULL))), 
                                  Cons(Atom("foo"), NULL))));
    thing_th *result;
    env_set("foo", Number("-2"));
    result=eval(expr);
    depict(result);
    depict(env);
    return 0;
}

static int procedure_eval_tinker(void) {
    thing_th *func=Proc(Cons(Atom("x"),
                             Cons(Atom("y"), NULL)),
                        Cons(Atom("+"),
                             Cons(Atom("x"), 
                                  Cons(Atom("y"), NULL))));
    thing_th *funcCall=Cons(Atom("foo"),
                            Cons(Number("3"),
                                 Cons(Number("4"), NULL)));
    thing_th *result;
    env_set("foo", func);
    result=eval(funcCall);
    depict(result);
    return 0;
}

static int macro_tinker(void) {
    thing_th *macro=Mac(Cons(Atom("x"), 
                             Cons(Atom("y"), NULL)),
                        Cons(Atom("+"),
                             Cons(Atom("x"),
                                  Cons(Atom("y"), NULL))));
    thing_th *expr=Cons(Atom("foo"),
                        Cons(Atom("3"),
                             Cons(Atom("4"), NULL)));
    env_set("foo", macro);
    depict(eval(expr));
    return 0;
}

