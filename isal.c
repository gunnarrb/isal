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
#include <time.h>
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
#define EVENT_GENERATE_FAILURES	9

// STREAMS
#define STREAM_WAGEN_ARRIVAL 1

//Other constants
#define NUM_MACHINES 7
#define SHIFT_LENGTH 57600.0;
#define WAGEN_LOAD 14
#define MACHINES_ON_THE_LEFT_SIDE 5
#define MACHINES_ON_THE_RIGHT_SIDE 2
#define OPTIMAL_THROUGHPUT 52
#define ACTUAL_THROUGHPUT 40
#define TRANSFER_ARRAY_LENGTH 11
#define PREP_TIME 0.0

typedef struct
{
    float failtime;
    float downtime;
    int machine_nr;
} breakdown;


//#define LOADING_TIME_PER_SKAUT 

// Global variables
int number_of_machines, min_productivity, min_no_failures, max_no_failures, skaut_throughput;
float mean_wagen_arrival, std_wagen_arrival, mean_failures, std_failures, min_machine_repair_time, max_machine_repair_time, end_warmup_time, end_simulation_time; 


int sampst_delays, throughput_time; // variable for queue delays and throughput time
time_t dummy;
unsigned int skaut_id, stream, failure_nr;
int queue_size[NUM_MACHINES +1], queue_max_lengths[NUM_MACHINES + 1];
float machine_broken[NUM_MACHINES +1];
breakdown *fail_list;	
int fail_index;

int is_machine_busy[NUM_MACHINES +1],
    queue_size[NUM_MACHINES +1];

float work_time[NUM_MACHINES + 1],
    transfer_time[NUM_MACHINES +1]; // +1 is the less preferable simlib indexing scheme


float temp_transfer[TRANSFER_ARRAY_LENGTH];

FILE *infile, *outfile;

/* Function signatures */

// Usage:	create_machine_fail_events()
// Pre:		init_twister must be called for random number generation
// Post:	scheduled events have been created for machines 
void create_machine_fail_events();


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


int main()
{
    // load datafiles
    parse_input("adal_inntak.in","velar_og_bidradir.in");
	
    // initialize arrays and variables
    if((fail_list = malloc(sizeof(breakdown)*NUM_MACHINES+1))==NULL) {
	printf("Allocation Error\n");
	exit(1);
    }



    int b;
/*    for (b=1; b <= number_of_machines; b++) {
      printf("transfer_time[%d] = %f\n", b,transfer_time[b] );
      printf("busy %d broken %f \n",is_machine_busy[b],machine_broken[b]);
      }*/
    // We perform simulation for "a few" failures per day
   
    for (failure_nr = min_no_failures; failure_nr<= max_no_failures; failure_nr++) {
	stream = (unsigned int)time(NULL) % 100;   
		
	memset( is_machine_busy,0, NUM_MACHINES +1 );
	memset( machine_broken,0, NUM_MACHINES +1);
	memset( queue_max_lengths,0, NUM_MACHINES +1);
	memset( fail_list,0, sizeof(breakdown)*(NUM_MACHINES+1));
	fail_index = 0;
	skaut_throughput = 0;
	sampst_delays = number_of_machines +1;
	throughput_time = number_of_machines +2;
		
	skaut_id = 1;
	skaut_throughput = 0;
		
		
	// Initialize rndlib
	init_twister();
		
	// Initialize simlib
	init_simlib();
		
	maxatr = 6; // how many attributes do we need?
		
	/* Schedule first wagen arrival */
	event_schedule( 10.0, EVENT_WAGEN_UNLOAD_ARRIVAL );
		
	/* Schedule end of warmup time */
	event_schedule( end_warmup_time, EVENT_END_WARMUP );
	event_schedule(end_warmup_time, EVENT_GENERATE_FAILURES );	
	/* Schedule simulation termination */
	event_schedule(end_simulation_time , EVENT_END_SIMULATION );
		
	next_event_type = 0;
		
        
	while (next_event_type != EVENT_END_SIMULATION) {
			
	    timing();
			
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
	    case EVENT_GENERATE_FAILURES:
		create_machine_fail_events();
		break;
					
	    }
	}
		
    }
}



