#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include "test_macros.h"
#define NOBBER_UTILS_IMPLEMENTAITON
#include "utils.h"
#define NOB_IMPLEMENTATION
#define NOB_STRIP_PREFIX
#include "nob.h"

int test_split_string () {
    setup_unit ();
    {
        char months[] = "JAN,FEB,MAR,APR,MAY,JUN,JUL,AUG,SEP,OCT,NOV,DEC\0";
        char *  months_arr[] = {"JAN\0","FEB\0","MAR\0","APR\0","MAY\0","JUN\0","JUL\0","AUG\0","SEP\0","OCT\0","NOV\0","DEC\0"};
        size_t tokens_len;
        char ** tokens = split_string (months, ",", &tokens_len);
        test_case (tokens_len == 12, "Expeted 12, got %zi", tokens_len);
        for (size_t i = 0; i < tokens_len; i++) {
            test_case (strcmp (tokens[i],months_arr[i]) == 0, "Expected %s, got %s", tokens[i], months_arr[i]);
        }
    }
    end_unit ();
}

int main ([[maybe_unused]] int argc,[[maybe_unused]] char * argv[]) {
    setup_suite ("Nobber");
    test_unit (test_split_string, "split_string");
    end_suite ();
}
