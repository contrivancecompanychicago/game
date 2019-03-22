#include "game.h"

time_t t;
gsl_rng * r;

extern unsigned cmd_params();

/* ----- from prey.c ------ */
extern void print_prey();
extern unsigned prey_num;

/* --- from predator.c --- */
extern void add_burnin_predator(unsigned num);
extern void init_pred();
extern unsigned pred_num;
extern char const_size;
extern unsigned * People; 			/* number of predators in each generation */
extern unsigned cram;
extern unsigned genotype_size;
extern unsigned * PosInfuence;
extern unsigned num_inf;
extern unsigned * MutationEvents;
extern double ** FitMap;
char pop_change = '0';
unsigned fit_size;

/* --- from produce_output.c --- */
extern void sampling(unsigned num);
extern void print_samples();
extern void free_samples();

extern void ms_output();
sample_events * sample_h = NULL;
unsigned samples = 10;
int sample_gen = -1;

/* from strategy_payoff.c */
extern unsigned social_choices[3];
extern float payoff_matrix[3];

unsigned burnin = 200;
unsigned rounds = 500;					/* default value unless user defined */
unsigned curr_gen = 0;
unsigned curr_flag = 1;					/* which part of the vector is used to store the current generation */
double dimension = 50.0;				/* default value unless user defined */
extern float tot_fit;

generation * gens = NULL; 			/* store predators in each generation */
extern prey * prey_array;

/* population size change */
unsigned first_event = -1; 			/* marks the generation for which the tree is pruned */
bottle * bottle_h = NULL;

void play(prey * p){
	find_in_range(p, !curr_flag);
	if (p -> num != 0)
		grant_payoff(p);
		/* if not constant pop size add stuff to calculate that */
	People[curr_gen + 1] += p -> num;
}

void prep_game(prey * p){	/* hard_coded --> faster */
	social_choices[0] = 0;
	social_choices[1] = 0;
	social_choices[2] = 0;
	payoff_matrix[0] = 0;
	payoff_matrix[1] = 0;
	payoff_matrix[2] = 0;
	free(p -> pred_index);
	p -> pred_index = NULL;
	p -> num = 0;
}

void allocate_fitness_table(){
	unsigned i;
	if (FitMap != NULL){
		for (i = 0; i < gens[curr_flag].num - 1; i++)
			free(FitMap[i]);
		free(FitMap);
	}
	FitMap = (double **)malloc((fit_size - 1) * sizeof(double *));
	for (i = 0; i < gens[!curr_flag].num - 1; i++)
		FitMap[i] = (double *)malloc((fit_size - 1) * sizeof(double));
}

void free_gen(unsigned gen){
	unsigned j;
	for (j = 0; j < fit_size; j++)
		free(gens[gen].pred[j].geno);
	free(gens[gen].pred);
	gens[gen].pred = NULL;
	gens[gen].num = 0;
}

void change_population_size(){
	/* in-order to facilitate an increase in pop-size we need to increase the size of the array */
	/* in this scenario we need to allocate memory for the predators */
	if ( fit_size < bottle_h -> preds){
		gens[curr_flag].pred = realloc(gens[curr_flag].pred, sizeof(predator) * bottle_h -> preds);
		unsigned i;
		for (i = pred_num; i < bottle_h -> preds; i++)
			gens[curr_flag].pred[i].geno = malloc(sizeof(num_type) * genotype_size);
		fit_size = bottle_h -> preds;
	}
	pred_num = bottle_h -> preds;
	bottle * tmp = bottle_h;
	bottle_h = bottle_h -> next;
	free(tmp);
	if (bottle_h == NULL)
		first_event = -1;
	else
		first_event = bottle_h -> gen;
	pop_change = '1'; // signals to change the pop_size of the next event */
}

