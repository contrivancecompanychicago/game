#include "predator.h"

#define MAXIMUM  8446744073709551615

extern generation * gens;
extern unsigned curr_gen; // remove
extern unsigned curr_flag;
extern unsigned genotype_size;
extern double dimension;
extern float range;
extern unsigned social_choices[3];
extern float payoff_matrix[3];
extern float tot_fit;

extern float com_rate;
extern float syn_rate;

extern strategy_t choose_strategy(unsigned aggro);
extern float variance;

extern unsigned rounds;
extern gsl_rng * r;
extern short byte2bit;

double recombination_rate = 0.1;
double mutation_rate = 0.02;
short * MutationEvents;

unsigned num_inf = 0;					/* number of 64-sized genome segments that influence strategy */
unsigned * PosInfuence; 			/* which those 64-sized segments are */

/* ------- genotype expression ------ */
unsigned genotype_size = 50; 	/* determines how many bases affect the strategy of each predator */
unsigned migration_size = 5;	/* determines how many bases affect the migration pattern of each predator */
/* ---------------------------------- */
char const_size = 0; 					/* determines whether the number of predators remains constant in each generation */
unsigned pred_num = 10;				/* initial number of predators ~ Default 10 unless user defined */
unsigned pred_pid = 0;
unsigned neut = 0;						/* number of user -defined neutral / all 0 players */
unsigned nsyn = 0;						/* number of user-defined synergetic predators */
unsigned nign = 0;						/* number of user-defined ignoring predators */
unsigned ncom = 0;						/* nubmer of user-defined competitive predators */

unsigned * People; 						/* number of predators in each generation */
double ** FitMap;

double mean(double x, double y){
	return (x + y)/(float)2;
}

void print_genotype(predator * p){
	FILE * f1 = fopen("genotype.txt", "a");
	unsigned i;
	for (i = 0; i < genotype_size; i++)
		fprintf(f1, "%lu ", p -> geno[i]);
	fprintf(f1, "\n");
	fclose(f1);
}

predator * recombine(predator * p, predator * parent1, predator * parent2){
	unsigned cutting_point = rand() % genotype_size; /* determine where the recombination happened */
	num_type left, right;
	if (rand() / (double)RAND_MAX < 0.5){ /* parent1 gives the left part */
		memcpy(p -> geno, parent2 -> geno, genotype_size * sizeof(num_type));
		if (cutting_point != 0)
			memcpy(p -> geno, parent1 -> geno, (cutting_point + 1) * sizeof(num_type));
		left = parent1 -> geno[cutting_point];
		right = parent2 -> geno[cutting_point];
	}
	else{
		memcpy(p -> geno, parent2 -> geno, (cutting_point + 1) * sizeof(num_type));
		if (cutting_point != 0)
			memcpy(p -> geno, parent1 -> geno, genotype_size * sizeof(num_type));
		left = parent2 -> geno[cutting_point];
		right = parent1 -> geno[cutting_point];
	}

	/* now we need to recombine the cutting point */
	unsigned cut_base = rand() % (sizeof(num_type) * byte2bit);
	left = left >> cut_base << cut_base;
	right = right << (63 - cut_base) >> ((sizeof(num_type) * byte2bit) - cut_base);
	p -> geno[cutting_point] = left | right;
	/* recalculate the aggession level */
	p -> aggro = 0;
	unsigned i;
	for (i = 0; i < num_inf; i++)
		p -> aggro += __builtin_popcount(p -> geno[PosInfuence[i]]);
	return p;
}

predator * set_geno(predator * p, predator * parent1, predator * parent2){
	/* we first need to see whether a recombination event takes place */
	double random = ((float)rand()/(float)(RAND_MAX));
	if (random < recombination_rate)
		p = recombine(p, parent1, parent2);
	else{
		random = ((float)rand()/(float)(RAND_MAX));
		if (random < 0.5){
			memcpy(p -> geno, parent1 -> geno, genotype_size * sizeof(num_type));
			p -> aggro = parent1 -> aggro;
		}
		else{
			memcpy(p -> geno, parent2 -> geno, genotype_size * sizeof(num_type));
			p -> aggro = parent2 -> aggro;
		}
	}
	/* check for mutations */
	random = ((float)rand()/(float)(RAND_MAX));
	if (random < mutation_rate){
		unsigned i = rand() % genotype_size; /* which of the num-sized genome segment will have the mutation */
		unsigned j = rand() % (byte2bit * sizeof(num_type));	/* which bit of that segment will get the mutation */
		num_type mask = 1 << j;
		num_type test_correct = p -> geno[i];
		p -> geno[i] = p -> geno[i] ^ mask;
		MutationEvents[i] = 1;
	}
	return p;
}

