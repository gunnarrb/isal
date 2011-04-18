/* This is simlib.h. */

/* Include files. */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "simlibdefs.h"

/* Declare simlib global variables. */

extern int *list_rank, *list_size, next_event_type, maxatr, maxlist;
extern double *transfer, sim_time;
extern struct master
{
  double *value;
  struct master *pr;
  struct master *sr;
} **head, **tail;

/* Declare simlib functions. */

extern void init_simlib (void);
extern void list_file (int option, int list);
extern void list_remove (int option, int list);
extern void timing (void);
extern void event_schedule (double time_of_event, int type_of_event);
extern int event_cancel (int event_type);
extern double sampst (double value, int varibl);
extern double timest (double value, int varibl);
extern double filest (int list);
extern void out_sampst (FILE * unit, int lowvar, int highvar);
extern void out_timest (FILE * unit, int lowvar, int highvar);
extern void out_filest (FILE * unit, int lowlist, int highlist);
