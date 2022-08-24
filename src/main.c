#include "minic.h"

#include <stdio.h>
#include <string.h>

typedef struct {
    const char* input;
} TranslationUnit;

static const char* s_exename;
static const char* s_includepath;

static Array* process_args(int argc, const char** argv)
{
    s_exename = argv[0];
    const char* p = strrchr(s_exename, '/');
    if (p) {
        s_exename = p + 1;
    } else {
        p = strrchr(s_exename, '\\');
        if (p) {
            s_exename = p + 1;
        }
    }

    Array* files = array_new(sizeof(TranslationUnit), 4);

    bool hasError = false;
    for (int i = 1; i < argc;) {
        const char* arg = argv[i];

        if (stricmp(arg, "-I") == 0) {
            if (i + 1 >= argc) {
                hasError = true;
                break;
            }

            s_includepath = argv[++i];
            ++i;
            continue;
        }

        TranslationUnit unit;
        unit.input = arg;
        array_push_back(TranslationUnit, files, unit);
        ++i;

        //     strncpy(s_output, argv[++i], MAX_OSPATH);
        //     ++i;
        //     continue;
        // }

        // if (stricmp(arg, "-s") == 0) {
        //     if (i + 1 >= argc) {
        //         hasError = true;
        //         break;
        //     }

        //     s_input = argv[++i];
        //     ++i;
        //     continue;
        // }

        // hasError = true;
        // break;
    }

    if (hasError) {
        error("%s: invalid command line\n", s_exename);
    }

    return files;
}

static void compile_one(const char* input)
{
    Array* rawToks = lex(input);
    List* toks = preproc(rawToks, s_includepath);
    Obj* prog = parse(toks);

    char* slash = strrchr(input, '/');
    char* bslash = strrchr(input, '\\');
    const char* p = MAX(slash, bslash);
    p = p ? p + 1 : input;
    char output[MAX_OSPATH];
    strncpy(output, p, MAX_OSPATH);

    char* ext = strrchr(output, '.');
    assert(ext);
    ext[1] = 's';
    ext[2] = 0;
    gen(prog, input, output);
}

int main(int argc, const char** argv)
{
    process_args(argc, argv);

    Array* files = process_args(argc, argv);
    if (files->len == 0) {
        error("%s: no input files\n", s_exename);
    }

    s_includepath = s_includepath ? s_includepath : "";

    for (int i = 0; i < files->len; ++i) {
        TranslationUnit* unit = array_at(TranslationUnit, files, i);
        compile_one(unit->input);
    }

    return 0;
}
