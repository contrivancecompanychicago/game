#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <time.h>

#include <gsl/gsl_randist.h>
#include <gsl/gsl_rng.h>

#include "prey.h"

strategy_t choose_strategy(unsigned aggro);

char choose_mig_policy(predator * p, strategy_t social);

float synergy_payoff(unsigned prey_val);

float competition_payoff(unsigned prey_val);

float ignore_payoff(unsigned prey_val);

void grant_payoff(prey * p);
