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

extern strategy_t choose_strategy(unsigned aggro);
extern float variance;

extern unsigned rounds;
extern gsl_rng * r;

double recombination_rate = 0.1;
double mutation_rate = 0.3;
short * MutationEvents;

unsigned num_inf = 0;					/* number of 64-sized genome segments that influence strategy */
unsigned * PosInfuence; 			/* which those 64-sized segments are */

/* ------ genotype expression ------ */
unsigned genotype_size = 5; 	/* determines how many bases affect the strategy of each predator */
unsigned migration_size = 5;	/* determines how many bases affect the migration pattern of each predator */
/* ---------------------------------- */
char const_size = 0; 					/* determines whether the number of predators remains constant in each generation */
unsigned pred_num = 10;				/* initial number of predators ~ Default 10 unless user defined */
unsigned pred_pid = 0;
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
	p -> geno = (num_type *)malloc(sizeof(num_type) * genotype_size);
	num_type left, right;
	if (rand() / (double)RAND_MAX < 0.5){ /* parent1 gives the left part */
		memcpy(p -> geno, parent1 -> geno, (cutting_point + 1) * sizeof(num_type));
		if (cutting_point != genotype_size - 1)
			memcpy(p -> geno + (cutting_point + 1)*sizeof(num_type), parent2 -> geno + (cutting_point + 1)*sizeof(num_type),
		 (genotype_size - cutting_point - 2) * sizeof(num_type));
		 left = parent1 -> geno[cutting_point + 1];
		 right = parent2 -> geno[cutting_point + 1];
	}
	else{
		memcpy(p -> geno, parent2 -> geno, (cutting_point + 1) * sizeof(num_type));
		if (cutting_point != genotype_size - 1)
			memcpy(p -> geno + (cutting_point + 1)*sizeof(num_type), parent1 -> geno + (cutting_point + 1)*sizeof(num_type),
			 (genotype_size - cutting_point - 2) * sizeof(num_type));
			 left = parent2 -> geno[cutting_point + 1];
			 right = parent1 -> geno[cutting_point + 1];
	}

	/* now we need to recombine the cutting point */
	unsigned cut_base = rand() % (sizeof(num_type) * 8);
	left = left >> cut_base << cut_base;
	right = right << (63 - cut_base) >> ((sizeof(num_type) * 8) - cut_base);
	p -> geno[cutting_point + 1] = left | right;
	p -> aggro = 0;
	unsigned i;
	for (i = 0; i < num_inf; i++){
		p -> geno[PosInfuence[i]] = rand() % MAXIMUM;
		p -> aggro += __builtin_popcount(p -> geno[PosInfuence[i]]);
	}
	return p;
}

predator * set_geno(predator * p, predator * parent1, predator * parent2){
	/* we first need to see whether a recombination event takes place */
	double random = ((float)rand()/(float)(RAND_MAX));
	if (0)//random < recombination_rate)
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
		unsigned j = rand() % (8 * sizeof(num_type));	/* which bit of that segment will get the mutation */
		num_type mask = 1 << j;
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

void create_fitness_map(unsigned gen){
	unsigned i, j;
	for (i = 0; i < gens[gen].num - 1; i++){
		for (j = i + 1; j < gens[gen].num - 1; j++){
			if (! (abs(gens[gen].pred[i].xaxis - gens[gen].pred[j].xaxis) > range) || (abs(gens[gen].pred[i].yaxis - gens[gen].pred[j].yaxis) > range) )
				tot_fit +=  (gens[gen].pred[i].fitness * gens[gen].pred[j].fitness) / (distance(i,j, gen) + 1);
			FitMap[i][j] = tot_fit;
		}
	}
}

/* we select a pair of predators to be the parents of the child */
predator * choose_parents(predator * p){
	predator * parent1 = NULL;
	predator * parent2 = NULL;
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


predator * choose_burnin_parents(predator * p){

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

void add_predator(unsigned num){
	gens[curr_flag].pred[num].fitness = 0.0; /* is determined after the game */
	gens[curr_flag].pred[num] = *choose_parents(&gens[curr_flag].pred[num]);
	gens[curr_flag].pred[num].strategy = choose_strategy(gens[curr_flag].pred[num].aggro);
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
		fprintf(f1,"%u [%lf %lf] %f\n", gens[gen].pred[j].strategy, gens[gen].pred[j].xaxis, gens[gen].pred[j].yaxis,
						gens[gen].pred[j].fitness);
		for (i = 0; i < genotype_size; i++)
			fprintf(f1, "%lu\t", gens[gen].pred[j].geno[i]);
		fprintf(f1,"\n");
	}
	fprintf(f1, "\n");
	fclose(f1);
}

/* ------------------ */

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
	if (nsyn > 0){ /* user-defined synergistic predators */
		gens[0].pred[x].strategy = synergy;
		nsyn--;
		for (i = 0; i < num_inf; i++)
			gens[0].pred[x].geno[PosInfuence[i]] = 0;
	}
	else if (nign > 0){
		gens[0].pred[x].strategy = ignore;
		nign--;
		for (i = 0; i < num_inf; i++){
			gens[0].pred[x].geno[PosInfuence[i]] = MAXIMUM >> 6;
			gens[0].pred[x].aggro += __builtin_popcount(gens[0].pred[x].geno[PosInfuence[i]]);
		}
	}
	else if (ncom > 0){
		gens[0].pred[x].strategy = competition;
		ncom--;
		for (i = 0; i < num_inf; i++){
			gens[0].pred[x].geno[PosInfuence[i]] = 1 << 31;
			gens[0].pred[x].aggro += __builtin_popcount(gens[0].pred[x].geno[PosInfuence[i]]);
		}
	}
	else{
		for (i = 0; i < num_inf; i++){
			gens[0].pred[x].geno[PosInfuence[i]] = rand() % MAXIMUM;
			gens[0].pred[x].aggro += __builtin_popcount(gens[0].pred[x].geno[PosInfuence[i]]);
		}
		gens[0].pred[x].strategy = choose_strategy(gens[0].pred[x].aggro);
	}
}
