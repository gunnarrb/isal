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

#define EVENT_WAGEN_ARRIVAL	1
#define EVENT_WAGEN_DEPARTURE 2
#define EVENT_SKAUT_ARRIVAL 3
#define EVENT_SKAUT_DEPARTURE 4
#define EVENT_MACHINE_FAILURE 5
#define EVENT_MACHINE_FIXED 6
#define EVENT_END_SIMULATION 7
#define EVENT_END_WARMUP	8

// What streams do we need?

/* Model variables */
int num_machines, num_queues;
int machine2queue[num_machines +1], is_machine_busy[num_machines +1];
float work_time[num_stations + 1] // +1 is the less preferable simlib indexing scheme
FILE *infile, *outfile;

/* Function signatures */
void wagen_arrival();
void wagen_departure();
void skaut_arrival();
void skaut_departure();
void generate_report();

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

