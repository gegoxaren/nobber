
#include <stdlib.h>
#include <stdio.h>
#include <limits.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>
#include <inttypes.h>
#include <unistd.h>

#define NOB_IMPLEMENTATION
#define NOB_STRIP_PREFIX
#include "nob.h"
#undef NOB_IMPLEMENTATION
#define NOBBER_HASH_SET_IMPREMENTATION
#include "hashset.h"
#define NOBBER_UTILS_IMPLEMENTAITON
#include "utils.h"

#define EXEC_NAME "main"
#define BUILD_DIRECTORY "build"

char * default_build_flags[] = {
    "-std=gnu23",
    "-Wall",
    "-Wextra",
    "-mtune=native",
    "-g",
    //"-I,thirdparty/raylib/include",
    NULL,
};
#define default_build_flags_count (NOB_ARRAY_LEN(default_build_flags) - 1)

char * default_linking_flags[] = {
    "-lc",
    //"-lraylib",
    //"-Wl,-rpath,thirdparty/raylib/lib",
    //"-Lthirdparty/raylib/lib",
    NULL,
};
#define default_linking_flags_count (NOB_ARRAY_LEN(default_linking_flags) - 1)

char * src_files[] = {
    "src/main.c",
    "src/foo.c",
    NULL,
};
#define src_file_count (NOB_ARRAY_LEN(src_files) - 1)

char * ld_paths[] = {
  //"thirdparty/raylib/lib",
  NULL
};
#define ld_paths_count (NOB_ARRAY_LEN (ld_paths) - 1)

int main (int argc, char ** argv) {

    for (size_t i = 0; i < ld_paths_count; i++) {
        append_ld_path(ld_paths[i]);
    }

    Nob_Procs procs = {0};
    Nob_Cmd cmd = {0};
    size_t build_flags_len = default_linking_flags_count;
    size_t build_flags_count = default_linking_flags_count;
    size_t files_count = src_file_count;
    NobberBuildContext ctx = {
        .build_dir = BUILD_DIRECTORY,
        .build_flags = default_build_flags, .build_flags_len = build_flags_count,
        .source_files = src_files, .source_files_len = files_count};


    NOB_GO_REBUILD_URSELF_PLUS (argc, argv, "utils.h");

    [[maybe_unused]]  const char * program_name = nob_shift_args(&argc, &argv);
    const char * command_name = "build";
    if (argc > 0) command_name = nob_shift_args(&argc, &argv);

    if ((strcmp (command_name, "build")) == 0) {
        nob_mkdir_if_not_exists (BUILD_DIRECTORY);
        DaStrings * objects = compile_files (&ctx, &cmd, &procs);
        link_files (objects, EXEC_NAME, (char**) default_linking_flags, default_linking_flags_count, &cmd);
        da_free_items (char *, objects);
        free (objects);
    } else if (strcmp (command_name, "clean") == 0) {
        clean_files (&ctx, EXEC_NAME, &cmd);
    } else if (strcmp(command_name, "run") == 0) {
        nob_cmd_append (&cmd, "./"EXEC_NAME );
        nob_cmd_run (&cmd);
    } else {
        nob_log (NOB_ERROR, " Unknown command %s\n", command_name);
        return NOBBER_RETURN_UNKNOWN_COMMAND;
    }
    if (cmd.capacity) nob_cmd_free (cmd);

    return 0;
}

