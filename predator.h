#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include <time.h>

#include <gsl/gsl_randist.h>
#include <gsl/gsl_rng.h>

#include "useful_math.h"
//#include "prey.h"

typedef enum strategy_t{
	undef, /*default case - not really in the game */
	synergy,
	competition,
	ignore
}strategy_t;

typedef struct predator{
	int prid;				/* unique id for each predator in a generation */
	double fitness;

	char * pheno;  /* a pattern of bases which determines the strategy of the predator */
	//char * migr; /* a patten of bases which determines the migration rate of the predator */


	strategy_t strategy;

	unsigned height;

	int rec; /* whether the phenotype of the offspring is a product of recombination */
	unsigned flag; /* number of offspings the predator has */
	double xaxis;
	double yaxis;

	struct predator * parent1; /* parent of the predator */
	struct predator * parent2; /* parent of the predator */

	struct predator * pnode; /* parental tree node, NOT parent of the predator */
	struct predator * lc;
	struct predator * rc;
}predator;

typedef struct next_gen{
	predator * pr;
	float fitness;
	struct next_gen * next;
}next_gen;

typedef struct ram{
	unsigned cram;
	struct ram * next;
}ram;

void init_pred();


//void find_in_range(unsigned latitude, unsigned longtitude, predator * root);

int get_height(predator *pr);

void rotate_left(predator * pr);

void rotate_right(predator * pr);

int check_balance(predator * pr);

void balance(predator * pr);

void add_predator(predator * p, predator * tmp);

predator * find_predator(unsigned prid, unsigned eucleidian, unsigned latitude, predator * tmp);

predator * next_inOrder(predator * p);

void remove_predator(unsigned prid, unsigned latitude, unsigned longtitude, unsigned gener);

void darwinism(unsigned gen);

void prune();

void free_tree(predator * root);
