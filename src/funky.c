#include "funky.h"
#include <unistd.h>

static int file_exists(const char *fpath) {
    return strlen(fpath)>0 && access(fpath, F_OK)==0;
}

static thing_th *do_nothing(thing_th *expr) {
    return NULL;
}

static int bad_source_file(const char *fpath) {
    fprintf(stderr, "Could not load funk file: %s\n", fpath);
    return 1;
}

static int evaluate_then_process(thing_th *exprs,
                                 c_routine post_processor) {
    while(exprs) {
        post_processor(eval(Car(exprs)));
        exprs=Cdr(exprs);
    }
    return 0;
}

static int finalize_file_consumption(thing_th *output, FILE *src, c_routine pproc) {
    fclose(src);
    return evaluate_then_process(output, pproc);
}

static int consume_file_contents(FILE *src, c_routine pproc) {
    if(src) 
        return finalize_file_consumption(read_exprs(src), src, pproc);
    fprintf(stderr, "Could not open source stream.\n");
    return 1;
}

static int consume_file(const char *filePath, c_routine pproc) {
    if(!file_exists(filePath))
        return bad_source_file(filePath);
    return consume_file_contents(fopen(filePath, "rt"), pproc);
}

static int desugar_cons_tree(thing_th *results) {
    while(results) {
        depict(Car(results));
        results=Cdr(results);
    }
    return 0;
}

static int desugar_stream(FILE *src) {
    return desugar_cons_tree(read_exprs(src));
}

static int desugar_stdin(void) {
    return desugar_stream(stdin);
}

static int process_arguments(int max, char **argv) {
    int batchMode=0;
    int silentMode=0;
    int argc;
    for(argc=0;argc<max;++argc) {
        if(streq(argv[argc], "-b"))
            batchMode=1;
        else if(streq(argv[argc], "-s"))
            silentMode=1;
        else if(streq(argv[argc], "-d"))
            return desugar_stdin();
        else if(streq(argv[argc], "-B") && (max-argc)>2) {
            Set(rootBacros, argv[argc+1], Atom(argv[argc+2]));
            argc+=2;
        }
        else if(file_exists(argv[argc]))
            if(consume_file(argv[argc], do_nothing)!=0)
                return 1;
    }
    if(batchMode)
        return 0;
    return consume_file_contents(stdin, silentMode ? do_nothing : depict);
}

static int finish(int status) {
    wipe_env();
    fclose(stdout);
    fclose(stdin);
    fclose(stderr);
    return status;
}

int main (int argc, char **argv) {
    establish_root_environment();
    return finish(process_arguments(argc-1, argv+1));
}
