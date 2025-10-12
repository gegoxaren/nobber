
#pragma once

#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <stdatomic.h>
#include <sys/time.h>

#ifndef RESET
/* Colour definitions for console prints */
#define RESET   "\033[0m"
#define BLACK   "\033[30m"      /* Black */
#define RED     "\033[31m"      /* Red */
#define GREEN   "\033[32m"      /* Green */
#define YELLOW  "\033[33m"      /* Yellow */
#define BLUE    "\033[34m"      /* Blue */
#define MAGENTA "\033[35m"      /* Magenta */
#define CYAN    "\033[36m"      /* Cyan */
#define WHITE   "\033[37m"      /* White */
#define BOLDBLACK   "\033[1m\033[30m"      /* Bold Black */
#define BOLDRED     "\033[1m\033[31m"      /* Bold Red */
#define BOLDGREEN   "\033[1m\033[32m"      /* Bold Green */
#define BOLDYELLOW  "\033[1m\033[33m"      /* Bold Yellow */
#define BOLDBLUE    "\033[1m\033[34m"      /* Bold Blue */
#define BOLDMAGENTA "\033[1m\033[35m"      /* Bold Magenta */
#define BOLDCYAN    "\033[1m\033[36m"      /* Bold Cyan */
#define BOLDWHITE   "\033[1m\033[37m"      /* Bold White */
#endif

/*
 * http://stackoverflow.com/a/1551641
 */
static inline
char *
test_current_time () {
   char * ret_val = malloc (50);
   char fmt[50];
   time_t t = time (NULL);
   struct timeval tv;
   gettimeofday (&tv, NULL);
   strftime (fmt, 50,"%Y-%m-%dT%H:%M:%S.%%i%z", localtime (&t));
   snprintf(ret_val, 50, fmt, tv.tv_usec);
   return ret_val;
}


/******************************************************************************/

#define setup_suite(sn)                                         \
  char * suit_name = sn;                                        \
  char * t_str = test_current_time ();                           \
  fprintf (stdout, MAGENTA "[%s]\n[STARTING TEST SUITE] %s\n"    \
  RESET, t_str, suit_name);                                     \
  free (t_str);                                               \
  unsigned int total_fails = 0;                                 \
  unsigned int tmp_val = 0;

#define end_suite()                                            \
  if (total_fails > 0){                                 \
    fprintf (stderr, RED "[TEST SUITE FAILED]\n"               \
    " Number of tests failed (total): %d\n" RESET,             \
    total_fails);                                              \
  } else {                                                     \
    fprintf (stdout, GREEN "[TEST SUITE PASSED] %s\n" RESET,   \
      suit_name);                                              \
  }                                                            \
  return total_fails;

/******************************************************************************/

#define setup_unit()            \
  unsigned int unit_retval = 0;

#define end_unit()    \
  return unit_retval;

#define test_case(cond, p, ...)                                     \
    if (cond) {                                                     \
      fprintf (stdout, GREEN "[PASS] " p " \n" RESET,               \
               ##__VA_ARGS__);                                      \
    } else {                                                        \
      fprintf (stderr, RED "[FAILED][%s:%d, %s] " p " \n" RESET,    \
      __FILE__, __LINE__, __func__ , ##__VA_ARGS__);                \
      unit_retval++;                                                \
    }


/******************************************************************************/


#define test_unit(fun, name){                                             \
  t_str = test_current_time ();                                           \
  fprintf (stdout, BLUE "[%s]\n[STARTING TEST UNIT] %s\n" RESET, t_str, name);\
  tmp_val = fun ();                                                       \
  free (t_str); t_str = test_current_time ();                             \
  if (tmp_val > 0) {                                                      \
    total_fails =+ tmp_val;                                               \
    fprintf (stderr, RED "[%s][TEST UNIT FAIL] %s\n" RESET,             \
     t_str, name);                                                        \
  } else {                                                                \
    fprintf (stdout, GREEN "[%s][TEST UNIT PASS] %s\n" RESET,           \
      t_str, name);                                                       \
  }                                                                       \
  free (t_str);                                                           \
}

