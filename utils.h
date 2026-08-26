#pragma once

/*******
 * utils.h
 * These utility functions and macros can requiers nob.h
 *
 * You may use them as you see fit.
 *
 * This header file uses the standard STB-style inclue stuff:
 * define NOBBER_UTILS_IMPREMENTATION 
 * to in one file, so they get built.
 ******/

#include <stdlib.h>
#include <stdio.h>

#include <errno.h>
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
#include <assert.h>
#if defined(__unix__) || defined(__linux__)
    #include <unistd.h>
#endif
#ifdef __darwin__
    #include <mach-o/dyld.h>
#endif
#if defined(WINVER) || defined(__WINDOWS__) || defined(_WIN32) || defined(__MSYS__) || defined(__MINGW__) || defined(_MSC_VER)
    #include <windows.h>
    #include <direct.h>
#endif

#include "nob.h"

#include "hashset.h"
//#include "ht.h"

#define IN /*Does nothing, just a mark.*/
#define OUT /*Does nothing, just a mark.*/
#define INOUT /*Does nothing, just a mark.*/

#if defined(WIN32) || defined(_WIN32) || defined(__MSYS__) || defined(_MINGW__)
#define PATH_SEPARATOR_CHAR '\\' 
#define PATH_SEPARATOR "\\" 
#else 
#define PATH_SEPARATOR_CHAR '/' 
#define PATH_SEPARATOR "/" 
#endif 




typedef struct da_strings_t {
    char ** items;
    size_t count;
    size_t capacity;
} DaStrings;

typedef struct nobber_build_context_t {
    char ** source_files;
    size_t source_files_len;
    char ** build_flags;
    size_t build_flags_len;
    char * build_dir;
} NobberBuildContext;

typedef enum NobberError {
    NOBBER_RETURN_NORMAL,
    NOBBER_RETURN_UNUSED0,
    NOBBER_RETURN_UNUSED1,
    NOBBER_RETURN_UNUSED2,
    NOBBER_RETURN_UNUSED3,
    NOBBER_RETURN_UNKNOWN_COMMAND,
    NOBBER_RETURN_EXEC,
    NOBBER_RETURN_COUNT
} NobberError;

void * malloc0 (size_t size);
char * strip_extention (IN char * file_name);
char * c_to_o_ext (IN char * str);
DaStrings * compile_files (IN NobberBuildContext * ctx, INOUT Nob_Cmd * cmd, INOUT Nob_Procs * procs);
void link_files (IN DaStrings * object_files, IN char * exec_name, IN char * link_flags[], size_t link_flags_len, INOUT Nob_Cmd * cmd);
void clean_files (IN NobberBuildContext * ctx, char * exec_name, INOUT Nob_Cmd * cmd);
char * construct_string (IN size_t count, ...);
void append_ld_path (char * s);
NobberError run_excutable (INOUT Nob_Cmd * cmd, IN char * executable);
char * get_path_of_current_executable ();
char ** split_string (IN char * str, IN char * delimeter, OUT size_t * out_len);
char * nobber_strndup (IN const char * str, size_t len);
char * nobber_strrstr (IN char * haystack, IN char* needle);
char * substr (IN char * str, ssize_t pos, ssize_t len);
char * get_parent_dir (IN char * path, OUT size_t * out_len);

#if defined(_WIN32) || defined(__MINGW__) || defined(__MSYS2__)
#define strndup nobber_strndup
#endif 

#define free0(ptr) \
    if (ptr) {\
      memset ((void *)ptr, 0, sizeof (ptr)); \
      free (ptr);\
    }


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

#ifdef NOBBER_UTILS_IMPLEMENTAITON
#undef NOBBER_UTILS_IMPLEMENTAITON


/* ---------------- */

void * malloc0 (size_t size) {
    void * out_val = malloc (size);
    memset (out_val, 0, size);
    return out_val;
}

