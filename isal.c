/*
 *  isal.c
 *  
 *
 *  Created by Gunnarr Baldursson on 4/18/11.
 *  Copyright 2011 Haskoli Islands. All rights reserved.
 *
 */

#include "simlib/rndlib.h"
#include "simlib/simlib.h"

#define EVENT_ARRIVAL	1
#define EVENT_DEPARTURE 2
#define EVENT_END_SIMULATION 3
#define EVENT_END_WARMUP	4

// Do we nee streams for arrivals or failures perhaps? */

/* Model variables */
int num_stations, num_queues;
float work_time[num_stations + 1] // +1 is the less preferable simlib indexing scheme
FILE *infile, *outfile;

/* Function signatures */
void arrive();
void departure();
void report();

int main()
{
	// read input parameters
	// write to output file
	
	// Initialize rndlib
	init_twister();
	
	// Initialize simlib
	init_simlib();
	maxatr = 4; // how many attributes do we need?
	
}

