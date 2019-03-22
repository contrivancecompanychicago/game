#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <time.h>

#include <gsl/gsl_randist.h>
#include <gsl/gsl_rng.h>

#include "prey.h"

typedef struct genome_sample{
  unsigned gen; //not sure if needed
  strategy_t strategy; //not sure if needed
	num_type * geno;
}genome_sample;

void sampling(unsigned num);

void print_binary( num_type number, unsigned counter, unsigned max, FILE * out_file);

void ms_output();
void vcf_output();
