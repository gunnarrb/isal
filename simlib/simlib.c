/*
  Thomas Philip Runarsson (email: tpr@hi.is) 
  Time-stamp: "2006-03-19 11:06:21 tpr"
*/

/* This is simlib.c (adapted from SUPERSIMLIB, written by Gregory Glockner). */


/* Include files. */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "simlibdefs.h"

/* Declare simlib global variables. */

int *list_rank, *list_size, next_event_type, maxatr = 0, maxlist = 0;
double *transfer, sim_time;
struct master
{
  double *value;
  struct master *pr;
  struct master *sr;
} **head, **tail;

/* Declare simlib functions. */
void init_simlib (void);
void list_file (int option, int list);
void list_remove (int option, int list);
void timing (void);
void event_schedule (double time_of_event, int type_of_event);
int event_cancel (int event_type);
double sampst (double value, int variable);
double timest (double value, int variable);
double filest (int list);
void out_sampst (FILE * unit, int lowvar, int highvar);
void out_timest (FILE * unit, int lowvar, int highvar);
void out_filest (FILE * unit, int lowlist, int highlist);
void pprint_out (FILE * unit, int i);

void
init_simlib ()
{

/* Initialize simlib.c.  List LIST_EVENT is reserved for event list, ordered by
   event time.  init_simlib must be called from main by user. */

  int list, listsize;

  if (maxlist < 1)
    maxlist = MAX_LIST;
  listsize = maxlist + 1;

  /* Initialize system attributes. */

  sim_time = 0.0;
  if (maxatr < 4)
    maxatr = MAX_ATTR;

  /* Allocate space for the lists. */

  list_rank = (int *) calloc (listsize, sizeof (int));
  list_size = (int *) calloc (listsize, sizeof (int));
  head = (struct master **) calloc (listsize, sizeof (struct master *));
  tail = (struct master **) calloc (listsize, sizeof (struct master *));
  transfer = (double *) calloc (maxatr + 1, sizeof (double));

  /* Initialize list attributes. */

  for (list = 1; list <= maxlist; ++list)
    {
      head[list] = NULL;
      tail[list] = NULL;
      list_size[list] = 0;
      list_rank[list] = 0;
    }

  /* Set event list to be ordered by event time. */

  list_rank[LIST_EVENT] = EVENT_TIME;

  /* Initialize statistical routines. */

  sampst (0.0, 0);
  timest (0.0, 0);
}

void
list_file (int option, int list)
{

/* Place transfr into list "list".
   Update timest statistics for the list.
   option = FIRST place at start of list
            LAST  place at end of list
            INCREASING  place in increasing order on attribute list_rank(list)
            DECREASING  place in decreasing order on attribute list_rank(list)
            (ties resolved by FIFO) */

  struct master *row=NULL, *ahead, *behind, *ihead, *itail;
  int item, postest;

  /* If the list value is improper, stop the simulation. */

  if (!((list >= 0) && (list <= MAX_LIST)))
    {
      printf ("\nInvalid list %d for list_file at time %f\n", list, sim_time);
      exit (1);
    }

  /* Increment the list size. */

  list_size[list]++;

  /* If the option value is improper, stop the simulation. */

  if (!((option >= 1) && (option <= DECREASING)))
    {
      printf ("\n%d is an invalid option for list_file on list %d at time %f\n", option, list, sim_time);
      exit (1);
    }

  /* If this is the first record in this list, just make space for it. */

  if (list_size[list] == 1)
    {

      row = (struct master *) malloc (sizeof (struct master));
      head[list] = row;
      tail[list] = row;
      (*row).pr = NULL;
      (*row).sr = NULL;
    }

  else
    {				/* There are other records in the list. */

      /* Check the value of option. */

      if ((option == INCREASING) || (option == DECREASING))
	{
	  item = list_rank[list];
	  if (!((item >= 1) && (item <= maxatr)))
	    {
	      printf ("%d is an improper value for rank of list %d at time %f\n", item, list, sim_time);
	      exit (1);
	    }

	  row = head[list];
	  behind = NULL;	/* Dummy value for the first iteration. */

	  /* Search for the correct location. */

	  if (option == INCREASING)
	    {
	      postest = (transfer[item] >= (*row).value[item]);
	      while (postest)
		{
		  behind = row;
		  row = (*row).sr;
		  postest = (behind != tail[list]);
		  if (postest)
		    postest = (transfer[item] >= (*row).value[item]);
		}
	    }

	  else
	    {

	      postest = (transfer[item] <= (*row).value[item]);
	      while (postest)
		{
		  behind = row;
		  row = (*row).sr;
		  postest = (behind != tail[list]);
		  if (postest)
		    postest = (transfer[item] <= (*row).value[item]);
		}
	    }

	  /* Check to see if position is first or last.  If so, take care of
	     it below. */

	  if (row == head[list])

	    option = FIRST;

	  else
	   if (behind == tail[list])

	    option = LAST;

	  else
	    {			/* Insert between preceding and succeeding records. */

	      ahead = (*behind).sr;
	      row = (struct master *) malloc (sizeof (struct master));
	      (*row).pr = behind;
	      (*behind).sr = row;
	      (*ahead).pr = row;
	      (*row).sr = ahead;
	    }
	}			/* End if inserting in increasing or decreasing order. */

      if (option == FIRST)
	{
	  row = (struct master *) malloc (sizeof (struct master));
	  ihead = head[list];
	  (*ihead).pr = row;
	  (*row).sr = ihead;
	  (*row).pr = NULL;
	  head[list] = row;
	}
      if (option == LAST)
	{
	  row = (struct master *) malloc (sizeof (struct master));
	  itail = tail[list];
	  (*row).pr = itail;
	  (*itail).sr = row;
	  (*row).sr = NULL;
	  tail[list] = row;
	}
    }

  /* Copy the data. */

  (*row).value = transfer;

  /* Make room for new transfer. */

  transfer = (double *) calloc (maxatr + 1, sizeof (double));

  /* Update the area under the number-in-list curve. */

  timest ((double) list_size[list], TIM_VAR + list);
}

