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

// EVENTS

#define EVENT_WAGEN_ARRIVAL	1
#define EVENT_WAGEN_DEPARTURE 2
#define EVENT_SKAUT_ARRIVAL 3
#define EVENT_SKAUT_DEPARTURE 4
#define EVENT_MACHINE_FAILURE 5
#define EVENT_MACHINE_FIXED 6
#define EVENT_END_SIMULATION 7
#define EVENT_END_WARMUP	8

//Other constants

#define NUM_MACHINES 7
#define NUM_QUEUES 5 // vid flokkum lasa einnig sem bidradir

// Global variables
int fjoldi_vela, lengd_bidrada, min_afkost_per_dag,lagmarksfjoldi_bilanna_per_day, hamarksfjold_bilanna_per_day;
float vinnutimar_vela,  mean_wagen_arrival, std_wagen_arrival, mean_bilanir, std_bilanir, min_vidgerdartimi_vela, max_vidgerdartimi_vela, end_warmup_timi, end_hermun_timi;

// What streams do we need?

/* Model variables */
int num_machines, num_queues;
int machine2queue[NUM_MACHINES +1], 
	is_machine_busy[NUM_MACHINES +1],
	queue_sizes[NUM_QUEUES +1];
	
float work_time[NUM_MACHINES + 1] // +1 is the less preferable simlib indexing scheme
FILE *infile, *outfile;

/* Function signatures */
void wagen_arrival();

void wagen_departure();

void skaut_arrival();

void skaut_departure();

// Usage:	parse_input(input_filename_data,input_filename_time, output_filename);
// Pre:		input_filename_data,input_filename_time output_filename are of type char[],
//			global variables from the input file exist.
// Post:	the global variables were assigned values from input_filename, 
//			the variables along with their values were written to output_filename
void parse_input(char[] ,char[], char[]);

void generate_report();

int main()
{
	parse_input("before_raising_electricity.in");
	// write to output file
	
	// Initialize rndlib
	init_twister();
	
	// Initialize simlib
	init_simlib();
	maxatr = 4; // how many attributes do we need?
	
}

void parse_input(char inputfile_data[], char inputfile_time[], char outputfile[])
{
  infile = fopen (inputfile_data, "r");
  outfile = fopen (outputfile, "w");
  
  /* Read input parameters. */

  fscanf (infile, "%d %d %d %d %d %f %f %f %f %f %f %f %f %f",   &fjoldi_vela, &lengd_bidrada, &min_afkost_per_dag, &lagmarksfjoldi_bilanna_per_day, &hamarksfjold_bilanna_per_day, &vinnutimar_vela, &mean_wagen_arrival, &std_wagen_arrival, &mean_bilanir, &std_bilanir, &min_vidgerdartimi_vela, &max_vidgerdartimi_vela, &end_warmup_timi, &end_hermun_timi);
);
  

}
