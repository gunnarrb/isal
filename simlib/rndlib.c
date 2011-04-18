/* 
  Thomas Philip Runarsson (email: tpr@hi.is) 
  Time-stamp: "2007-01-22 09:20:43 tpr"

  Modified for multiple streams based on older code by Jeff Deifik found here:
  http://www.math.sci.hiroshima-u.ac.jp/~m-mat/MT/VERSIONS/C-LANG/c-lang.html
*/

/* 
   A C-program for MT19937, with initialization improved 2002/1/26.
   Coded by Takuji Nishimura and Makoto Matsumoto.

   Before using, initialize the state by using init_genrand(seed)  
   or init_by_array(init_key, key_length).

   Copyright (C) 1997 - 2002, Makoto Matsumoto and Takuji Nishimura,
   All rights reserved.                          

   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions
   are met:

     1. Redistributions of source code must retain the above copyright
        notice, this list of conditions and the following disclaimer.

     2. Redistributions in binary form must reproduce the above copyright
        notice, this list of conditions and the following disclaimer in the
        documentation and/or other materials provided with the distribution.

     3. The names of its contributors may not be used to endorse or promote 
        products derived from this software without specific prior written 
        permission.

   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
   "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
   LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
   A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR
   CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
   EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
   PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
   PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
   LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
   NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
   SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.


   Any feedback is very welcome.
   http://www.math.sci.hiroshima-u.ac.jp/~m-mat/MT/emt.html
   email: m-mat @ math.sci.hiroshima-u.ac.jp (remove space)
*/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

/* Period parameters */  
#define N 624
#define M 397
#define MATRIX_A 0x9908b0dfUL   /* constant vector a */
#define UPPER_MASK 0x80000000UL /* most significant w-r bits */
#define LOWER_MASK 0x7fffffffUL /* least significant r bits */

#define	NUM_RNG	100 + 1		/* JTD */

/* 
   Set the default seeds for all 100 streams, seem main() below
   for method used to generate these seeds, spaced 1.000.000.000.
*/
unsigned long seedtwister[] = {5489UL,
1263627045UL, 4072462190UL,  801638233UL,  596572407UL, 1568620148UL,
3307885626UL, 1728445917UL, 3558695259UL,  672027896UL, 1446363660UL, 2981318205UL,
1507700002UL, 1492456090UL, 3085167704UL, 3500951483UL, 3483162123UL, 3034722587UL,
2829588004UL, 4247472284UL, 1839261138UL,  215668215UL,  707568411UL, 2224591914UL,
1293133163UL, 1763966846UL, 1653414111UL,  598513515UL, 1896751691UL, 2451325222UL,
1360412952UL,  204147348UL, 1614630052UL, 2704780403UL, 2475589660UL, 2060922129UL,
2360808056UL, 1694652355UL,   54591922UL, 2079818658UL, 1121376548UL, 2035157517UL,
3842014417UL, 3975075359UL, 1263229939UL, 1829672161UL, 3145896336UL, 1346916742UL,
3448955197UL, 3952156313UL,   65937246UL,  483886396UL, 3547883797UL, 1396378457UL,
  44197632UL, 3896181341UL, 2003787018UL, 2188241415UL, 1587488932UL,  714884487UL,
3054942729UL, 1795827299UL, 1985255906UL, 2738883830UL, 2098067776UL, 1246251158UL,
3159686355UL, 2298188348UL, 4013814915UL,  241658521UL,  968353435UL, 4198053454UL,
3492924117UL, 2337824961UL, 2575713569UL, 1861738854UL, 2652886546UL,  424497283UL,
2979577714UL,  825893190UL,  377507871UL, 3318425289UL, 3835584373UL, 3557877385UL,
1278083044UL, 2646500492UL, 1318376355UL, 3294378928UL, 1929089714UL, 3719883303UL,
4072087309UL,  945989092UL, 4279925794UL, 1723551385UL, 3199355493UL,  681168582UL,
2845320190UL,  464199453UL, 3167045080UL, 2516607905UL
};

