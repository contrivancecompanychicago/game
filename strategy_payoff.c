#include "strategy_payoff.h"

/* ----------------------- */
float synergy_gain = 0.037;
float synergy_loss = 0.015;
float synergy_exploit = 0.065;
/* ----------------------- */
float comp_exploit = 0.055;
float comp_loss = 0.015;
float antag_loss = 0.02;
/* ----------------------- */
double genes = 0.3;
float variance = 0.05; 			/* variance from the inherited to the actually strategy of each predator ~ default is no variance */

unsigned social_choices[3];
float payoff_matrix[3];

extern unsigned num_inf;
float com_rate = 0.8;	 			/* determines below which threshold does the strategy become competitive */
float syn_rate = 0.2;	 			/* determines below which threshold does the strategy become synergetic */

extern generation * gens;
extern unsigned curr_flag;
extern gsl_rng * r;
extern short byte2bit;

num_type syn_mask;
num_type ign_mask;
num_type com_mask;

/* ----- different areas of the genotype effecting different strategies ----- */
void set_masks(){
	num_type all_ones = ~0;	/* complementary number of zero */
	unsigned s = syn_rate * sizeof(num_type) * byte2bit; /* how many '1's mask the synergy part */
	unsigned c = (1 - com_rate) * sizeof(num_type) * byte2bit; /* how many '1's mask the competition part */
  unsigned tot = __builtin_popcount(all_ones);
  num_type syn_mask, ign_mask, com_mask;
	syn_mask = all_ones << (tot - s);
	ign_mask = all_ones << s ^ syn_mask;
	com_mask = (1 << c) - 1;
}

strategy_t choose_strategy_mask(predator * p){
	unsigned syn_count, ign_count, com_count, i = 0;
	for (i = 0; i < num_inf; i++){
		syn_count += __builtin_popcount(syn_mask & p -> geno[i]);
		ign_count += __builtin_popcount(ign_mask & p -> geno[i]);
		com_count += __builtin_popcount(com_mask & p -> geno[i]);
	}
	unsigned sum = syn_count + ign_count + com_count;
	unsigned random = rand() % sum;
	if (sum < syn_count)
		return synergy;
	if (sum < ign_count)
		return ignore;
	return competition;
}

strategy_t choose_strategy(unsigned aggro){ /* chooses the strategy each predator will follow */
	double ph = (double)aggro/(double)(num_inf * sizeof(num_type) * byte2bit);
	double flex_geno = ph;// + gsl_ran_gaussian(r, variance);
	if (flex_geno < 0)
		flex_geno = 0;
	if (flex_geno > 1)
		flex_geno = 1;
	if (flex_geno >= com_rate)
		return competition;
	if (flex_geno < syn_rate)
		return synergy;
	return ignore;
}

// needs to change
char choose_mig_policy(predator * p, strategy_t social){
	double random = rand() / (float)RAND_MAX;
	if (random < 0.3 && social != competition)
		return 'm';
	else if (random < 0.35 && social != competition && p -> strategy != competition) /* a non-competitive entity has a slightly higher chance to "flee" from a competitive environment */
		return 'm';
	return 'r';
}

/* -------------------------------------------------------------------------------------- */

float synergy_payoff(unsigned prey_val){
	if (!social_choices[synergy - 1]) /* no people so no payoff */
		return 0;
	//unsigned player_sum = social_choices[0] + social_choices[1] + social_choices[2];
	/* we first calculate the profit from synergizing */
	float payoff = prey_val	+ ((float)social_choices[synergy - 1] - 1) * synergy_gain;
	/* we then substract the slight loss from those ignoring the "synergy offer" */
	payoff -= (float)social_choices[ignore - 1] * synergy_loss;
	/* Now we substract the severe loss from the competitive ones */
	payoff -= (float)social_choices[competition - 1] * synergy_exploit;
	/* finally we divide the final payoff by the number of lone predators, since they split it equally */
	payoff = payoff/(float)social_choices[synergy - 1];
	if (payoff < 0.001)
		payoff = 0.001;
	return payoff;
}

float competition_payoff(unsigned prey_val){
	if (!social_choices[competition - 1]) /* no people so no payoff */
		return 0;
	//unsigned player_sum = social_choices[0] + social_choices[1] + social_choices[2];
	/* first we add the payoff gained by expoiting the ones that try to synergize */
	float payoff = prey_val;
	payoff += (float)social_choices[synergy - 1] * comp_exploit;
	/* we then add the small gain from the lone predators */
	payoff += (float)social_choices[ignore - 1] * comp_loss;
	/* we now reduce the penalty from each competitor antagonizing with each other */
	payoff -= (float)(social_choices[competition - 1] - 1) * antag_loss;
	/* we know split the payoff to each competitor */
	payoff = payoff/(float)social_choices[competition - 1];
	if (payoff < 0.001)
		payoff = 0.001;
	return payoff;
}

float ignore_payoff(unsigned prey_val){ /* we calculate the payoff for each player choosing the ignore strategy on a certain game set */
	if (!social_choices[ignore - 1]) /* no people so no payoff */
		return 0;
	//unsigned player_sum = social_choices[0] + social_choices[1] + social_choices[2];
	float payoff = prey_val;
	/* first, we add the payoff gained from other trying to be synergetic - which is equal to what these players lose for "failing" to synergize */
	payoff += (float)social_choices[synergy - 1] * synergy_loss;
	/* then we subtract the payoff lost due to competitors */
	payoff -= (float)social_choices[competition - 1] * comp_loss;
	/* finally we divide the final payoff by the number of lone predators, since they split it equally */
	payoff = payoff / (float)social_choices[ignore -1];
	if (payoff < 0.001)
		payoff = 0.001;
	return payoff;
}

/* returns to each player the payoff they get from playing the game */
void grant_payoff(prey * p){
	if (p -> pred_index == NULL){
		fprintf(stderr,"payoff %u %u\n", p -> id, p -> num);
		return;
	}
	payoff_matrix[synergy - 1] = synergy_payoff(p -> value);
	payoff_matrix[competition - 1] = competition_payoff(p -> value);
	payoff_matrix[ignore - 1] = ignore_payoff(p -> value);

	assert(payoff_matrix[0] + payoff_matrix[1] + payoff_matrix[2] != 0);

	unsigned i;
	for (i = 0; i < p -> num; i++)
		gens[!curr_flag].pred[p -> pred_index[i]].fitness = payoff_matrix[gens[!curr_flag].pred[p -> pred_index[i]].strategy - 1];
}