void
list_remove (int option, int list)
{

/* Remove a record from list "list" and copy attributes into transfer.
   Update timest statistics for the list.
   option = FIRST remove first record in the list
            LAST  remove last record in the list */

  struct master *row = NULL, *ihead, *itail;

  /* If the list value is improper, stop the simulation. */

  if (!((list >= 0) && (list <= MAX_LIST)))
    {
      printf ("\nInvalid list %d for list_remove at time %f\n", list, sim_time);
      exit (1);
    }

  /* If the list is empty, stop the simulation. */

  if (list_size[list] <= 0)
    {
      printf ("\nUnderflow of list %d at time %f\n", list, sim_time);
      exit (1);
    }

  /* Decrement the list size. */

  list_size[list]--;

  /* If the option value is improper, stop the simulation. */

  if (!(option == FIRST || option == LAST))
    {
      printf ("\n%d is an invalid option for list_remove on list %d at time %f\n", option, list, sim_time);
      exit (1);
    }

  if (list_size[list] == 0)
    {

      /* There is only 1 record, so remove it. */

      row = head[list];
      head[list] = NULL;
      tail[list] = NULL;
    }

  else
    {

      /* There is more than 1 record, so remove according to the desired
         option. */

      switch (option)
	{

	  /* Remove the first record in the list. */

	case FIRST:
	  row = head[list];
	  ihead = (*row).sr;
	  (*ihead).pr = NULL;
	  head[list] = ihead;
	  break;

	  /* Remove the last record in the list. */

	case LAST:
	  row = tail[list];
	  itail = (*row).pr;
	  (*itail).sr = NULL;
	  tail[list] = itail;
	  break;
	}
    }

  /* Copy the data and free memory. */

  free ((char *) transfer);
  transfer = (*row).value;
  free ((char *) row);

  /* Update the area under the number-in-list curve. */

  timest ((double) list_size[list], TIM_VAR + list);
}

void
timing ()
{

/* Remove next event from event list, placing its attributes in transfer.
   Set sim_time (simulation time) to event time, transfer[1].
   Set next_event_type to this event type, transfer[2]. */

  /* Remove the first event from the event list and put it in transfer[]. */

  list_remove (FIRST, LIST_EVENT);

  /* Check for a time reversal. */

  if (transfer[EVENT_TIME] < sim_time)
    {
      printf ("\nAttempt to schedule event type %f for time %f at time %f\n",
	      transfer[EVENT_TYPE], transfer[EVENT_TIME], sim_time);
      exit (1);
    }

  /* Advance the simulation clock and set the next event type. */

  sim_time = transfer[EVENT_TIME];
  next_event_type = transfer[EVENT_TYPE];
}

void
event_schedule (double time_of_event, int type_of_event)
{

/* Schedule an event at time event_time of type event_type.  If attributes
   beyond the first two (reserved for the event time and the event type) are
   being used in the event list, it is the user's responsibility to place their
   values into the transfer array before invoking event_schedule. */

  transfer[EVENT_TIME] = time_of_event;
  transfer[EVENT_TYPE] = type_of_event;
  list_file (INCREASING, LIST_EVENT);
}

