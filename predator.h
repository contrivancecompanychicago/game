#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdint.h>
#include <time.h>

#include <gsl/gsl_randist.h>
#include <gsl/gsl_rng.h>

#include "useful_math.h"

typedef uint64_t num_type;

typedef enum strategy_t{
	undef, /*default case - not really in the game */
	synergy,
	competition,
	ignore
}strategy_t;

typedef struct sample_events{
	unsigned gen;
	unsigned num;
	struct sample_events * next;
}sample_events;

typedef struct predator{
	double fitness;
	strategy_t strategy;

	unsigned aggro; /* number of '1' the parts of the genotype infuencing the strategy contain */
	double xaxis;
	double yaxis;

	num_type * geno;  /* a pattern of bases which determines the strategy of the predator */
	//char * migr; /* a patten of bases which determines the migration rate of the predator */

}predator;

typedef struct generation{
	predator * pred; /* array of predators in the current generation */
	unsigned num;			/* number of said predators */
}generation;

typedef struct bottle{
	unsigned gen;
	unsigned preds;
	struct bottle * next;
}bottle;

double mean(double x, double y);

void print_genotype(predator * p);
predator * recombine(predator * p, predator * parent1, predator * parent2);
predator * set_geno(predator * p, predator * parent1, predator * parent2);

predator * set_position(predator * p, predator * parent1, predator * parent2);

unsigned find_parent(double fit);
predator * choose_parents(predator * p);

void add_predator(unsigned num);
void reproduce();

void print_predators(unsigned gen, unsigned curr_gen);

void init_predator();