char * nobber_get_cwd () {
    char * buf = malloc0(PATH_MAX);
#if defined(_WIN32) || defined(__MSYS__) || defined(__MINGW__)
    if (!_getcwd(buf, PATH_MAX)) {
#else
    if (!getcwd(buf, PATH_MAX)) {
#endif
        nob_log (NOB_ERROR, "Could not read current working directory: %s", strerror (errno));
    }
    buf = NOB_REALLOC (buf, strlen (buf));
    return buf;
}


// https://stackoverflow.com/questions/43163677/how-do-i-strip-a-file-extension-from-a-string-in-c
char * strip_extention (IN char * file_name) {
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

char * c_to_o_ext (IN char * str) {
    char * out_str = malloc (strlen(str) + 1);
    char * tmp_str = strip_extention (str);
    sprintf (out_str, "%s.o", tmp_str); 
    free (tmp_str);
    return out_str;
}

bool foreach_directory_create (void * directory, void * user_data) {
    (void) user_data;
    mkdir_if_not_exists (directory);
}

DaStrings * compile_files (IN NobberBuildContext * ctx, INOUT Nob_Cmd * cmd, INOUT Nob_Procs * procs) {

    char ** source_files = ctx->source_files;
    size_t source_files_len = ctx->source_files_len;
    char ** build_flags = ctx->build_flags;
    size_t build_flags_len = ctx->build_flags_len;
    char * build_dir = ctx->build_dir;

    char * tmp_obj_file;
    char * tmp_str;
    NobberHashSet * directories = nobber_hash_set_new ((NobberHashFunc) str_hash, (NobberEqualFunc) strcmp, 0);

    // Create list of object files to be built.
    DaStrings * object_files = malloc0 (sizeof (DaStrings));
    for (size_t i = 0; i < source_files_len; i++) {
        if (source_files[i] == NULL) break;
        char * file_name = source_files[i];
        if (!nob_file_exists (file_name)) {
            nob_log (NOB_ERROR, "File \"%s\" does not exist! Aborting.");
            goto err_out;

        }
        tmp_obj_file = c_to_o_ext (file_name);
        tmp_str = malloc0 (strlen (file_name));
        sprintf (tmp_str, "%s" PATH_SEPARATOR "%s",build_dir, tmp_obj_file);

        char * s = malloc0 (strlen (tmp_str) + 1);
        strcpy (s, tmp_str);
        nob_da_append (object_files, s);

        // add directories to the hash_set.
        char * last_delim = strrchr (tmp_str, PATH_SEPARATOR_CHAR);
        intptr_t last_index = last_delim - tmp_str;

        char dir[PATH_MAX] = {0};
        (void) strncpy (dir, tmp_str, last_index);

        nobber_hash_set_add (directories, dir);

    }
    // create appropreate directories
    nobber_hash_set_foreach (directories, foreach_directory_create, NULL);

    cmd->count = 0;

    for (size_t i = 0; i < source_files_len; i++) {
        nob_cmd_append (cmd, "gcc");
        nob_da_append_many (cmd, build_flags, build_flags_len);
        nob_cmd_append (cmd, "-c", "-o", (char*) object_files->items[i], source_files[i]);
        nob_cmd_run (cmd, .async = procs);
    }

    nob_procs_wait (*procs);

    return object_files;

err_out:
    free0 (tmp_str);
    free0 (tmp_obj_file);
    return NULL;
}

void link_files (IN DaStrings * object_files, IN char * exec_name,
        IN char * link_flags[], size_t link_flags_len,
        INOUT Nob_Cmd * cmd) {
    nob_cmd_append (cmd, "cc");
    nob_cmd_append (cmd, "-o", exec_name);
    nob_da_append_many (cmd, (char**) object_files->items, object_files->count);
    nob_da_append_many (cmd, (char**) link_flags, link_flags_len);
    nob_cmd_run (cmd);
}

void clean_files (IN NobberBuildContext * ctx, char * exec_name, INOUT Nob_Cmd * cmd) {
    cmd->count = 0;

    char ** source_files = ctx->source_files;
    char * build_dir = ctx->build_dir;
    size_t len = ctx->source_files_len;

    DaStrings tmp_strings = {0};
    for (size_t i = 0; i < len; i++) {
        if (source_files[i] == NULL) break;
        char * tmp_obj_file = c_to_o_ext (source_files[i]);
        char tmp_with_path[PATH_MAX] = {0};
        sprintf (tmp_with_path, "%s" PATH_SEPARATOR "%s", build_dir, tmp_obj_file);
        char * tmp_with_path_2 = malloc0 (strlen (tmp_with_path));
        strcpy (tmp_with_path_2, tmp_with_path);
        nob_da_append (&tmp_strings, tmp_with_path_2);
    }
    nob_da_foreach(char *, it, &tmp_strings) {
        nob_cmd_append (cmd, "rm", *it);
        nob_cmd_run (cmd);
        cmd->count = 0;
    }
    nob_cmd_append (cmd, "rm", exec_name);
    nob_cmd_run (cmd);
    cmd->count = 0;
    da_free_items (char *, &tmp_strings);
}


void append_ld_path (char * s) {
  static Nob_String_Builder sb = {0};
  char * current_env = getenv ("LD_LIBRARY_PATH");
  if (current_env == NULL) {
    nob_sb_appendf(&sb, "LD_LIBRARY_PATH=%s",s);
  } else {
    nob_sb_appendf(&sb, "LD_LIBRARY_PATH=%s:%s", current_env ,s);
  }
  
  putenv (sb.items);
}

char * construct_string (size_t count, ...) {
    char * retval = NULL;
    va_list list = {0};
    Nob_String_Builder sb = {0};
    va_start (list, count);
    for (size_t i = 0; i <= count; i++) {
        const char * s = va_arg (list, char *);
        nob_sb_append_cstr(&sb, s);
    }
    va_end (list);
    Nob_String_View sv = nob_sv_from_parts (sb.items, sb.count);
    retval = malloc0 (strlen (sv.data) + 1);
    strcpy (retval, sv.data);
    return retval;
}


char * get_path_of_current_executable () {
    char buf[PATH_MAX];
#if  defined(__DragonFly__) || defined(__NetBSD__) || defined(__linux__)
    #if defined(__linux__)
      #define PROC_EXE_PATH "/proc/self/exe"
    #elif defined(__NetBSD__)
      #define PROC_EXE_PATH "/proc/curproc/exe"
    #elif defined(__DragonFly__)
      #define PROC_EXE_PATH "/proc/curproc/file"
    #endif
    if (readlink (PROC_EXE_PATH, buf, PATH_MAX) < 0) {
        nob_log (NOB_ERROR, "Could not readlink /proc/self/exe: %s", strerror (errno));
        return NULL;
    }
#elif defined(__FreeBSD__)
    int mib[4];
    mib[0] = CTL_KERN;
    mib[1] = KERN_PROC;
    mib[2] = KERN_PROC_PATHNAME;
    mib[3] = -1;
    sysctl(mib, 4, buf, PATH_MAX, NULL, 0);
#elif defined(__darwin__)
    if (_NSGetExecutablePath(buf, PATH_MAX) < 0) {
        nob_log (NOB_ERROR, "Could not get executable path.");
    }
#elif defined(WINVER) || defined(__WINDOWS__) || defined(_WIN32) || defined(__MSYS__) || defined(__MINGW__) || defined(_MSC_VER)
    if (!GetModuleFileNameA (NULL, buf, MAX_PATH)) {
        nob_log (NOB_ERROR, "Could not get executable path: %s", strerror (errno));
        return NULL;
    }
#else
    static_assert (false, "get_path_of_current_executable () is not implemented for this platform.");
#endif
    errno = 0;
    char * ret = malloc (strlen (buf));
    if (ret == NULL) {
        nob_log (NOB_ERROR, "Could not allocate memory: %s", strerror (errno));
        return NULL;
    }
    strcpy(ret, buf);
    return ret;
}

char ** split_string (IN char * str, IN char * deliminator, OUT size_t * out_len) {
    if (str == NULL) return NULL;
    char * str_cpy = strndup (str, strlen (str));
    char * ptr = str_cpy;
    size_t no_of_tokens = 0;
    size_t delim_len = strlen (deliminator);
    // Count the nuber of tokens
    while (*ptr) {
        if (strncmp (ptr, deliminator, delim_len) == 0) {
            no_of_tokens++;
        }
        ptr++;
    }
    // if the delimitor is at the end.
    if (strncmp (ptr - delim_len, deliminator, delim_len) == 0 ) {
        no_of_tokens--;
    }
    // if the string does not star with the a delimiter..
    if (strncmp (str_cpy, deliminator, delim_len) != 0 ) {
        no_of_tokens++;
    }
    no_of_tokens++; // add empty space.
    errno = 0;
    char ** result = malloc0 (sizeof (char *) * no_of_tokens);
    if (result == NULL) {
        nob_log (NOB_ERROR, "Could not allocate memory: %s", strerror (errno));
        return NULL;
    }
    if (result) {
        size_t idx = 0;
        char * tok_ptr = str_cpy;

        char * token = strtok (tok_ptr, deliminator);
        while (token) {
            result[idx] = strndup (token, strlen (token));
            token = strtok (NULL, deliminator);
            idx++;
        }
    }
    // The last slot is just a null terminaor, we return the actual number
    // of tokens.
    *out_len = no_of_tokens - 1;
    free (str_cpy);
    return result;
}

char * nobber_strndup (const char * s, size_t len) {
    errno = 0;
    size_t s_len = strlen (s);
    if (s_len >= len) {
        s_len = len;
    }
    char * buf = malloc0 (len);
    if (buf == NULL) {
        if (!errno) {
            errno = ENOMEM;
        }
        return NULL;
    }
    for (size_t i = 0; i <= s_len; i++) {
        buf[i] = s[i];
    }

    return buf;
}

char * nobber_strrstr (IN char * haystack, IN char* needle) {
    size_t str_len = strlen (haystack);
    size_t n_len = strlen (needle);
    char * tmp_hs = haystack + str_len - n_len;
    bool found = false;

    while (str_len--) {
        if (strncmp (tmp_hs, needle, n_len)) {
            found = true;
            break;
        }
    }

    if (!found) return NULL;
    return tmp_hs;
}

char * substr (IN char * str, ssize_t pos, ssize_t len) {
    if (pos < 0) {
        NOB_TODO("substr(): Negative pos not yet implemented!");
    }

    if (len < 0) {
        NOB_TODO("substr(): Negative len not yet implemented!");
    }

    char * tmp_str = str;

    char * out_val = malloc0 (len + 1);
    tmp_str += pos;

    while (len--) *out_val++ = *tmp_str++;
    *out_val = '\0';

    return out_val;
}


char * get_parent_dir (IN char * path, OUT size_t * out_len) {
    size_t t_out_len;
    char * ret_val;
    char * dir_mark = nobber_strrstr (path, PATH_SEPARATOR);
    if (!dir_mark) {
        if (out_len) out_len = 0;
        return NULL;
    }

    size_t diff = strlen (path) - strlen (dir_mark);
    ret_val = malloc0 (diff + 1);

    while (dir_mark++) *ret_val = *dir_mark;

    return ret_val;
}


#endif // NOBBER_UTILS_IMPLEMENTATION
