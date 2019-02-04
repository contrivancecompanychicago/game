#include "game.h"

time_t t;

extern void cmd_params();


/* ----- from prey.c ------ */
extern void add_prey();
extern void print_prey();

extern unsigned prey_num;
extern prey * preylist;

/* --- from predator.c --- */
extern void init_pred();

extern unsigned pred_num;
extern char const_size;
extern unsigned * People; 		/* number of predators in each generation */
extern unsigned cram;

/* from strategy_payoff.c */
extern unsigned social_choices[3];
extern float payoff_matrix[3];


unsigned seed = 0; 			/* for use with random ~ also user defined */

unsigned rounds = 100;			/* default value unless user defined */
unsigned curr_gen = 0;
double dimension = 50.0;		/* default value unless user defined */

extern next_gen * chosen;

predator ** gens = NULL; /* store predators in each generation */

void play(prey * p){
	p -> num = 0; /* reset the number of predators participating in this game */
	find_in_range(p, p -> xaxis, p -> yaxis, gens[curr_gen]);
	grant_payoff(p -> value);
	if (const_size)
		p -> num = social_choices[0] + social_choices[1] + social_choices[2];
	else
		 p -> num = reprofunction(); /* sets the number of next generation predators caused by this single game */
	print_next(p);

	People[curr_gen + 1] += p -> num;
	printf("[%d %d %d]\n", social_choices[0], social_choices[1], social_choices[2]);
}

void prep_game(prey * p){	/* hard_coded --> faster */
	social_choices[0] = 0;
	social_choices[1] = 0;
	social_choices[2] = 0;
	payoff_matrix[0] = 0;
	payoff_matrix[1] = 0;
	payoff_matrix[2] = 0;
	next_gen * tmp = NULL;
	while (p -> ng != NULL){
		tmp = p -> ng;
		p -> ng = p -> ng -> next;
		free(tmp);
	}
}

void will_go_somewhere(prey * p){
	/* ----- this shit is the problem ----- */
	if (p -> ng != NULL && p -> ng -> next != NULL){
		unsigned i;
		for (i = 0; i < p -> num; i++)
			next_generation(p -> xaxis, p -> yaxis);
	}
	/* after setting the new generation has been added we remove the predators that already played from the board */
	next_gen * ng = NULL;
	while (chosen != NULL ){
		ng = chosen;
		chosen = chosen -> next;
		free(ng);
	}
	// /deplete_prey(p); /* apply effect of game on the prey */
	/* ----- this shit is the problem ----- */
}

void game(){
	while (curr_gen < rounds){
		printf("GENERATION: %d\n", curr_gen);
		if (curr_gen == cram)
			prune();
		prey * p = preylist;
		while (p != NULL){
			prep_game(p);
			play(p);

			will_go_somewhere(p);
			p = p -> next;
		}
		++curr_gen;
		print_predators(gens[curr_gen]); /* test whether predators reproduce correctly */
	}
}

int main(int argc, char** argv){
	seed = (unsigned)time(&t);
	cmd_params(argc, argv);
	clock_t start = clock();
	srand(seed);

	/* ------------------------------------- */
	gens = calloc(rounds+1, sizeof(predator*));
	People = calloc(rounds+1, sizeof(unsigned));
	People[0] = pred_num;

	int i;
	for (i = 0; i < prey_num; i++)
		add_prey();
	//print_preys();

	for (i = 0; i < pred_num; i++)
		init_pred();
	//print_predators();

	game();

	for (i = 0; i < rounds; i++)
		free_tree(gens[i]);

	/* ------------------------------------- */
	clock_t end = clock();
	FILE * f1 = fopen("seedtimesps.txt", "a");
	float seconds = (float)(end - start) / CLOCKS_PER_SEC;
	fprintf(f1,"----------------------\n seed: %d in %f seconds \n",seed, seconds);
	fclose(f1);
	printf ("\nALL DONE in %f\n", seconds);
	return 0;
}