static unsigned long mt[NUM_RNG][N]; /* the array for the state vector  */
/* static int mti=N+1;  mti==N+1 means mt[N] is not initialized */
static int mti[NUM_RNG]; /* mti[x]==N+1 means mt[x][N] is not initialized */

/* initializes mt[N] with a seed */
void init_genrand(unsigned long int rng_num, unsigned long s)
{
  if (rng_num >= NUM_RNG) {
    printf("MT19937AR: fatal error rng_num (%ld) is larger than NUM_RNG (%d)\n", rng_num, NUM_RNG);
    exit(1);
  }

  mt[rng_num][0] = s & 0xffffffffUL;
  for (mti[rng_num]=1; mti[rng_num]<N; mti[rng_num]++) {
    mt[rng_num][mti[rng_num]] = 
      (1812433253UL * (mt[rng_num][mti[rng_num]-1] ^ (mt[rng_num][mti[rng_num]-1] >> 30)) + mti[rng_num]); 
    /* See Knuth TAOCP Vol2. 3rd Ed. P.106 for multiplier. */
    /* In the previous versions, MSBs of the seed affect   */
    /* only MSBs of the array mt[].                        */
    /* 2002/01/09 modified by Makoto Matsumoto             */
    mt[rng_num][mti[rng_num]] &= 0xffffffffUL;
    /* for >32 bit machines */
  }
}

/* Call this before init_genrand. */
void init_twister(void)
{
  int i;
  
  for (i = 0; i < NUM_RNG; i++) {
    mti[i] = N+1;	/* mti[x]==N+1 means mt[x][N] is not initialized */
    init_genrand(i, seedtwister[i]);
  }
}

/* generates a random number on [0,0xffffffff]-interval */
unsigned long genrand_int32(unsigned long int rng_num)
{
  unsigned long y;
  
  static int first = 1;
  /*    static unsigned long mag01[2]={0x0UL, MATRIX_A};*/
  static unsigned long mag01[NUM_RNG][2];
  /* mag01[x] = x * MATRIX_A  for x=0,1 */
  
  /* Init mag01 */
  if (1 == first) {
    int i;
    for (i = 0; i < NUM_RNG; i++) {
      mag01[i][0] = 0x0UL;
      mag01[i][1] = MATRIX_A;
    }
    first = 0;
  }	
  
  if (mti[rng_num] >= N) { /* generate N words at one time */
    int kk;
    if (mti[rng_num] == N+1)   /* if init_genrand() has not been called, */
      init_genrand(rng_num, 5489UL); /* a default initial seed is used */
    
    for (kk=0;kk<N-M;kk++) {
      y = (mt[rng_num][kk]&UPPER_MASK)|(mt[rng_num][kk+1]&LOWER_MASK);
      mt[rng_num][kk] = mt[rng_num][kk+M] ^ (y >> 1) ^ mag01[rng_num][y & 0x1UL];
    }
    for (;kk<N-1;kk++) {
      y = (mt[rng_num][kk]&UPPER_MASK)|(mt[rng_num][kk+1]&LOWER_MASK);
      mt[rng_num][kk] = mt[rng_num][kk+(M-N)] ^ (y >> 1) ^ mag01[rng_num][y & 0x1UL];
    }
    y = (mt[rng_num][N-1]&UPPER_MASK)|(mt[rng_num][0]&LOWER_MASK);
    mt[rng_num][N-1] = mt[rng_num][M-1] ^ (y >> 1) ^ mag01[rng_num][y & 0x1UL];
    
    mti[rng_num] = 0;
  }
  
  y = mt[rng_num][mti[rng_num]++];
  
  /* Tempering */
  y ^= (y >> 11);
  y ^= (y << 7) & 0x9d2c5680UL;
  y ^= (y << 15) & 0xefc60000UL;
  y ^= (y >> 18);
  
  return y;
}

/* generates a random number on [0,0x7fffffff]-interval */
long genrand_int31(unsigned long int rng_num)
{
  return (long)(genrand_int32(rng_num)>>1);
}

/* generates a random number on [0,1]-real-interval */
double genrand_real1(unsigned long int rng_num)
{
  return genrand_int32(rng_num)*(1.0/4294967295.0); 
  /* divided by 2^32-1 */ 
}

