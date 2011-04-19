void parse_input(char inputfile_data[], char inputfile_time[])
{


  if ((infile = fopen (inputfile_data, "r")) == NULL) {
    printf("Could not open file %s\n",inputfile_data);
  }
  
  fscanf (infile, "%d %d %d %d %f %f %f %f %f %f", &number_of_machiens, &min_productivity, &min_no_failures, &max_no_failures, &mean_wagen_arrival, &std_wagen_arrival,  &min_machine_repair_time, &max_machine_repair_time, &end_warmup_time, &end_simulation_time);
  fclose(infile);
  
  
  if ((infile = fopen (inputfile_time, "r")) == NULL) {
    printf("Could not open file %s\n",inputfile_time);
  } 
  /*  printf( "%d %d %d %d %f %f %f %f %f %f\n", number_of_machiens, min_productivity, min_no_failures, max_no_failures, mean_wagen_arrival, std_wagen_arrival,  min_machine_repair_time, max_machine_repair_time, end_warmup_time, end_simulation_time);*/

  int counter = 1;
  while (!feof(infile)) {
    fscanf(infile, "%f %d", &work_time[counter], &queue_size[counter] );
    counter++;
  }
  fclose(infile);

}