void change_fit_population_size(){
	if (gens[curr_flag].num < fit_size){
		gens[curr_flag].pred = realloc(gens[curr_flag].pred, sizeof(predator) * pred_num);
		allocate_fitness_table();
		unsigned i;
		for (i = gens[curr_flag].num; i < pred_num; i++)
			gens[curr_flag].pred[i].geno = malloc(sizeof(num_type) * genotype_size);
	}
	pop_change = '0';
}

void burn_in(){
	unsigned i;
	while (curr_gen < burnin){
		gens[curr_flag].num = 0;
		for (i = 0; i < pred_num; i++){
			add_burnin_predator(i);
			print_predators(curr_flag, curr_gen);
		}
		gens[curr_flag].num += pred_num;
		curr_gen++;
		curr_flag = !curr_flag; /* switch the index of the previous generation */
	}
	curr_gen = 0;
}

void game(){
	unsigned i = 0;
	while (curr_gen < rounds){
		/* special event happens during this generation */
		if (curr_gen % 500 == 0)
			fprintf(stderr, "Current Generation: %u\n", curr_gen);
		if (curr_gen == sample_gen && sample_h != NULL){
			sampling(sample_h -> num);
			sample_events * tmp = sample_h;
			sample_h = sample_h -> next;
			free(tmp);
			if (sample_h != NULL)
				sample_gen = sample_h -> gen;
			print_samples();
		}
		if (curr_gen == first_event)
			change_population_size();
		else if(pop_change != '0')
			change_fit_population_size();


		gens[curr_flag].num = 0;
		tot_fit = 0.0;
		for (i = 0; i < prey_num; i++){
			prep_game(&prey_array[i]);
			play(&prey_array[i]);
		}
		reproduce();
		curr_gen++;
		curr_flag = !curr_flag; /* switch the index of the previous generation */
	}
}

void freedom(){
	unsigned i;
	for (i = 0; i < fit_size - 1; i++)
		free(FitMap[i]);
	free(FitMap);
	free_gen(0);
	free_gen(1);
	free(gens);
	free_prey();
	free(PosInfuence);
	free(MutationEvents);
	free_samples();
	free(People);
	gsl_rng_free(r);
}

int main(int argc, char** argv){
	unsigned seed = cmd_params(argc, argv);

	srand(seed);
	const gsl_rng_type * T;
	gsl_rng_env_setup();
	T = gsl_rng_default;
	r = gsl_rng_alloc(T);
	gsl_rng_set(r, seed);
	clock_t start = clock();

	/* ---- initialization step ------ */
	gens = calloc(2, sizeof(generation)); /* we only need the current and the previous generation */
	fit_size = pred_num;
	People = calloc(rounds + 1, sizeof(unsigned));
	People[0] = pred_num;
	init_preys();
	if (!num_inf){
		num_inf = 1;
		PosInfuence = malloc(sizeof(unsigned));
		PosInfuence[0] = rand()% genotype_size;
	}
	unsigned i;
	for (i = 0; i < pred_num; i++)
		init_predator();

	/* ---- init generation 1 (no values assigned)---- */
	gens[1].pred = malloc(pred_num * sizeof(predator));
	for (i = 0; i < pred_num; i++)
		gens[1].pred[i].geno = malloc(sizeof(num_type) * genotype_size);
	MutationEvents = calloc(genotype_size, sizeof(short));
	/* ---- we now create the map containing the fitness of each pair of predators */

	burn_in();

	allocate_fitness_table();
	/* actually play the game */
	game();

	/* we now sample predators and print the mutation table */
	//assert(samples <= pred_num);
	// if (sample_h != NULL) /* final sampling event */
  // 	sampling(sample_h -> num);
	ms_output();

	/* free everything */
	freedom();

	/* ------------------------------------- */
	clock_t end = clock();
	float seconds = (float)(end - start) / CLOCKS_PER_SEC;
	FILE * f1 = fopen("seed_time.txt", "a");
	fprintf(f1,"----------------------\n seed: %d in %f seconds \n",seed, seconds);
	fclose(f1);
	printf ("\nALL DONE in %f\n", seconds);
	return 0;
}
