#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <time.h>

#include <gsl/gsl_randist.h>
#include <gsl/gsl_rng.h>

#include "strategy_payoff.h"

void play(prey * p);

void prep_game(prey * p);

void free_prev(unsigned prev_gen);

void game();
