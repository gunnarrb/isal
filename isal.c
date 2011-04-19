/*
 *  isal.c
 *  
 *
 *  Created by Gunnarr Baldursson & Ragnar Gisli Olafsson on 4/18/11.
 *  Copyright 2011 Haskoli Islands. All rights reserved.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "simlib/rndlib.h"
#include "simlib/simlib.h"

#incluce 

// EVENTS
#define EVENT_WAGEN_ARRIVAL	1
#define EVENT_WAGEN_DEPARTURE 2
#define EVENT_SKAUT_ARRIVAL 3
#define EVENT_SKAUT_DEPARTURE 4
#define EVENT_MACHINE_FAILURE 5
#define EVENT_MACHINE_FIXED 6
#define EVENT_END_SIMULATION 7
#define EVENT_END_WARMUP	8

// STREAMS
#define STREAM_WAGEN_ARRIVAL 1

//Other constants
#define NUM_MACHINES 7
#define NUM_QUEUES 5 // vid flokkum lasa einnig sem bidradir

// Global variables
int Number_of_machiens, Min_productivity, Min_no_failures, Max_no_failures;
float Mean_wagen_arrival, Std_wagen_arrival, Mean_failures, Std_failures, Min_machine_repair_time, Max_machine_repair_time, End_warmup_time, End_simulation_time;

int machine2queue[NUM_MACHINES +1], 
	is_machine_busy[NUM_MACHINES +1],
	queue_size[NUM_QUEUES +1];
	
float work_time[NUM_MACHINES + 1]; // +1 is the less preferable simlib indexing scheme
FILE *infile, *outfile;

/* Function signatures */

// Usage:	wagen_arrival();
// Post:	14 skaut have been assigned to unit B, and their arrival events scheduled
//			an EVENT_WAGEN_ARRIVAL has been scheduled after 30 minutes
void wagen_arrival();

void wagen_departure(); // do we need an event for wagen departure? WHY?

// Usage:	skaut_arrival();
// Post:	a skaut has been processed by a machine or put in it's queue.
//			subsequent events may have been scheduled
void skaut_arrival();

void skaut_departure(); // do we need an event for departure? 



void machine_failure();

void machine_fixed();


// Usage:	end_warmup();
// Post:	SIMLIB statistical variables have been cleared
void end_warmup();

// Usage:	parse_input(input_filename_data,input_filename_time, output_filename);
// Pre:		input_filename_data,input_filename_time output_filename are of type char[],
//			global variables from the input file exist.
// Post:	the global variables were assigned values from input_filename, 
//			the variables along with their values were written to output_filename
void parse_input(char[] ,char[], char[]);

// Usage:	x = N(muy, sigma, stream);
// Pre:		muy and sigma are of type float
//			stream is of type int
// Post:	x is a random gaussian distributed variable of type float 
//			with mean muy and std sigma
float N(float muy, float sigma, int stream);

// Usage:	report("the_report.out");
// Pre:		the values to be reported have values
// Post:	a report on program values and simlib statistics 
//			have been APPENDED to "the_report.out"
void report(char[]);

int main()
{
	//parse_input("adal_inntak.in","velar_og_bidradir.in","output.out");
	
	// We perform simulation for "a few" failures per day
	int i;
	for (i = Min_no_failures; i < Max_no_failures; i++) {
		// Initialize rndlib
		init_twister();
	
		// Initialize simlib
		init_simlib();
		maxatr = 4; // how many attributes do we need?
		
		/* Schedule first wagen arrival */
		event_schedule( nrand(STREAM_WAGEN_ARRIVAL), EVENT_WAGEN_ARRIVAL );
		
		/* Schedule end of warmup time */
		event_schedule( End_warmup_time, EVENT_END_WARMUP );
		
		/* Schedule simulation termination */
		event_schedule( End_simulation_time, EVENT_END_SIMULATION );
		
		while (next_event_type != EVENT_END_SIMULATION) {
	
			timing();
		
			switch (next_event_type) {
				case EVENT_WAGEN_ARRIVAL:
					wagen_arrival();
					break;
				case EVENT_WAGEN_DEPARTURE:
					wagen_departure();
					break;
				case EVENT_SKAUT_ARRIVAL:
					skaut_arrival();
					break;
				case EVENT_SKAUT_DEPARTURE:
					skaut_departure();
					break;
				case EVENT_MACHINE_FAILURE:
					machine_failure();
					break;
				case EVENT_MACHINE_FIXED:
					machine_fixed();
					break;
				case EVENT_END_WARMUP:
					end_warmup();
					break;
				case EVENT_END_SIMULATION:
					report();
					break;
			}
		}
	}
}