int
event_cancel (int event_type)
{

/* Remove the first event of type event_type from the event list, leaving its
   attributes in transfer.  If something is cancelled, event_cancel returns 1;
   if no match is found, event_cancel returns 0. */

  struct master *row, *ahead, *behind;
  static double high, low, value;

  /* If the event list is empty, do nothing and return 0. */

  if (list_size[LIST_EVENT] == 0)
    return 0;

  /* Search the event list. */

  row = head[LIST_EVENT];
  low = event_type - EPSILON;
  high = event_type + EPSILON;
  value = (*row).value[EVENT_TYPE];

  while (((value <= low) || (value >= high)) && (row != tail[LIST_EVENT]))
    {
      row = (*row).sr;
      value = (*row).value[EVENT_TYPE];
    }

  /* Check to see if this is the end of the event list. */

  if (row == tail[LIST_EVENT])
    {

      /* Double check to see that this is a match. */

      if ((value > low) && (value < high))
	{
	  list_remove (LAST, LIST_EVENT);
	  return 1;
	}

      else			/* no match */
	return 0;
    }

  /* Check to see if this is the head of the list.  If it is at the head, then
     it MUST be a match. */

  if (row == head[LIST_EVENT])
    {
      list_remove (FIRST, LIST_EVENT);
      return 1;
    }

  /* Else remove this event somewhere in the middle of the event list. */

  /* Update pointers. */

  ahead = (*row).sr;
  behind = (*row).pr;
  (*behind).sr = ahead;
  (*ahead).pr = behind;

  /* Decrement the size of the event list. */

  list_size[LIST_EVENT]--;

  /* Copy and free memory. */

  free ((char *) transfer);	/* Free the old transfer. */
  transfer = (*row).value;	/* Transfer the data. */
  free ((char *) row);		/* Free the space vacated by row. */

  /* Update the area under the number-in-event-list curve. */

  timest ((double) list_size[LIST_EVENT], TIM_VAR + LIST_EVENT);
  return 1;
}

double
sampst (double value, int variable)
{

/* Initialize, update, or report statistics on discrete-time processes:
   sum/average, max (default -1E30), min (default 1E30), number of observations
   for sampst variable "variable", where "variable":
       = 0 initializes accumulators
       > 0 updates sum, count, min, and max accumulators with new observation
       < 0 reports stats on variable "variable" and returns them in transfer:
           [1] = average of observations
           [2] = number of observations
           [3] = maximum of observations
           [4] = minimum of observations */

  static int ivar, num_observations[SVAR_SIZE];
  static double max[SVAR_SIZE], min[SVAR_SIZE], sum[SVAR_SIZE];

  /* If the variable value is improper, stop the simulation. */

  if (!(variable >= -MAX_SVAR) && (variable <= MAX_SVAR))
    {
      printf ("\n%d is an improper value for a sampst variable at time %f\n", variable, sim_time);
      exit (1);
    }

  /* Execute the desired option. */

  if (variable > 0)
    {				/* Update. */
      sum[variable] += value;
      if (value > max[variable])
	max[variable] = value;
      if (value < min[variable])
	min[variable] = value;
      num_observations[variable]++;
      return 0.0;
    }

  if (variable < 0)
    {				/* Report summary statistics in transfer. */
      ivar = -variable;
      transfer[2] = (double) num_observations[ivar];
      transfer[3] = max[ivar];
      transfer[4] = min[ivar];
      if (num_observations[ivar] == 0)
	transfer[1] = 0.0;
      else
	transfer[1] = sum[ivar] / transfer[2];
      return transfer[1];
    }

  /* Initialize the accumulators. */

  for (ivar = 1; ivar <= MAX_SVAR; ++ivar)
    {
      sum[ivar] = 0.0;
      max[ivar] = -INFINITY;
      min[ivar] = INFINITY;
      num_observations[ivar] = 0;
    }

  return 0.0;
}

