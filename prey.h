#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <time.h>
#include <stdint.h>

#include <gsl/gsl_randist.h>
#include <gsl/gsl_rng.h>

#include "predator.h"

/* an event regarding a change in preys that will take place during a certain generation */
typedef struct prey_event{
	unsigned ev_type; /* what type of event it is. 0 ~ remove ~ other == number of preys to add */
	unsigned gen;
	float xaxis;
	float yaxis;
	struct prey_event * next;
}prey_event;

/* sets a specific position for some preys */
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
	unsigned * pred_index; /* indexes of predators that will participate in the game */
}prey;

typedef struct trigger{
	unsigned entry_time;
	unsigned prey_num;
	struct trigger * next;
}trigger;

void add_next(prey * p, unsigned index, strategy_t strat);

void find_in_range(prey * p, unsigned gen);

void init_preys();

void enable_trigger();
void set_new_trigger();
void remove_prey(unsigned pid);
void deplete_prey(prey * p);

void print_preys();
void print_next(prey * p);
void free_prey();
