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
//#define LOADING_TIME_PER_SKAUT 

// Global variables
int number_of_machines, min_productivity, min_no_failures, max_no_failures, skaut_throughput;
float mean_wagen_arrival, std_wagen_arrival, mean_failures, std_failures, min_machine_repair_time, max_machine_repair_time, end_warmup_time, end_simulation_time; 

int is_machine_busy[NUM_MACHINES +1],
	queue_size[NUM_MACHINES +1];
	
float work_time[NUM_MACHINES + 1],
	  transfer_time[NUM_MACHINES +1]; // +1 is the less preferable simlib indexing scheme

FILE *infile, *outfile;

/* Function signatures */

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
        // initialize arrays and variables
        memset( is_machine_busy,0, NUM_MACHINES +1 );
        skaut_throughput = 0;
		
		int b;
		for (b=1; b <= number_of_machines; b++) {
			printf("transfer_time[%d] = %f\n", b,transfer_time[b] );
		}
	
        // We perform simulation for "a few" failures per day
	int i;
	for (i = min_no_failures; i < max_no_failures; i++) {
		// Initialize rndlib
		init_twister();
	
		// Initialize simlib
		init_simlib();
		maxatr = 4; // how many attributes do we need?
		
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

void wagen_unload_arrivalOLD()
{
	int i;
	
	for (i = 1; i <= WAGEN_LOAD; i++) {
		transfer[3]=1.0;
		//transfer[4] = 9.0;
		event_schedule( sim_time + (i * work_time[1]), EVENT_SKAUT_DEPARTURE );
	}

	// DO SIMLIB STATISTICS?
	event_schedule( sim_time + 60*30 , EVENT_WAGEN_UNLOAD_ARRIVAL );

}

void wagen_unload_arrival()
{
	
	/*	when the wagen arrives, 14 skaut arrivals are fired with current location (transfer[3]) = 1.0.
		this means that we are able to handle statistics also for machine 1. Machine 1 has a queue of size 14, which is the
		wagen load.
	*/
	
	int i;
	for (i=1; i <= WAGEN_LOAD; i ++) {
		/*	transfer[3] = 1.0; transfer[3] can be 0 here, because there is no current location, they are on der wagen!!! skaut_arrival then
			increments transfer[3], which is normal. 
		*/
		event_schedule( sim_time + (i*0.1),	EVENT_SKAUT_ARRIVAL);
	}
	
	// schedule next wagen arrival
	event_schedule(sim_time + 30*60, EVENT_WAGEN_UNLOAD_ARRIVAL);
}


void skaut_arrivalOLD()
{
	int current_unit = (int) ++transfer[3];
	
	//if (is_machine_busy[current_unit]) { // machine is busy
	if (list_size[current_unit] == 1) { // machine is busy	
		if (list_size[number_of_machines + current_unit] == queue_size[current_unit]) { // machine busy, row full
			//queue_is_full();
		} 
                else { // machine busy, row not full
			list_file(LAST, number_of_machines + current_unit); // skaut appended to queue
                        transfer[3] = (float) current_unit;
                        //	printf("Appended to row %d\n", current_unit);
		}
	
	} 
        else { // machine is not busy
                list_file(LAST, current_unit); // skaut put in machine 
		is_machine_busy[current_unit] = 1; // machine is busy
		sampst(0.0, current_unit); // the delay is zero
		//printf("Scheduling event 4 from skaut_arrival, current_unit = %d\n", current_unit);
		transfer[3] = (float) current_unit;
		event_schedule( sim_time + work_time[current_unit], EVENT_SKAUT_DEPARTURE);
	}
}


void skaut_arrival() 
{
	int current_unit = (int) ++transfer[3]; // this then increments skauts from the wagen!!
	
	// check if machine is not busy
	if (list_size[current_unit] == 0) {
		// put skaut in machine
		list_file(FIRST, current_unit); // last := first here because there are only to be 0 or 1 items in machine
		// schedule departure after machine processing time
		transfer[3] = (float) current_unit;
		event_schedule(sim_time + work_time[current_unit], EVENT_SKAUT_DEPARTURE);
	} else {
		// check if queue is full
		if (list_size[number_of_machines + current_unit] == queue_size[current_unit]) {
			printf("BOOM! UNIT %d exploded with %d items!\n", current_unit, list_size[number_of_machines + current_unit]);
			// execution for this function call ends here
		} else {
			// put skaut in the queue
			list_file(LAST, number_of_machines + current_unit);
		}

	}


}

void skaut_departureOLD()
{
	int current_unit = (int)transfer[3];

	if (current_unit != 1) {
          list_remove(FIRST, current_unit);
        }

	if (current_unit == MACHINES_ON_THE_LEFT_SIDE) {	//last machine on left side, so the skaut goes into the skautaskali
		skaut_throughput += 2;
	} 
        else {
		event_schedule(sim_time + transfer_time[current_unit], EVENT_SKAUT_ARRIVAL);
		transfer[3] = (float) current_unit;
	}

	if (list_size[number_of_machines + current_unit] == 0) {
		is_machine_busy[current_unit] = 0;
		// STATISTICS
	} 
        else {
          list_file(LAST, current_unit); // skaut appended to machine's queue
          transfer[3] = (float) current_unit;
	  list_remove(FIRST, number_of_machines + current_unit);  //get skaut from queue and process it
          event_schedule(sim_time + work_time[current_unit], EVENT_SKAUT_DEPARTURE);
	}
}

void skaut_departure()
{
	int current_unit = (int) transfer[3];
	
	// is the station the skaut is departuring from the last station on the left side?
	if(current_unit == MACHINES_ON_THE_LEFT_SIDE) {
		skaut_throughput +=2;
	} else { 
		// chedule arrival time at the next station
		event_schedule(sim_time + transfer_time[current_unit], EVENT_SKAUT_ARRIVAL);
	}
	// remove this skaut from the machine
	list_remove(LAST, current_unit);
	// restore transfer[3] because list_remove overwrites it
	transfer[3] = (float) current_unit;
	
	// are there any skauts in the machine's queue that need some processing?
	if (list_size[number_of_machines + current_unit] != 0) {
		list_remove(FIRST, number_of_machines + current_unit); // aafter this line, transfer is overwritten with the values of the first skaut's in queue.
		// transfer[3] should therefor contain the correct station number, which is in fact current_unit
		list_file(current_unit, LAST);
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
}