/* generates a random number on [0,1)-real-interval */
double genrand_real2(unsigned long int rng_num)
{
  return genrand_int32(rng_num)*(1.0/4294967296.0); 
  /* divided by 2^32 */
}

/* generates a random number on (0,1)-real-interval */
double genrand_real3(unsigned long int rng_num)
{
  return (((double)genrand_int32(rng_num)) + 0.5)*(1.0/4294967296.0); 
  /* divided by 2^32 */
}

/* generates a random number on [0,1) with 53-bit resolution*/
double genrand_res53(unsigned long int rng_num) 
{ 
  unsigned long a=genrand_int32(rng_num)>>5, b=genrand_int32(rng_num)>>6; 
  return(a*67108864.0+b)*(1.0/9007199254740992.0); 
} 
/* These real versions are due to Isaku Wada, 2002/01/09 added */

/* Add random variates below */

/* Exponential variate generation function. */
double
expon (double mean, unsigned long int stream)
{
  return -mean * log (1.0 - genrand_real2 (stream));
}

/* Discrete-variate generation function. */
int
intrand (double prob_distrib[], unsigned long int stream)
{
  int i;
  double u;

  u = 1.0 - genrand_real2 (stream);

  for (i = 1; u >= prob_distrib[i]; ++i)
    ;
  return i;
}

/* Uniform variate generation function on [a b]. */
double
unirand (double a, double b, unsigned long int stream)
{
  return a + genrand_real1 (stream) * (b - a);
}

/* Uniform variate generation function on [0 1] */
double
urand (unsigned long int stream)
{
  return genrand_real1 (stream);
}

/* Erlang variate generation function. */
double
erlang (int m, double mean, unsigned long int stream)
{
  int i;
  double mean_exponential, sum;

  mean_exponential = mean / m;
  sum = 0.0;
  for (i = 1; i <= m; ++i)
    sum += expon (mean_exponential, stream);
  return sum;
}

/* 
   Gaussian random variable N(0,1) 
   Ratio method (Kinderman-Monahan); see Knuth v2, 3rd ed, p130
   K+M, ACM Trans Math Software 3 (1977) 257-260.
*/
double
nrand (unsigned long int stream)
{
  double u, v, x;

  do
    {
      v = urand (stream);
      do
	{
	  u = urand (stream);
	}
      while (u == 0);
      /* Const 1.715... = sqrt(8/e) */
      x = 1.71552776992141359295 * (v - 0.5) / u;
    }
  while (x * x > -4.0 * log (u));
  return (x);
}

/* Gamma distribution with shape parameter $a$ */
double grand (double a, unsigned long int stream) 
{
  double  d, c, x, v, u;

  d = a - 1.0 /3.0; 
  c = 1.0 / sqrt(9.0*d);
  for (;;) {
    do {
      x = nrand (stream);
      v = 1.0 + c * x;
      v = v * v * v; 
    } 
    while(v <= 0.0);
    u = urand (stream);
    if (u < 1.0 - 0.0331 * (x * x) * (x * x)) 
      return (d * v);
    if (log(u) < 0.5 * x * x + d * (1.0 - v + log(v)))
      return (d * v); 
  }
  return 0.0;
}

/* The following main is used to test distributions */
/*
int 
main (void)
{
  int i;
  init_twister();
  
  for (i = 0; i < 1000; i++) {
    printf("%10.8f  ", grand (2.0, 0));
    printf("%10.8f \n ", grand (2.0, 42));
  }
  return 0;
}
*/

/* The following main generates NUM_RNG seeds */
/*
  int 
  main(void)
  {
  unsigned long int j, i, ispace = 1000000000;
  unsigned long seed = 5489UL;
  
  init_twister();
  printf("static unsigned long seedtwister[] = {5489UL,\n");
  init_genrand(0, seed);
  for (j = 0; j < NUM_RNG; j++) {
    for (i = 0; i < ispace; i++) 
      genrand_int32(0);
    printf("%10luUL, ", mt[0][0]);
    if (j%6==5) printf("\n");
  }
  printf("\n};\n\n");
  return 0;
}
*/
