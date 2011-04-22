/*
 *  isal.c
 *  
 *
 *  Created by Gunnarr Baldursson & Ragnar Gisli Olafsson on 4/18/11.
 *  Copyright 2011 Haskoli Islands. All rights reserved.
 *
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include "simlib/rndlib.h"
#include "simlib/simlib.h"

// EVENTS
#define EVENT_WAGEN_UNLOAD_ARRIVAL	1
#define EVENT_WAGEN_UNLOAD_DEPARTURE 2
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
#define WAGEN_LOAD 14
#define MACHINES_ON_THE_LEFT_SIDE 5
#define MACHINES_ON_THE_RIGHT_SIDE 2
#define OPTIMAL_THROUGHPUT 52
#define ACTUAL_THROUGHPUT 40
#define TRANSFER_ARRAY_LENGTH 11

//#define LOADING_TIME_PER_SKAUT 

// Global variables
int number_of_machines, min_productivity, min_no_failures, max_no_failures, skaut_throughput;
float mean_wagen_arrival, std_wagen_arrival, mean_failures, std_failures, min_machine_repair_time, max_machine_repair_time, end_warmup_time, end_simulation_time; 
int SAMPST_DELAYS, THROUGHPUT_TIME; // variable for queue delays and throughput time

int skaut_id;

int queue_size[NUM_MACHINES +1];

float work_time[NUM_MACHINES + 1],
transfer_time[NUM_MACHINES +1]; // +1 is the less preferable simlib indexing scheme

float temp_transfer[TRANSFER_ARRAY_LENGTH];

FILE *infile, *outfile;

/* Function signatures */

// Usage:	push_array();
// Pre:		we expect that correct values are in transfer array
// Post:	our temp_transfer array now has the values in transfer_array
void push_array();

// Usage:	pop_array();
// Pre:		we expect that correct values are in transfer_temp array
// Post:	our transfer array now has the values in transfer_temp
void pop_array();

// Usage:	wagen_arrival();
// Pre:		EVENT_WAGEN_UNLOAD_ARRIVAL is the next event to be processed
// Post:	14 EVENT_SKAUT_ARRIVAL events are next to be processed on the event list.
void wagen_unload_arrival();

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

// Usage:	parse_input(input_filename_data,input_filename_time);
// Pre:		input_filename_data,input_filename_time of type char[],
//			global variables from the input file exist.
// Post:	the global variables were assigned values from input_filename, 
//			
void parse_input(char[] ,char[]);

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
void report();

// Usage:	schedule_failures(i);
// Pre:		the global variable end_simulation_time has a value, i is of type int
// Post:	i failures have been scheduled uniformly on machines
//			with ?random? repair times on the interval [min_machine_repair_time,...max_machine_repair_time]
//			uniformly distributed over the interval 0...end_simulation_time
void schedule_failures(int i);

void queue_is_full();

