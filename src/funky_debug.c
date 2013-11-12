#include "funky.h"

static thing_th *inner_depict(FILE *tgtStream, thing_th *thing);

static int depict_grid_elements(FILE *tgtStream, thing_th *src) {
    thing_th *keywords=Keys(src);
    thing_th *keys=keywords;
    fprintf(tgtStream, "{");
    while(keys) {
        fprintf(tgtStream, "\"%s\": ", 
               sym(Car(keys)));
        inner_depict(tgtStream, Get(src, sym(Car(keys))));
        keys=Cdr(keys);
        if(keys)
            fprintf(tgtStream, ",\n ");
    }
    fprintf(tgtStream, "}");
    return 0;
}

static int depict_routine(FILE *tgtStream, thing_th *rooteen) {
    fprintf(tgtStream, "<c_routine @%d@>", (int)rooteen);
    return 0;
}

static thing_th *depict_lambda(FILE *tgtStream, const char *lbl, thing_th *lambda) {
    thing_th *exprs=Cdr(lambda);
    fprintf(tgtStream, "%s ", lbl, (int)lambda);
    inner_depict(tgtStream, Car(lambda));
    while(exprs) {
        fprintf(tgtStream, " ");
        inner_depict(tgtStream, Car(exprs));
        exprs=Cdr(exprs);
    }
    return Cdr(Cdr(lambda));
}

static thing_th *inner_depict(FILE *tgtStream, thing_th *thing) {
    int withinList=thing && is_list(thing);
    thing_th *last=NULL;
    if(withinList)
        fprintf(tgtStream, "(");
    while(thing) {
        switch(th_kind(thing)) {
            case string_k:
                fprintf(tgtStream, "\"%s\"", ((string_th *)thing)->repr);
                break;
            case routine_k: case method_k:
                depict_routine(tgtStream, thing);
                break;
            case procedure_k:
                thing=depict_lambda(tgtStream, "def", thing);
                break;
            case macro_k: 
                thing=depict_lambda(tgtStream, "mac", thing);
                break;
            case gen_k: case cons_k: case error_k:
                inner_depict(tgtStream, Car(thing));
                break;
            case grid_k:
                depict_grid_elements(tgtStream, thing);
                break;
            case atom_k: case number_k:
            default:
                fprintf(tgtStream, "%s", sym(thing));
        }
        last=thing;
        thing=Cdr(thing);
        if(thing)
            fprintf(tgtStream, " ");
    }
    if(withinList)
        fprintf(tgtStream, ")");
    return last;
}

static thing_th *raw_depiction(FILE *targetStream, thing_th *thing) {
    inner_depict(targetStream, thing);
    fprintf(targetStream, "\n");
    return NULL;
}

thing_th *depict(thing_th *thing) {
    return raw_depiction(stdout, thing);
}

thing_th *depict_error(thing_th *errThing) {
    return raw_depiction(stderr, errThing);
}

const char *debug_lbl(const thing_th *thing) {
    switch(th_kind(thing)) {
        case null_k: return "nil"; 
        case error_k: return "error"; 
        case atom_k: return "atom"; 
        case number_k: return "number"; 
        case string_k: return "string"; 
        case cons_k: return "cons"; 
        case grid_k: return "grid"; 
        case method_k: return "method";
        case routine_k: return "routine"; 
        case procedure_k: return "procedure"; 
        case macro_k: return "macro"; 
        case gen_k: return "gen"; 
    }
    return "UNKNOWN!";
}

static int inner_dbglst(int depth, const thing_th *thing) {
    int i;
    for(i=0;i<depth;++i) 
        fprintf(stdout, " ");
    fprintf(stdout, "%s @%d@: %s\n", 
           debug_lbl(thing),
           (int)thing,
           sym(thing));
    if(Car(thing)) {
        for(i=0;i<depth;++i) fprintf(stdout, " ");
        inner_dbglst(depth+1, Car(thing));
    } 
    if(Cdr(thing))
        inner_dbglst(depth, Cdr(thing));
    return 0;
}

int debug_list(const thing_th *thing) {
    inner_dbglst(0, thing);
    return 0;
}