predator * set_position(predator * p, predator * parent1, predator * parent2){
	double p1 = parent1 -> xaxis;
	double p2 = parent2 -> xaxis;
	p -> xaxis = mean(p1, p2) + gsl_ran_gaussian(r, variance);
	p -> yaxis = mean(parent1 -> yaxis, parent2 -> yaxis) + gsl_ran_gaussian(r, variance);
	return p;
}

double distance(unsigned p1, unsigned p2, unsigned gen){
	return abs(gens[gen].pred[p1].xaxis - gens[gen].pred[p2].xaxis)
			 + abs(gens[gen].pred[p1].yaxis - gens[gen].pred[p2].yaxis);
}


void create_neutral_fitness_map(unsigned gen){
        unsigned i, j;
	tot_fit = 0;
        for (i = 0; i < gens[gen].num - 1; i++){
                for (j = i + 1; j < gens[gen].num - 1; j++){
                        tot_fit +=  1.0 / (distance(i,j, gen) + 1);
                        FitMap[i][j] = tot_fit;
                }
   	}
}

void create_fitness_map(unsigned gen){
	unsigned i, j;
	//double tot_fit = 0.0;
	for (i = 0; i < gens[gen].num - 1; i++){
		for (j = i + 1; j < gens[gen].num - 1; j++){
			tot_fit +=  gens[gen].pred[i].fitness * gens[gen].pred[j].fitness;// / (distance(i,j, gen) + 1);
			FitMap[i][j] = tot_fit;
		}
	}
	assert(tot_fit != 0);
}

/* we select a pair of predators to be the parents of the child */
predator * choose_parents(predator * p){
	predator * parent1 = NULL;
	predator * parent2 = NULL;
	assert(tot_fit != 0);
	double fit = tot_fit * (rand() / (double)RAND_MAX);
	unsigned num = gens[(!curr_flag)].num;
	unsigned i,j;
	for (i = 0; i < num - 1; i++){
		if (FitMap[i][num - 2] < fit) /* we first locate the row of the pair */
			continue;
		for (j = i + 1; j < num - 1; j++){	/* upper triangular array */
			if (FitMap[i][j] >= fit){
				parent1 = &gens[(!curr_flag)].pred[i];
				parent2 = &gens[(!curr_flag)].pred[j];
				i = num;
				break;
			}
		}
	}
	assert(parent2 != NULL);
	assert(parent1 != NULL);
	/* now we need to determine the parameters defined by the parents */
	p = set_position(p, parent1, parent2);
	p = set_geno(p, parent1, parent2);
	return p;
}

void add_burnin_predator(unsigned num){
	gens[curr_flag].pred[num].fitness = 0.0; /* is determined after the game */
	unsigned parent1, parent2 = 0;
	parent1 = rand() % pred_num;
	parent2 = rand() % pred_num;
	while (parent2 == parent1)
		parent2 = rand() % pred_num;
	gens[curr_flag].pred[num] = *set_position(&gens[curr_flag].pred[num], &gens[!curr_flag].pred[parent1], &gens[!curr_flag].pred[parent2]);
	gens[curr_flag].pred[num] = *set_geno(&gens[curr_flag].pred[num], &gens[!curr_flag].pred[parent1], &gens[!curr_flag].pred[parent2]);
	gens[curr_flag].pred[num].strategy = choose_strategy(gens[curr_flag].pred[num].aggro);
}

void add_neutral_predator(unsigned num){
	gens[curr_flag].pred[num].fitness = 0.0; /* all same since all neutral */
	gens[curr_flag].pred[num] = *choose_parents(&gens[curr_flag].pred[num]);
	gens[curr_flag].pred[num].strategy = choose_strategy(gens[curr_flag].pred[num].aggro);
}

void add_predator(unsigned num){
	gens[curr_flag].pred[num].fitness = 0.0; /* is determined after the game */
	gens[curr_flag].pred[num] = *choose_parents(&gens[curr_flag].pred[num]);
	gens[curr_flag].pred[num].strategy = choose_strategy(gens[curr_flag].pred[num].aggro);
}

void neutral_reproduce(){
	create_neutral_fitness_map( !curr_flag);
	unsigned i;
	for (i = 0; i < pred_num; i++)
		add_neutral_predator(i);
	gens[curr_flag].num += pred_num;
	return;
}

void reproduce(){
	create_fitness_map( !curr_flag);
	unsigned i;
	for (i = 0; i < pred_num; i++)
		add_predator(i);
	gens[curr_flag].num += pred_num;
	return;
}

/* ------------------------------- */