void wagen_unload_arrival()
{
	
    int i;
    int current_unit = 0;
    float wagen_arrival_zeit = unirand((mean_wagen_arrival-std_wagen_arrival)*60.0,(mean_wagen_arrival+std_wagen_arrival)*60.0,stream); 
	
    for (i = 1; i<NUM_MACHINES+1; i++) {  //delay unload of skaut by the time it takes to repair
	if (machine_broken[i] > 0.0) {
	    event_schedule(sim_time + machine_broken[i], EVENT_WAGEN_UNLOAD_ARRIVAL);
	    return;
	}
    }
	
    if (list_size[number_of_machines + 1] != 0) { // ef allt er enn fullt þá koma með næsta vagn eftir uþb hálftíma
	event_schedule(sim_time + wagen_arrival_zeit, EVENT_WAGEN_UNLOAD_ARRIVAL);
	return;
    }
    
    int vagn_magn =  WAGEN_LOAD-((int)unirand(0.0,3.0,stream));  //12 - 14 skaut á hverjum vagni
    for (i=1; i <= vagn_magn; i++) {
		
	transfer[3]=1.0;
	transfer[4] = sim_time + (i * 0.01); // skaut entering system time
	transfer[6] = (float) skaut_id++;
	//printf("tr4 in wagen: %f\n", transfer[4]);
	event_schedule( sim_time + ( i* 0.01),	EVENT_SKAUT_ARRIVAL);
    }
	
    event_schedule(sim_time+wagen_arrival_zeit, EVENT_WAGEN_UNLOAD_ARRIVAL);
}


void skaut_arrival() 
{
    push_array();
    int current_unit = (int)transfer[3];
    int i;
	
    for (i = NUM_MACHINES; i>=current_unit; i--) {  //add delay if there is a broken machine before current one

	if (machine_broken[i] > 0.0) {
	    if ((list_size[1+number_of_machines + current_unit] < queue_size[1+current_unit])||queue_size[1+current_unit] == 0) {  // if current machine is broken then delay it.x
		event_schedule(PREP_TIME+sim_time +  machine_broken[i] + work_time[current_unit], EVENT_SKAUT_ARRIVAL); //also if next queue is full then delay it.
		return;
	    }
	}
    }
	
    // check if machine is not busy
    if (list_size[current_unit] == 0 && machine_broken[current_unit] == 0.0) {
	sampst(0.0, sampst_delays);
	sampst(0.0, current_unit);
	    
	list_file(FIRST, current_unit); // last := first here because there are only to be 0 or 1 items in machine
		
	// schedule departure after machine processing time
	pop_array();
	event_schedule(PREP_TIME + sim_time + work_time[current_unit], EVENT_SKAUT_DEPARTURE);
    } else {
		
	if (list_size[number_of_machines + current_unit] == queue_size[current_unit]) {
			
	    event_schedule(PREP_TIME + sim_time + work_time[current_unit], EVENT_SKAUT_ARRIVAL); //also if queue is full then delay it.
			
	} else {
	    transfer[5] = sim_time;
	    list_file(LAST, number_of_machines + current_unit);
	    if(list_size[current_unit] > queue_max_lengths[number_of_machines + current_unit]) {
		queue_max_lengths[current_unit] = list_size[number_of_machines + current_unit];
	    }
	}
		
    }
	
}

