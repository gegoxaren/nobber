#include <alloca.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>
#include <inttypes.h>
#define NOB_IMPLEMENTATION
#define NOB_STRIP_PREFIX
#include "nob.h"

typedef struct da_strings_t DaStrings;

void * malloc0 (size_t size);
char * strip_extention (char * file_name);
char * c_to_o_ext (char * str);
DaStrings * compile_files (char * source_files[], size_t len, Nob_Cmd * cmd, Nob_Procs * procs);
void link_files (DaStrings * object_files,char * exec_name, Nob_Cmd * cmd);
void clean_files (char * source_files[], size_t len, Nob_Cmd * cmd);

#define EXEC_NAME "nobber"

const char * default_build_flags[] = {
    "-std=gnu23",
    "-Wall",
    "-Wextra",
    "-mtune=native",
    "-g",
    NULL,
};
#define default_build_flags_count (NOB_ARRAY_LEN(default_build_flags) - 1)

const char * default_linking_flags[] = {
    "-lc",
    NULL,
};
#define default_linking_flags_count (NOB_ARRAY_LEN(default_linking_flags) - 1)

const char * src_files[] = {
    "src/main.c",
    "src/foo.c",
    NULL,
};
#define src_file_count (NOB_ARRAY_LEN(src_files) - 1)

//#define free0(ptr) \
//    free (ptr); \
//    ptr = NULL;
#define free0(ptr) \
    memset ((void *)ptr, 0, sizeof ((void *) ptr)); \
    free (ptr);

struct da_strings_t {
    char ** items;
    size_t count;
    size_t capacity;
};

/** 
 * free the items in a list propperly!
 * TODO: Need a version that takes a FreeFunc.
 **/
#define da_free_items(Type, da) { \
    for (Type *it = (da)->items; it < (da)->items + (da)->capacity; it++) { \
        if (*it == NULL) break;\
        free (*it);\
    }\
    free ((da)->items);\
}

int main (int argc, char ** argv) {

    Nob_Procs procs = {0};
    Nob_Cmd cmd = {0};

    NOB_GO_REBUILD_URSELF (argc, argv);
    [[maybe_unused]]  const char * program_name = nob_shift_args(&argc, &argv);
    const char * command_name = "build";
    if (argc > 0) command_name = nob_shift_args(&argc, &argv);

    if ((strcmp (command_name, "build")) == 0) {
        DaStrings * objects = compile_files ((char **)src_files, src_file_count, &cmd, &procs);
        nob_procs_wait_and_reset (&procs);
        link_files (objects, EXEC_NAME, &cmd);
        da_free_items (char *, objects);
        free (objects);
    } else if (strcmp (command_name, "clean") == 0) {
        clean_files ((char**)src_files, src_file_count, &cmd);
    } else {
        nob_log (NOB_INFO, "Unknown command %s\n", command_name);
    }
    if (cmd.capacity) nob_cmd_free (cmd);

    return 0;
}

/* ----------------- */

void * malloc0 (size_t size) {
    void * out_val = malloc (size);
    memset (out_val, 0, size);
    return out_val;
}


// https://stackoverflow.com/questions/43163677/how-do-i-strip-a-file-extension-from-a-string-in-c
char * strip_extention (char * file_name) {
    char * out_str = malloc(strlen(file_name) + 1);
    strcpy (out_str, file_name);
    char *end = out_str + strlen(out_str);

    while (end > out_str && *end != '.' && *end != '\\' && *end != '/') {
        --end;
    }

    if ((end > out_str && *end == '.') &&
            (*(end - 1) != '\\' && *(end - 1) != '/')) {
        *end = '\0';
    }

    return out_str;
}

char * c_to_o_ext (char * str) {
    char * out_str = malloc (strlen(str) + 1);
    char * tmp_str = strip_extention (str);
    sprintf (out_str, "%s.o", tmp_str); 
    free (tmp_str);
    return out_str;
}

DaStrings * compile_files (char * source_files[], size_t len, Nob_Cmd * cmd, Nob_Procs * procs) {
    DaStrings * object_files = malloc0 (sizeof (DaStrings));
    for (size_t i = 0; i < len; i++) {
        cmd->count = 0;
        if (source_files[i] == NULL) break;
        nob_cmd_append (cmd, "cc");
        nob_cmd_append (cmd, source_files[i]);
        nob_da_append_many (cmd, default_build_flags, default_build_flags_count);
        nob_cmd_append (cmd, "-c");
        char * tmp_str = c_to_o_ext (source_files[i]);
        nob_cmd_append (cmd, "-o", tmp_str);

        char * s = malloc0 (strlen (tmp_str) + 1);
        strcpy (s, tmp_str);
        nob_da_append (object_files, s);

        nob_cmd_run (cmd, .async = procs);
        free0 (tmp_str);
    }
    return object_files;
}

void link_files (DaStrings * object_files,char * exec_name, Nob_Cmd * cmd) {
    nob_cmd_append (cmd, "cc");
    nob_cmd_append (cmd, "-o", exec_name);
    nob_da_append_many (cmd, (char**)object_files->items, object_files->count);
    nob_da_append_many (cmd, (char**) default_linking_flags, default_linking_flags_count);
    nob_cmd_run (cmd);
}

void clean_files (char * source_files[], size_t len, Nob_Cmd * cmd) {
    cmd->count = 0;
    DaStrings tmp_strings = {0};
    for (size_t i = 0; i < len; i++) {
        if (source_files[i] == NULL) break;
        char * tmp_file = strip_extention ((char *)source_files[i]);
        char * tmp_obj_file = c_to_o_ext (tmp_file);
        nob_da_append (&tmp_strings, tmp_obj_file);
        tmp_file = c_to_o_ext (tmp_file);
        free0 (tmp_file);
    }
    nob_cmd_append (cmd, "rm");
    nob_da_append_many (cmd, tmp_strings.items, tmp_strings.count);
    for (size_t i = 0; i < tmp_strings.count; i++) {
        char * s = tmp_strings.items[i];
        if (s == NULL) break;
    }
    nob_cmd_append (cmd, EXEC_NAME);
    nob_cmd_run (cmd);
    da_free_items (char *, &tmp_strings);
}

