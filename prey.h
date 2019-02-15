#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <time.h>

#include <gsl/gsl_randist.h>
#include <gsl/gsl_rng.h>

#include "predator.h"

typedef struct prey_pos{
	double x;
	double y;
	struct prey_pos * next;
}prey_pos;

typedef struct prey{
	unsigned id;
	unsigned value;
	unsigned num;
	float xaxis;
	float yaxis;
	next_gen * ng; /* predators which played the game and will thus form the next generation */
}prey;

typedef struct trigger{
	unsigned entry_time;
	unsigned prey_num;
	struct trigger * next;
}trigger;

//void find_in_range(prey * p, unsigned latitude, unsigned longtitude, unsigned gen);
void find_in_range(prey * p, unsigned gen);

void add_prey();

void enable_trigger();

void set_new_trigger();

void remove_prey(unsigned pid);

void deplete_prey(prey * p);
