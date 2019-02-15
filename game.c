#include "game.h"

time_t t;
gsl_rng * r;

extern void cmd_params();

/* ----- from prey.c ------ */
extern void print_prey();

extern unsigned prey_num;


/* --- from predator.c --- */
extern void init_pred();

extern unsigned pred_num;
extern char const_size;
extern unsigned * People; 			/* number of predators in each generation */
extern unsigned cram;

extern unsigned phenotype_size;

/* from strategy_payoff.c */
extern unsigned social_choices[3];
extern float payoff_matrix[3];
extern unsigned unable;

unsigned seed = 0; 							/* for use with random ~ also user defined */

unsigned rounds = 100;					/* default value unless user defined */
unsigned curr_gen = 0;
double dimension = 50.0;				/* default value unless user defined */

generation * gens = NULL; 			/* store predators in each generation */
extern prey * prey_array;

void reproduce(prey * p){
	if (gens[curr_gen+1].pred == NULL)
		gens[curr_gen+1].num = 0;
	unsigned i;
	for (i = 0; i < p -> num; i++) /* create the predators for the next generation */
		add_predator(p -> ng, p -> num);
}

void add_unable(){
	unsigned i;
	unsigned random = 0;
	for (i = 0; i < unable; i++){
		random = rand() % prey_num; /* select prey to host the extra predator */
		if (prey_array[random].num == 0){
			i--;
			continue;
		}
		add_predator(prey_array[random].ng, prey_array[random].num);
		People[curr_gen+1]++;
	}
}

void play(prey * p){
	find_in_range(p, curr_gen);
	if (p -> num != 0){
		grant_payoff(p);
		/* if not constant pop size add stuff to calculate that */
		reproduce(p);
	}
	People[curr_gen+1] += p -> num;
}

void prep_game(prey * p){	/* hard_coded --> faster */
	social_choices[0] = 0;
	social_choices[1] = 0;
	social_choices[2] = 0;
	payoff_matrix[0] = 0;
	payoff_matrix[1] = 0;
	payoff_matrix[2] = 0;
	free(p -> ng);
	p -> ng = NULL;
	p -> num = 0;
}

void game(){
	while (curr_gen < rounds){
		if (curr_gen == cram)
			prune();
		unable = gens[curr_gen].num; /* no-one has played a game yet */
		unsigned i;
		for (i = 0; i < prey_num; i++){
			prep_game(&prey_array[i]);
			play(&prey_array[i]);
		}
		add_unable();
		curr_gen++;
	}
}

int main(int argc, char** argv){
	seed = (unsigned)time(&t);
	cmd_params(argc, argv);
	clock_t start = clock();

	srand(seed);
	const gsl_rng_type * T;
	gsl_rng_env_setup();
	T = gsl_rng_default;
	r = gsl_rng_alloc(T);
	gsl_rng_set(r, seed);


	/* ------------------------------------- */
	gens = calloc(rounds+1, sizeof(generation));
	People = calloc(rounds+1, sizeof(unsigned));
	People[0] = pred_num;

	unsigned i;
	init_preys();
	print_preys();
	for (i = 0; i < pred_num; i++)
		init_predator();
	print_predators(0);

	game();
	//get_the_ms_output
	//maybe needs a mutation / recombination array

	/* free everything */
	unsigned j = 0;
	for (i = 0; i <= rounds; i++){
		for (j = 0; j < gens[i].num; j++)
			free(gens[i].pred[j].pheno);
		free(gens[i].pred);
	}
	free(gens);
	free_prey();
	free(People);
	gsl_rng_free(r);

	/* ------------------------------------- */
	clock_t end = clock();
	FILE * f1 = fopen("seed_time.txt", "a");
	float seconds = (float)(end - start) / CLOCKS_PER_SEC;
	fprintf(f1,"----------------------\n seed: %d in %f seconds \n",seed, seconds);
	fclose(f1);
	printf ("\nALL DONE in %f\n", seconds);
	return 0;
}
