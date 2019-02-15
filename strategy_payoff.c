#include "strategy_payoff.h"

/* ------------------ */
float synergy_gain = 0.4;
float synergy_loss = 0.2;
float synergy_exploit = 0.8;
/* ------------------ */
float comp_exploit = 0.4;
float comp_loss = 0.3;
float antag_loss = 0.9;
/* ------------------ */
double genes = 0.3;
float variance = 0.1; /* variance from the inherited to the actually strategy of each predator ~ default is no variance */

unsigned social_choices[3];
float payoff_matrix[3];
float tot_fit;

extern unsigned phenotype_size;
extern float com_rate;
extern float syn_rate;

extern unsigned pred_num; //will be removed later
extern gsl_rng * r;

strategy_t choose_strategy(char * pheno){ /* chooses the strategy each predator will follow */

	unsigned i, counter;
	counter = 0;
	for (i = 0; i < phenotype_size; i++){
  	if (pheno[i] == 1)
  		counter++;
	}
	double ph = (double)counter/(double)phenotype_size;
	double flex_pheno = ph + gsl_ran_gaussian(r, variance);
	if (flex_pheno > com_rate)
		return competition;
	if (flex_pheno < syn_rate)
		return synergy;
	return ignore;
}

char choose_mig_policy(predator * p, strategy_t social){
	double random = rand() / (float)RAND_MAX;
	if (random < 0.3 && social != competition)
		return 'm';
	else if (random < 0.35 && social != competition && p -> strategy != competition) /* a non-competitive entity has a slightly higher chance to "flee" from a competitive environment */
		return 'm';
	return 'r';
}

/* -------------------------------------------------------------------------------------- */

unsigned reprofunction(){
	/* needs to be something more dynamical if to be considered an option */
	unsigned sum = social_choices[0] + social_choices[1] + social_choices[2];
	return sum;
}


float synergy_payoff(unsigned prey_val){
	if (!social_choices[0]) /* no people so no payoff */
		return 0;
	unsigned player_sum = social_choices[0] + social_choices[1] + social_choices[2];
	/* we first calculate the profit from synergizing */
	float payoff = prey_val	+ (float)social_choices[0] * synergy_gain;
	/* we then substract the slight loss from those ignoring the "synergy offer" */
	payoff -= (float)social_choices[1] * synergy_loss;
	/* Now we substract the severe loss from the competitive ones */
	payoff -= (float)social_choices[2] * synergy_exploit;
	/* finally we divide the final payoff by the number of lone predators, since they split it equally */
	payoff = payoff/(float)social_choices[0];
	return payoff;
}

float competition_payoff(unsigned prey_val){
	if (!social_choices[1]) /* no people so no payoff */
		return 0;
	unsigned player_sum = social_choices[0] + social_choices[1] + social_choices[2];
	/* first we add the payoff gained by expoiting the ones that try to synergize */
	float payoff = prey_val;
	payoff += (float)social_choices[1] * comp_exploit;
	/* we then add the small gain from the lone predators */
	payoff += (float)social_choices[2] * comp_loss;
	/* we now reduce the penalty from each competitor antagonizing with each other */
	payoff -= (float)social_choices[1] * antag_loss;
	/* we know split the payoff to each competitor */
	payoff = payoff/(float)social_choices[1];
	return payoff;
}

float ignore_payoff(unsigned prey_val){ /* we calculate the payoff for each player choosing the ignore strategy on a certain game set */
	if (!social_choices[2]) /* no people so no payoff */
		return 0;
	unsigned player_sum = social_choices[0] + social_choices[1] + social_choices[2];
	float payoff = prey_val;
	/* first, we add the payoff gained from other trying to be synergetic - which is equal to what these players lose for "failing" to synergize */
	payoff += (float)social_choices[0] * synergy_loss;
	/* then we subtract the payoff lost due to competitors */
	payoff += (float)social_choices[1] * comp_loss;
	/* finally we divide the final payoff by the number of lone predators, since they split it equally */
	payoff = payoff / (float)social_choices[2];
	return payoff;
}

/* returns to each player the payoff they get from playing the game */
void grant_payoff(prey * p){
	if (p -> ng == NULL){
		fprintf(stderr,"payoff %u %u\n",p->id,p->num);
		return;
	}
	payoff_matrix[synergy -1] = synergy_payoff(p -> value);
	payoff_matrix[competition - 1] = competition_payoff(p -> value);
	payoff_matrix[ignore-1] = ignore_payoff(p -> value);
	tot_fit = payoff_matrix[0] * social_choices[0] + payoff_matrix[1] * social_choices[1] + payoff_matrix[2] * social_choices[2];

	double tot_payoff = 0.0; /* payoff given to predators thus far */
	unsigned i;
	for (i = 0; i < p -> num; i++){
		tot_payoff += payoff_matrix[p -> ng -> pr -> strategy -1];
		p -> ng -> fitness = tot_payoff;
	}
	// fprintf(stderr, "%f %f %f\n", payoff_matrix[0], payoff_matrix[1], payoff_matrix[2]);
	// fprintf(stderr,"Total fitness: %f\n", tot_fit);
}

/* μία λογική σκέψη είναι να έχει ένα κόστος η μετανάστευση με την έννοια του χρόνου άφιξης στον εκάστοτε προορισμό. Να παίρνει πχ ο predator 90% του Payoff
που το αντιστοιχεί κι όχι το πλήρες */