void print_predators(unsigned gen, unsigned curr_gen){
	FILE * f1 = fopen("wirsinddiejaeger.txt", "a");
	unsigned i, j = 0;
	fprintf(f1,"gen: %u(%u) with %u predators\n", curr_gen, gen, gens[gen].num);
	for (j = 0; j < gens[gen].num; j++){
		fprintf(f1, "%u ", gens[gen].pred[j].strategy);
		// fprintf(f1,"[%lf %lf] strat:%u\n", gens[gen].pred[j].xaxis, gens[gen].pred[j].yaxis, gens[gen].pred[j].strategy);
		// for (i = 0; i < genotype_size; i++)
		// 	fprintf(f1, "%lu\t", gens[gen].pred[j].geno[i]);
		// fprintf(f1,"\n");
	}
	fprintf(f1, "\n");
	fclose(f1);
}

/* ------------------ */
predator * init_genotype(predator * p, unsigned pos, unsigned num){
	unsigned bits = (sizeof(num_type)*byte2bit);
	gsl_permutation * perm = gsl_permutation_alloc((size_t)bits);
	gsl_permutation_init(perm);
	gsl_ran_shuffle(r, perm -> data, (size_t)bits, sizeof(size_t) );

	unsigned i;
	gsl_permutation_free(perm);
	///assert(num == __builtin_popcount(p -> geno[pos]) );
	return p;
}

void init_predator(){
	unsigned x = gens[0].num;
	if (gens[0].pred == NULL){ /* first entry in the current generation */
		gens[0].pred = malloc(pred_num * sizeof(predator));
		gens[0].num = 0;
		x = 0;
	}

	gens[0].pred[x].fitness = 0.0; /* is determined after the game */
	gens[0].pred[x].xaxis = ((double)rand()/(double)RAND_MAX) * dimension;
	gens[0].pred[x].yaxis = ((double)rand()/(double)RAND_MAX) * dimension;
	gens[0].num++;

	/* now we need to initialize both the genotype and the strategy */
	gens[0].pred[x].geno = calloc(sizeof(num_type), genotype_size);
	gens[0].pred[x].aggro = 0;
	unsigned i = 0;
	unsigned bits = sizeof(num_type) * byte2bit;
	if (neut > 0){ /* neutral scenario for comparison with ms | start with 0's everywhere */
		gens[0].pred[x].strategy = synergy;
		neut--;
	}

	else if (nsyn > 0){ /* user-defined synergistic predators */
		unsigned num;
		gens[0].pred[x].strategy = synergy;
		gens[0].pred[x].aggro = 0;
		for (i = 0; i < num_inf; i++){
			num = rand() % (unsigned)(bits * syn_rate);
			gens[0].pred[x] = *init_genotype(&gens[0].pred[x], PosInfuence[i], num);
			gens[0].pred[x].aggro += __builtin_popcount(gens[0].pred[x].geno[PosInfuence[i]]);
		}
		//gens[0].pred[x].strategy = choose_strategy(gens[0].pred[x].aggro);
		nsyn--;
	}
	else if (nign > 0){ /* user-defined ignore predators */

		gens[0].pred[x].strategy = ignore;
		gens[0].pred[x].aggro = 0;
		unsigned num;
		for (i = 0; i < num_inf; i++){
			if (com_rate <= 1)
				num = rand() % (unsigned)round(bits * (com_rate - syn_rate))  + (unsigned)round(bits * syn_rate);
			else
				num = rand() % (unsigned)round(bits * (1 - syn_rate))  + (unsigned)round(bits * syn_rate);
			gens[0].pred[x] = *init_genotype(&gens[0].pred[x], PosInfuence[i], num);
			gens[0].pred[x].aggro += __builtin_popcount(gens[0].pred[x].geno[PosInfuence[i]]);
		}
		//gens[0].pred[x].strategy = choose_strategy(gens[0].pred[x].aggro);
		nign--;
	}
	else if (ncom > 0){ /* user-defined competition predators */
		gens[0].pred[x].strategy = competition;
		gens[0].pred[x].aggro = 0;
		unsigned num;
		for (i = 0; i < num_inf; i++){
			num = (unsigned)(bits - rand() % (unsigned)(bits * (1.0 - com_rate)));
			gens[0].pred[x] = *init_genotype(&gens[0].pred[x], PosInfuence[i], num);
			gens[0].pred[x].aggro += __builtin_popcount(gens[0].pred[x].geno[PosInfuence[i]]);
		}
		//gens[0].pred[x].strategy = choose_strategy(gens[0].pred[x].aggro);
		ncom--;
	}
	else{
		for (i = 0; i < num_inf; i++){
			gens[0].pred[x].geno[PosInfuence[i]] = (unsigned long long)rand() % MAXIMUM;
			gens[0].pred[x].aggro += __builtin_popcount(gens[0].pred[x].geno[PosInfuence[i]]);
		}
		gens[0].pred[x].strategy = choose_strategy(gens[0].pred[x].aggro);
	}
}
