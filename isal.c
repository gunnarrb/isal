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
int Number_of_machiens, Min_productivity, Min_no_failures, Max_no_failures;
float Mean_wagen_arrival, Std_wagen_arrival, Mean_failures, Std_failures, Min_machine_repair_time, Max_machine_repair_time, End_warmup_time, End_simulation_time;

int machine2queue[NUM_MACHINES +1], 
	is_machine_busy[NUM_MACHINES +1],
	queue_size[NUM_QUEUES +1];
	
float work_time[NUM_MACHINES + 1]; // +1 is the less preferable simlib indexing scheme
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

// Usage:	x = N(muy, sigma, stream);
// Pre:		muy and sigma are of type float
//			stream is of type int
// Post:	x is a random gaussian distributed variable of type float 
//			with mean muy and std sigma
float N(float muy, float sigma, int stream);

void generate_report();

int main()
{
	//parse_input("adal_inntak.in","velar_og_bidradir.in","output.out");
	
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
	
	
	
}

void parse_input(char inputfile_data[], char inputfile_time[], char outputfile[])
{
  infile = fopen (inputfile_data, "r");
  outfile = fopen (outputfile, "w");
  
  /* Read input parameters. */
  fscanf (infile, "%d %d %d %d %d %f %f %f %f %f %f %f %f %f",   &fjoldi_vela, &lengd_bidrada, &min_afkost_per_dag, &lagmarksfjoldi_bilanna_per_day, &hamarksfjold_bilanna_per_day, &vinnutimar_vela, &mean_wagen_arrival, &std_wagen_arrival, &mean_bilanir, &std_bilanir, &min_vidgerdartimi_vela, &max_vidgerdartimi_vela, &end_warmup_timi, &end_hermun_timi);

  fclose (infile);
  infile = fopen (inputfile_time, "r");
 
  int counter = 1;
  while (!feof(infile)) {
    fscanf(infile, "%f %d", &work_time[counter], &queue_size[counter] );
    counter++;
  }

  close(infile);
}

float N(float muy, float sigma, int stream)
{
	// This method of converting from N(0,1) to N(muy,sigma) has not been verified!
	float x = nrand(stream);
	return (x*sigma)+15;
}