void skaut_departure()
{
    push_array();
    int current_unit = (int) transfer[3];
    int i = 0;
    for (i = NUM_MACHINES; i>=current_unit; i--) {  //add delay if machine is broken or there is a broken machine before current one

	if (machine_broken[i] > 0.0) {
	    if ((i == current_unit)  || (list_size[1+number_of_machines + current_unit] < queue_size[1+current_unit])) {  // if current machine is broken then delay it.
		event_schedule(PREP_TIME+sim_time + machine_broken[i], EVENT_SKAUT_DEPARTURE); //also if next queue is full then delay it.
		return;
	    }
//	    printf("Size of next queue %d, limit of next queue %d\n",list_size[1+number_of_machines + current_unit], queue_size[1+current_unit]);
	    break;
	}
    }
	
    if (current_unit == MACHINES_ON_THE_LEFT_SIDE) {
	skaut_throughput += 2;
	sampst(sim_time - transfer[4], throughput_time);
	list_remove(FIRST,current_unit);  
    } else {
	list_remove(FIRST,current_unit);
	pop_array();
	transfer[3]++;
	event_schedule(PREP_TIME + sim_time + transfer_time[(int)(transfer[3])-1], EVENT_SKAUT_ARRIVAL);
    }
	
	
    if (list_size[number_of_machines + current_unit] != 0) {
	pop_array();
		
	list_file(FIRST,current_unit); // first equals last because size should only be 1
	pop_array();
		
	list_remove(FIRST, number_of_machines + current_unit);
	pop_array();
		
	sampst(sim_time - transfer[5], sampst_delays);
	sampst(sim_time - transfer[5], current_unit);
	event_schedule(PREP_TIME + sim_time + work_time[current_unit], EVENT_SKAUT_DEPARTURE);
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
    skaut_throughput = 0;
}

void report()
{
	
    int i;
    float total_downtime = 0.0;
    printf("\n*****************************************************\n");
    printf("Report for %d failures per day\n",failure_nr);
    
    for (i=0; i <NUM_MACHINES; i++) {
	printf("--Breakdown in machine nr %d--\n", i+1);
	printf("Number of fails\t Downtime \t\n");
	printf("\t %d\t", fail_list[i].machine_nr);
	printf("%.3f sec / %.3f min\t", fail_list[i].downtime,fail_list[i].downtime/60.0);
	printf("\n");
	total_downtime+=fail_list[i].downtime;
    }
    printf("\n\n");
    
	
    printf("Total downtime was %.3lf seconds or %.3lf minutes\n",total_downtime, total_downtime/60.0);
    
    printf("--------------\nMachine load\n--------------\n");
    for (i=1; i <= number_of_machines; i++) {
	printf("Machine %d\t", i);
    }
    printf("\n");
    for (i=1; i <= number_of_machines; i++) {
	printf("%f\t", filest(i) );
    }
    printf("\n\n");
	
    printf("-----------------------\nAverage delay in queues\n-----------------------\n");
    for (i=1; i <= number_of_machines; i++) {
	printf("Queue %d \t", i);
    }
	
    printf("\n");
	
    for (i=1; i <= number_of_machines; i++) {
	printf("%f\t", sampst(0.0, -i));
    }
    printf("\n\n");
    printf("Average queue delay: %f\n", sampst(0.0, -sampst_delays));

    printf("Worst case queue delay: %f\n", transfer[3]);
    printf("Best case queue delay: %f\n", transfer[4]);

    printf("System throughput: %d\n", skaut_throughput );	
    printf("Average throughput time: %f\n", sampst(0.0, -throughput_time));
    printf("Min throughput time: %f\n", transfer[4]);
    printf("Random seed: %d\n\n", stream);
	
    int l;
    int sum_q_lenths =0;
    int number_of_queues =0;
    for (l = 1; l <= number_of_machines; l++) {
	if (queue_size[l] < 1) continue;
	printf("Maximum length of queue %d: %d\n", l, queue_max_lengths[l]);
	sum_q_lenths += queue_max_lengths[l];
	number_of_queues++;
    }
	
    printf("Average maximum length of queues: %f\n\n", (float) sum_q_lenths / (float) number_of_queues);
}

void push_array() {
	
    memcpy(temp_transfer,transfer,TRANSFER_ARRAY_LENGTH*sizeof(float));  
}

void pop_array() {
    memcpy(transfer,temp_transfer,TRANSFER_ARRAY_LENGTH*sizeof(float));  
}

void create_machine_fail_events() {
    int i;
    float a[20],shift_length;
    shift_length = (float)SHIFT_LENGTH;
    int n = failure_nr;
    memset(a,0,20*sizeof(float));
    float span = shift_length / (float)n+1.0;  //max time between machine failures
    float current_span = 0.0;
    int machine;
    float repair_time ;
    float breakdown_time;
	
    for (i = 0;i<n;i++) {

	current_span+=span;
	machine = (int)unirand(1,number_of_machines+1,stream);
	breakdown_time = unirand(0.0,current_span,stream);
	repair_time =(min_machine_repair_time + expon(max_machine_repair_time ,stream))*60.0;
	if (a[machine]<breakdown_time) {  // 
	    a[machine] = breakdown_time+repair_time;
	}
	else { // if breakdown_time clashes with the same machine then let the breakdown happen after the machine goes up again
	    breakdown_time = a[machine] + 1.0;
	    a[machine] = breakdown_time+repair_time;
	}
	transfer[3] = repair_time;
	transfer[4] = (float)machine;
	fail_list[machine-1].downtime+= repair_time;
	fail_list[machine-1].machine_nr++; 
	event_schedule(sim_time + breakdown_time, EVENT_MACHINE_FAILURE );
    }
	
    event_schedule(sim_time + shift_length, EVENT_GENERATE_FAILURES );
}

void machine_failure(){	
    float repair_time = transfer[3];
    int   machine     = (int)transfer[4];
    machine_broken[machine] = repair_time;
    //    printf(" Machine %d broke down and it takes %f to repair\n", machine, repair_time/60.0);
	
    event_schedule(sim_time + repair_time, EVENT_MACHINE_FIXED);
}

void machine_fixed(){
	
    int   machine     = (int)transfer[4];
    machine_broken[machine] = 0.0;
}
