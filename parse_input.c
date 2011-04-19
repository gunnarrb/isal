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
