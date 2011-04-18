/* This is rndlib.h. */

/* Include files. */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

/* Declare rndlib functions. */

/* Initialize the Twister *MUST* be called once in the start */
extern void init_twister(void);
/* Exponential variate generation function. */
extern double expon (double mean, unsigned long int stream);
/* Discrete-variate generation function. */
extern int intrand (double prob_distrib[], unsigned long int stream);
/* Uniform variate generation function on [a b]. */
extern double unirand (double a, double b, unsigned long int stream);
/* Uniform variate generation function on [0 1]. */
extern double urand (unsigned long int stream);
/* Erlang variate generation function. */
extern double erlang (int m, double mean, unsigned long int stream);
/* Gaussian random variable N(0,1). */
extern double nrand (unsigned long int stream);
/* Gamma distribution with shape parameter $a$ */
extern double grand (double a, unsigned long int stream); 
