void parse_input(char inputfile_data[], char inputfile_time[], char outputfile[])
{
  infile = fopen (inputfile_data, "r");
  outfile = fopen (outputfile, "w");
  
 
  /* Read input parameters. */
  fscanf (infile, " %d %d %d %d %f %f %f %f %f %f %f %f %f", &number_of_machiens, &min_productivity, &min_no_failures, &max_no_failures, &mean_wagen_arrival, &std_wagen_arrival, &mean_failures, &std_failures, &min_machine_repair_time, &max_machine_repair_time, &end_warmup_time, &end_simulation_time);
  fclose (infile);
  infile = fopen (inputfile_time, "r");
  
  int counter = 1;
  while (!feof(infile)) {
    fscanf(infile, "%f %d", &work_time[counter], &queue_size[counter] );
    counter++;
  }
  
  close(infile);


}
