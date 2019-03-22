#include "cmd_params.h"

/* ----- from game.c ----- */
extern unsigned burnin;
extern unsigned rounds;
extern time_t t;
extern int sample_gen;

/* ----- from prey.c ----- */
extern unsigned prey_num;
extern unsigned user_prey;
extern prey_pos * posh;
prey_pos * post = NULL;

/* --- from predator.c --- */
extern char const_size;
extern unsigned pred_num;
extern unsigned first_event;
extern bottle * bottle_h;
extern unsigned nsyn;
extern unsigned nign;
extern unsigned ncom;
extern unsigned samples;

extern unsigned genotype_size;
extern unsigned * PosInfuence;
extern unsigned num_inf;

/*- from strategy_payoff.c -*/
extern float variance;

bottle * tail = NULL;
extern sample_events * sample_h;
sample_events * sample_t;

void add_prey_pos(float x, float y){
	if (posh == NULL){ /* first entry */
		posh = malloc(sizeof(prey_pos));
		posh -> x = x;
		posh -> y = y;
		posh -> next = NULL;
		post = posh;
		return;
	}
	prey_pos * tmp = malloc(sizeof(prey_pos));
	tmp -> x = x;
	tmp -> y = y;
	tmp -> next = NULL;
	posh -> next = tmp;
	post = tmp;
}

/* bottleneck events */
void add_bottle(unsigned gen, unsigned preds){
	bottle * tmp = malloc(sizeof(bottle));
	tmp -> gen = gen;
	tmp -> preds = preds;
	tmp -> next = NULL;
	if (bottle_h == NULL){
		first_event = gen;
		bottle_h = tmp;
		tail = bottle_h;
		return;
	}
	tail -> next = tmp;
	tail = tail -> next;
}

/* sampling events */
void add_samples(unsigned gen, unsigned num){
	sample_events * tmp = malloc(sizeof(sample_events));
	tmp -> gen = gen;
	tmp -> num = num;
	tmp -> next = NULL;
	if (sample_h == NULL){
		sample_h = tmp;
		sample_t = tmp;
		sample_gen = gen;
		return;
	}
	sample_t -> next = tmp;
	sample_t = sample_t -> next;
}

void print_help(){
	printf("\n\n ------------ Command Line Parameters ------------ \n\n");

	printf("General purpose commands:\n\n");
	printf("\t-seed: user-defined seed for the random number generator function.\n\n");

	printf("\t-rnds: determines the number of generations the program will simulate. Paremeter value must be a positive integer.\n\n");
	printf("\t---\n\n");

	printf("\n\nGo read the fucking code\n\n");
}

unsigned cmd_params(int argc, char** argv){
	unsigned seed = (unsigned)time(&t);
	int i;
	for (i = 1; i < argc; i++){

		/* --------- general use ---------- */

		/* seed for random */
		if ( (!strcmp(argv[i], "-seed" ) ) ){
			seed = atoi(argv[++i]);
			continue;
		}

		/* gaussian parameter */
		if ( (!strcmp(argv[i], "-gsnd" ) ) ){
			variance = atof(argv[++i]);
			continue;
		}

		/* ----- help ----- */
		if ( (!strcmp(argv[i], "-help" ) ) ){
			print_help();
			exit(0);
		}

		if ( (!strcmp(argv[i], "-rnds" ) ) ){
			rounds = atoi(argv[++i]);
			continue;
		}

		if ( (!strcmp(argv[i], "-burn" ) ) ){
			burnin = atoi(argv[++i]);
			continue;
		}

		/* whether population size will remain constant */
		if ( (!strcmp(argv[i], "-cnst" ) ) ){
			const_size = 1;
			continue;
		}

		if ( (!strcmp(argv[i], "-pred" ) ) ){
			pred_num = atoi(argv[++i]);
			continue;
		}

		if ( (!strcmp(argv[i], "-prey" ) ) ){
			prey_num = atoi(argv[++i]);
			continue;
		}

		/* position in map for a prey */
		if ( (!strcmp(argv[i], "-ppos" ) ) ){
			 add_prey_pos(atof(argv[++i]), atof(argv[++i]));
			 user_prey++;
			continue;
		}

		if ( (!strcmp(argv[i], "-nsyn" ) ) ){
	 		nsyn = atoi(argv[++i]);
			continue;
		}

		if ( (!strcmp(argv[i], "-nign" ) ) ){
	 		nign = atoi(argv[++i]);
			continue;
		}

		if ( (!strcmp(argv[i], "-ncom" ) ) ){
	 		ncom = atoi(argv[++i]);
			continue;
		}

		if ( (!strcmp(argv[i], "-bttl" ) ) ){
			unsigned gen = atoi(argv[++i]);
			unsigned prd = atoi(argv[++i]);
			add_bottle(gen, prd);
			continue;
		}

		if ( (!strcmp(argv[i], "-size" ) ) ){
			genotype_size = atoi(argv[++i]);
			continue;
		}

		/* determines sample events */
		if ( (!strcmp(argv[i], "-smpl" ) ) ){
			unsigned gen = atoi(argv[++i]);
			unsigned num = atoi(argv[++i]);
			add_samples(gen, num);
			continue;
		}

		if ( (!strcmp(argv[i], "-pinf" ) ) ){
			num_inf = atoi(argv[++i]);
			PosInfuence = malloc(num_inf * sizeof(unsigned));
			unsigned x;
			for (x = 0; x < num_inf; x++)
				PosInfuence[x] = atoi(argv[++i]);
			continue;
		}

		fprintf(stderr, "Argument %s is invalid\n\n", argv[i]);
	}
	return seed;
}
