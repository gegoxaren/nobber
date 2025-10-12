#include <stdlib.h>
#include <stddef.h>
#include <stdio.h>
#include <inttypes.h>
#include <stdint.h>
#define NOB_IMPLEMENTATION
#include "../nob.h"


typedef struct {
    int32_t something;
    char * something_else;
} Foo;

typedef struct {
    int32_t * items;
    size_t count;
    size_t capacity;
} DaInt;

typedef struct {
    Foo ** items;
    size_t count;
    size_t capacity;
} DaFoo;

int main ([[maybe_unused]] int argc,[[maybe_unused]] char * argv[]) {
//    DaFoo * foo_list = malloc (sizeof (DaFoo));
//    for (int i = 0; i <= 1000; i++) {
//        Foo * f = malloc (sizeof (Foo));
//        f->something = i;
//        f->something_else = malloc (4);
//        sprintf (f->something_else, "%i", i);
//        nob_da_append (foo_list, f);
//    }
//
//    nob_da_foreach (Foo *, it, foo_list) {
//        fprintf (stdout, "something: %i, somethig_else: %s\n", (*it)->something, (*it)->something_else);
//    }


    DaInt * int_list = malloc (sizeof (DaInt));
    for (int32_t i = 0; i <= 1000; i++) {
        nob_da_append (int_list, i);
    }

    nob_da_foreach (int32_t , it, int_list) {
        auto i = *it;
        fprintf (stdout,"%"PRIi32"\n", i);
    }
}