double
timest (double value, int variable)
{

/* Initialize, update, or report statistics on continuous-time processes:
   integral/average, max (default -1E30), min (default 1E30)
   for timest variable "variable", where "variable":
       = 0 initializes counters
       > 0 updates area, min, and max accumulators with new level of variable
       < 0 reports stats on variable "variable" and returns them in transfer:
           [1] = time-average of variable updated to the time of this call
           [2] = maximum value variable has attained
           [3] = minimum value variable has attained
   Note that variables TIM_VAR + 1 through TVAR_SIZE are used for automatic
   record keeping on the length of lists 1 through MAX_LIST. */

  int ivar;
  static double area[TVAR_SIZE], max[TVAR_SIZE], min[TVAR_SIZE], preval[TVAR_SIZE], tlvc[TVAR_SIZE], treset;

  /* If the variable value is improper, stop the simulation. */

  if (!(variable >= -MAX_TVAR) && (variable <= MAX_TVAR))
    {
      printf ("\n%d is an improper value for a timest variable at time %f\n", variable, sim_time);
      exit (1);
    }

  /* Execute the desired option. */

  if (variable > 0)
    {				/* Update. */
      area[variable] += (sim_time - tlvc[variable]) * preval[variable];
      if (value > max[variable])
	max[variable] = value;
      if (value < min[variable])
	min[variable] = value;
      preval[variable] = value;
      tlvc[variable] = sim_time;
      return 0.0;
    }

  if (variable < 0)
    {				/* Report summary statistics in transfer. */
      ivar = -variable;
      area[ivar] += (sim_time - tlvc[ivar]) * preval[ivar];
      tlvc[ivar] = sim_time;
      transfer[1] = area[ivar] / (sim_time - treset);
      transfer[2] = max[ivar];
      transfer[3] = min[ivar];
      return transfer[1];
    }

  /* Initialize the accumulators. */

  for (ivar = 1; ivar <= MAX_TVAR; ++ivar)
    {
      area[ivar] = 0.0;
      max[ivar] = -INFINITY;
      min[ivar] = INFINITY;
      preval[ivar] = 0.0;
      tlvc[ivar] = sim_time;
    }
  treset = sim_time;

  return 0.0;
}

double
filest (int list)
{

/* Report statistics on the length of list "list" in transfer:
       [1] = time-average of list length updated to the time of this call
       [2] = maximum length list has attained
       [3] = minimum length list has attained
   This uses timest variable TIM_VAR + list. */

  return timest (0.0, -(TIM_VAR + list));
}

void
out_sampst (FILE * unit, int lowvar, int highvar)
{

/* Write sampst statistics for variables lowvar through highvar on file
   "unit". */

  int ivar, iatrr;

  if (lowvar > highvar || lowvar > MAX_SVAR || highvar > MAX_SVAR)
    return;

  fprintf (unit, "\n sampst                         Number");
  fprintf (unit, "\nvariable                          of");
  fprintf (unit, "\n number       Average           values          Maximum");
  fprintf (unit, "          Minimum");
  fprintf (unit, "\n___________________________________");
  fprintf (unit, "_____________________________________");
  for (ivar = lowvar; ivar <= highvar; ++ivar)
    {
      fprintf (unit, "\n\n%5d", ivar);
      sampst (0.00, -ivar);
      for (iatrr = 1; iatrr <= 4; ++iatrr)
	pprint_out (unit, iatrr);
    }
  fprintf (unit, "\n___________________________________");
  fprintf (unit, "_____________________________________\n\n\n");
}

void
out_timest (FILE * unit, int lowvar, int highvar)
{

/* Write timest statistics for variables lowvar through highvar on file
   "unit". */

  int ivar, iatrr;

  if (lowvar > highvar || lowvar > TIM_VAR || highvar > TIM_VAR)
    return;


  fprintf (unit, "\n  timest");
  fprintf (unit, "\n variable       Time");
  fprintf (unit, "\n  number       average          Maximum          Minimum");
  fprintf (unit, "\n________________________________________________________");
  for (ivar = lowvar; ivar <= highvar; ++ivar)
    {
      fprintf (unit, "\n\n%5d", ivar);
      timest (0.00, -ivar);
      for (iatrr = 1; iatrr <= 3; ++iatrr)
	pprint_out (unit, iatrr);
    }
  fprintf (unit, "\n________________________________________________________");
  fprintf (unit, "\n\n\n");
}

void
out_filest (FILE * unit, int lowlist, int highlist)
{

/* Write timest list-length statistics for lists lowlist through highlist on
   file "unit". */

  int list, iatrr;

  if (lowlist > highlist || lowlist > MAX_LIST || highlist > MAX_LIST)
    return;

  fprintf (unit, "\n  File         Time");
  fprintf (unit, "\n number       average          Maximum          Minimum");
  fprintf (unit, "\n_______________________________________________________");
  for (list = lowlist; list <= highlist; ++list)
    {
      fprintf (unit, "\n\n%5d", list);
      filest (list);
      for (iatrr = 1; iatrr <= 3; ++iatrr)
	pprint_out (unit, iatrr);
    }
  fprintf (unit, "\n_______________________________________________________");
  fprintf (unit, "\n\n\n");
}

void
pprint_out (FILE * unit, int i)	/* Write ith entry in transfer to file
				   "unit". */
{
  if (transfer[i] == -1e30 || transfer[i] == 1e30)
    fprintf (unit, " %#15.6G ", 0.00);
  else
    fprintf (unit, " %#15.6G ", transfer[i]);
}
