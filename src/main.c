#include <stdlib.h>
#include <stddef.h>
#include <stdio.h>
#include <inttypes.h>
#include <stdint.h>
#define NOB_IMPLEMENTATION
#include "../nob.h"
#undef NOB_IMPLEMENTATION

#define NOBBER_UTILS_IMPLEMENTAITON 
#include "../utils.h"
#undef NOBBER_UTILS_IMPLEMENTAITON

#define NOBBER_HASH_SET_IMPREMENTATION
#include "../hashset.h"
#undef NOBBER_HASH_SET_IMPREMENTATION


int main ([[maybe_unused]] int argc,[[maybe_unused]] char * argv[]) {
    char * exec = get_path_of_current_executable ();
    fprintf (stdout, "exec path: %s\n", exec);
    fprintf (stdout, "WCD: %s\n", nobber_get_cwd ());

    size_t out_len = 0;
    char ** parts = split_string (exec, PATH_SEPARATOR, &out_len);
    
    for (size_t i = 0; i < out_len; i++) {
        fprintf (stdout, "%s\n", parts[i]);
    }

    return 0;
}


