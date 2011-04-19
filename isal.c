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
int number_of_machines, min_productivity, min_no_failures, max_no_failures;
float mean_wagen_arrival, std_wagen_arrival, mean_failures, std_failures, min_machine_repair_time, max_machine_repair_time, end_warmup_time, end_simulation_time; 

int is_machine_busy[NUM_MACHINES +1],
	queue_size[NUM_QUEUES +1];
	
float work_time[NUM_MACHINES + 1]; // +1 is the less preferable simlib indexing scheme
FILE *infile, *outfile;

/* Function signatures */

// Usage:	wagen_arrival();
// Pre:		EVENT_WAGEN_ARRIVAL is the next event to be processed
// Post:	14 skaut have been assigned to unit B, and their arrival events scheduled
void wagen_arrival();

// Usage:	wagen_departure();
// Pre:		EVENT_WAGEN_DEPARTURE is the next event to be processed
// Post:	simlib statistical functions were called,
//			an EVENT_WAGEN_ARRIVAL has been scheduled after 30 minutes
void wagen_departure();

// Usage:	skaut_arrival();
// Pre:		EVENT_SKAUT_ARRIVAL is the next event to be processed
// Post:	a skaut has been processed by a machine or put in it's queue.
//			subsequent events may have been scheduled
void skaut_arrival();

// Usage:	skaut_departure();
// Pre:		EVENT_SKAUT_DEPARTURE is the next event to be processed
// Post:	
void skaut_departure(); // do we need an event for departure? 

// Usage:	machine_failure();
// Pre:		EVENT_MACHINE_FAILURE is the next event to be processed
// Post:	
void machine_failure();

// Usage:	machine_fixed();
// Pre:		EVENT_MACHINE_FIXED is the next event to be processed
// Post:	
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

// Usage:	schedule_failures(i);
// Pre:		the global variable end_simulation_time has a value, i is of type int
// Post:	i failures have been scheduled uniformly on machines
//			with ?random? repair times on the interval [min_machine_repair_time,...max_machine_repair_time]
//			uniformly distributed over the interval 0...end_simulation_time
void schedule_failures(int i);

void queue_is_full();

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
		transfer[3] = 1; 
		event_schedule( nrand(STREAM_WAGEN_ARRIVAL), EVENT_WAGEN_ARRIVAL );
		
		/* Schedule end of warmup time */
		event_schedule( end_warmup_time, EVENT_END_WARMUP );
		
		/* Schedule simulation termination */
		event_schedule( end_simulation_time, EVENT_END_SIMULATION );
		
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

void wagen_arrival()
{
	int i;
	for (i = 0; i < 14; i++) {
		if (is_machine_busy[2]) {
			if (LIST_SIZE[2] == queue_size[2]) {
				queue_is_full();
				break;
			} else {
				transfer[3] = 2; // we are currently in unit 2
				list_file(LAST, number_of_machines + 2); // skaut appended to machine B's queue
			}

		} else {
			transfer[3] = 2;
			list_file(LAST, 2); // skaut put in machine B
			is_machine_busy[2] = 1; // machine is busy
			sampst(0.0, 2); // the delay is zero
		}

	}
	// DO SIMLIB STATISTICS?
	event_schedule( sim_time + work_time[1], EVENT_WAGEN_DEPARTURE ); // using simlib's less preferable indexing scheme for machine A's work time
}

float N(float muy, float sigma, int stream)
{
		// This method of converting from N(0,1) to N(muy,sigma) has not been verified!
		float x = nrand(stream);
			return (x*sigma)+30;
}

