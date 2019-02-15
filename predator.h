#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include <time.h>

#include <gsl/gsl_randist.h>
#include <gsl/gsl_rng.h>

#include "useful_math.h"

typedef enum strategy_t{
	undef, /*default case - not really in the game */
	synergy,
	competition,
	ignore
}strategy_t;

typedef struct predator{
	int prid;				/* unique id for each predator in a generation */
	double fitness; // maybe we don't need fitness. Char play or no_play needed
	strategy_t strategy;

	int rec; /* whether the phenotype of the offspring is a product of recombination */
	unsigned flag; /* number of offspings the predator has */
	double xaxis;
	double yaxis;
	struct predator * parent1; /* parent of the predator */
	struct predator * parent2; /* parent of the predator */

	char * pheno;  /* a pattern of bases which determines the strategy of the predator */
	//char * migr; /* a patten of bases which determines the migration rate of the predator */

}predator;

typedef struct generation{
	predator * pred; /* array of predators in the current generation */
	unsigned num;			/* number of said predators */
}generation;

typedef struct next_gen{
	predator * pr;
	float fitness;
}next_gen;

typedef struct ram{
	unsigned cram;
	struct ram * next;
}ram;

void init_pred();

//void find_in_range(unsigned latitude, unsigned longtitude, predator * root);

//void find_in_range(unsign
int get_height(predator *pr);

void rotate_left(predator * pr);

void rotate_right(predator * pr);

int check_balance(predator * pr);

void balance(predator * pr);

//void add_predator(predator * p);

predator * find_predator(unsigned prid, unsigned eucleidian, unsigned latitude, predator * tmp);

//predator * next_inOrder(predator * p);

void remove_predator(unsigned gen, unsigned index);

unsigned darwinism(unsigned gen);

void prune();

void free_tree(predator * root);
