
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

#define NOBBER_UTILS_IMPLEMENTAITON
#include "utils.h"
#define NOB_IMPLEMENTATION
#define NOB_STRIP_PREFIX
#include "nob.h"


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

int main (int argc, char ** argv) {

    Nob_Procs procs = {0};
    Nob_Cmd cmd = {0};

    NOB_GO_REBUILD_URSELF (argc, argv);

    [[maybe_unused]]  const char * program_name = nob_shift_args(&argc, &argv);
    const char * command_name = "build";
    if (argc > 0) command_name = nob_shift_args(&argc, &argv);

    if ((strcmp (command_name, "build")) == 0) {
        DaStrings * objects = compile_files ((char**) src_files, src_file_count,
                                             (char**) default_build_flags, default_build_flags_count,
                                             &cmd, &procs);
        nob_procs_wait_and_reset (&procs);
        link_files (objects, EXEC_NAME, (char**) default_linking_flags, default_linking_flags_count, &cmd);
        da_free_items (char *, objects);
        free (objects);
    } else if (strcmp (command_name, "clean") == 0) {
        clean_files ((char**)src_files, src_file_count, EXEC_NAME, &cmd);
    } else if (strcmp(command_name, "run") == 0) {
        nob_cmd_append (&cmd, "exec", );
        nob_cmd_run (&cmd);
    } else {
        nob_log (NOB_INFO, "[ERROR] Unknown command %s\n", command_name);
        return NOBBER_RETURN_UNKNOWN_COMMAND;
    }
    if (cmd.capacity) nob_cmd_free (cmd);

    return 0;
}

