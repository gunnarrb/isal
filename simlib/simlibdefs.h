/* This is simlibdefs.h. */

/* Define limits. */

#define MAX_LIST    101		/* Max number of lists. */
#define MAX_ATTR    30		/* Max number of attributes. */
#define MAX_SVAR    101		/* Max number of sampst variables. */
#define TIM_VAR     101		/* Max number of timest variables. */
#define MAX_TVAR    202		/* Max number of timest variables + lists. */
#define EPSILON     0.001	/* Used in event_cancel. */

/* Define array sizes. */

#define LIST_SIZE   102		/* MAX_LIST + 1. */
#define ATTR_SIZE   31		/* MAX_ATTR + 1. */
#define SVAR_SIZE   102		/* MAX_SVAR + 1. */
#define TVAR_SIZE   203		/* MAX_TVAR + 1. */

/* Define options for list_file and list_remove. */

#define FIRST        1		/* Insert at (remove from) head of list. */
#define LAST         2		/* Insert at (remove from) end of list. */
#define INCREASING   3		/* Insert in increasing order. */
#define DECREASING   4		/* Insert in decreasing order. */

/* Define some other values. */

#define LIST_EVENT   101		/* Event list number. */
#ifdef INFINITY
#undef INFINITY
#endif
#define INFINITY     1.E30	/* Not really infinity, but a very large number. */

/* Pre-define attribute numbers of transfer for event list. */

#define EVENT_TIME   1		/* Attribute 1 in event list is event time. */
#define EVENT_TYPE   2		/* Attribute 2 in event list is event type. */
