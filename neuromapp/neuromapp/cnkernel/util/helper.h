#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>


void print_usage() {
    printf("Usage: cnkernels --mechanism [string] --function [string] --data [string] --numthread [int] \n");
    printf("Details: \n");
    printf("                 --mechanism [Na, Synapse or Ih] \n");
    printf("                 --function [state or current] \n");
    printf("                 --data [path to the input] \n");
    printf("                 --numthread [threadnumber] \n");
    exit(1);
}

struct input_parameters{
    char * m; // mechanism
    char * f; // function state/current
    char * d; // data set
    int th; // number of thread
};

int help(int argc, char* argv[], struct input_parameters * p)
{
  int c;

  p->m = "Na"; // default
  p->f = "state"; // default
  p->d = "";
  p->th = 1; // one omp thread by default

  while (1)
  {
      static struct option long_options[] =
      {
          {"help", no_argument, 0, 'h'},
          {"mechanism", required_argument, 0, 'm'},
          {"function",  required_argument, 0, 'f'},
          {"data",  required_argument,     0, 'd'},
          {"numthread",  required_argument,0, 'n'},

          {0, 0, 0, 0}
      };
      /* getopt_long stores the option index here. */
      int option_index = 0;

      c = getopt_long (argc, argv, "m:f:d:",
                       long_options, &option_index);
      /* Detect the end of the options. */
      if (c == -1)
        break;

      switch (c)
      {
          case 'm': p->m = optarg;
              break;
          case 'f': p->f = optarg;
              break;
          case 'd': p->d = optarg;
              break;
          case 'n': p->th = atoi(optarg);
              break;
          case 'h':
              print_usage();
              break;
          default:
              print_usage ();
	      break;
      }
  }

  return 0 ;


}