int main()
{
	
	// load datafiles
	parse_input("adal_inntak.in","velar_og_bidradir.in");
	SAMPST_DELAYS = number_of_machines +1;
	THROUGHPUT_TIME = number_of_machines +2;
	
	int b;
	for (b=1; b <= number_of_machines; b++) {
		printf("transfer_time[%d] = %f\n", b,transfer_time[b] );
	}
	
	// We perform simulation for "a few" failures per day
	int i;
	for (i = min_no_failures; i < max_no_failures; i++) {
	//for ( i=1; i<2; i++) {
		skaut_id = 1;
		skaut_throughput = 0;
		
		// Initialize rndlib
		init_twister();
		
		// Initialize simlib
		init_simlib();
		maxatr = 6; // how many attributes do we need?
		
		/* Schedule first wagen arrival */
		//transfer[3] = 1.0; 
		event_schedule( 1.0, EVENT_WAGEN_UNLOAD_ARRIVAL );
		
		/* Schedule end of warmup time */
		event_schedule( end_warmup_time, EVENT_END_WARMUP );
		
		/* Schedule simulation termination */
		event_schedule( end_simulation_time, EVENT_END_SIMULATION );
		
		while (next_event_type != EVENT_END_SIMULATION) {
			
			timing();
			printf("event_type = %d, transfer[3] = %f\n", next_event_type, transfer[3]);
			int k;
			for (k = 1; k <= number_of_machines; k++)
				printf("Items in machines/queues %d:  %d, %d\n", k, list_size[k], list_size[number_of_machines +k]);
			printf("\n");
			
			
			switch (next_event_type) {
				case EVENT_WAGEN_UNLOAD_ARRIVAL:
					wagen_unload_arrival();
					break;
				case EVENT_SKAUT_ARRIVAL:
					skaut_arrival();
					break;
				case EVENT_SKAUT_DEPARTURE:
					skaut_departure();
					break;
				case EVENT_MACHINE_FAILURE:
					//machine_failure();
					break;
				case EVENT_MACHINE_FIXED:
					//machine_fixed();
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

void wagen_unload_arrival()
{
	
	int i;
	for (i=1; i <= WAGEN_LOAD; i ++) {
		
		transfer[3]=1.0;
		transfer[4] = sim_time + (i * 0.01); // skaut entering system time
		transfer[6] = (float) skaut_id++;
		//printf("tr4 in wagen: %f\n", transfer[4]);
		event_schedule( sim_time + ( i* 0.01),	EVENT_SKAUT_ARRIVAL);
	}

	float wagen_arrival_zeit = sim_time + 30.0 * 60.0; // this should be sampled from a distribution!!
	event_schedule(wagen_arrival_zeit, EVENT_WAGEN_UNLOAD_ARRIVAL);
}


void skaut_arrival() 
{
	push_array();
	int current_unit = (int)transfer[3];

	if (list_size[current_unit] == 0) {
		sampst(0.0, SAMPST_DELAYS);
		sampst(0.0, current_unit);
		
		list_file(FIRST, current_unit); // last := first here because there are only to be 0 or 1 items in machine
		
		// schedule departure after machine processing time
		pop_array();
		event_schedule(sim_time + work_time[current_unit], EVENT_SKAUT_DEPARTURE);
		
	} else {
		
		if (list_size[number_of_machines + current_unit] == queue_size[current_unit]) {
			printf("BOOM! UNIT %d exploded with %d items!\n", current_unit, list_size[number_of_machines + current_unit]);
			exit(1);
		} else {
			transfer[5] = sim_time;
			list_file(LAST, number_of_machines + current_unit);
			//printf("puting skaut in queue: %d\n", current_unit);
		}
		
	}

}

void skaut_departure()
{
	push_array();
	int current_unit = (int) transfer[3];
		
	if (current_unit == MACHINES_ON_THE_LEFT_SIDE) {
		skaut_throughput += 2;
		sampst(sim_time - transfer[4], THROUGHPUT_TIME);
		list_remove(FIRST,current_unit);  
	} else {
		list_remove(FIRST,current_unit);
		pop_array();
		transfer[3]++;
		event_schedule(sim_time + work_time[current_unit], EVENT_SKAUT_ARRIVAL);
	}
	
		
	if (list_size[number_of_machines + current_unit] != 0) {
		pop_array();
		
		list_file(FIRST,current_unit); // first equals last because size should only be 1
		pop_array();

		list_remove(FIRST, number_of_machines + current_unit);
		pop_array();

		sampst(sim_time - transfer[5], SAMPST_DELAYS);
		sampst(sim_time - transfer[5], current_unit);
		event_schedule(sim_time + work_time[current_unit], EVENT_SKAUT_DEPARTURE);
	}
}


void parse_input(char inputfile_data[], char inputfile_time[])
{
	
	
	if ((infile = fopen (inputfile_data, "r")) == NULL) {
		printf("Could not open file %s\n",inputfile_data);
	}
	
	fscanf (infile, "%d %d %d %d %f %f %f %f %f %f", &number_of_machines, &min_productivity, &min_no_failures, &max_no_failures, &mean_wagen_arrival, &std_wagen_arrival,  &min_machine_repair_time, &max_machine_repair_time, &end_warmup_time, &end_simulation_time);
	fclose(infile);
	
	
	if ((infile = fopen (inputfile_time, "r")) == NULL) {
		printf("Could not open file %s\n",inputfile_time);
	} 
	printf( "%d %d %d %d %f %f %f %f %f %f\n", number_of_machines, min_productivity, min_no_failures, max_no_failures, mean_wagen_arrival, std_wagen_arrival,  min_machine_repair_time, max_machine_repair_time, end_warmup_time, end_simulation_time);
	
	int counter = 1;
	while (!feof(infile)) {
		fscanf(infile, "%f %d %f", &transfer_time[counter], &queue_size[counter], &work_time[counter] );
		printf("%f %d %f\n", transfer_time[counter], queue_size[counter], work_time[counter] );
		counter++;
	}
	fclose(infile);
	
}

void end_warmup()
{
	sampst(0.0, 0); 
	timest(0.0, 0);
}

void report()
{
	printf("System throughput: %d\n", skaut_throughput );
	int i;
	for (i=1; i <= number_of_machines; i++) {
		printf("Machine %d: %f\n", i, filest(i) );
	}
	for (i=1; i <= number_of_machines; i++) {
		printf("Avg delay in queue %d: %f\n", i, sampst(0.0, -i));
	}
	printf("Avarage queue delay: %f\n", sampst(0.0, -SAMPST_DELAYS));
	
	printf("Average throughput time: %f\n", sampst(0.0, -THROUGHPUT_TIME));
	printf("Min throughput time: %f\n", transfer[4]);

}

void push_array() {

	memcpy(temp_transfer,transfer,TRANSFER_ARRAY_LENGTH*sizeof(float));  
}

void pop_array() {
	memcpy(transfer,temp_transfer,TRANSFER_ARRAY_LENGTH*sizeof(float));  
}

